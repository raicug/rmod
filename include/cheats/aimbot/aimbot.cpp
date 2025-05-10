#include "aimbot.h"
#include <globals/settings.h>
#include "penetrate_walls.h"
#include "backtrack/history.h"
#include "backtrack/lag_compensation.h"
#include <SDK/utils/utilities.h>

float normalize_angle(float angle) {
	while (angle > 180.0f) angle -= 360.0f;
	while (angle <= -180.0f) angle += 360.0f;
	return angle;
}

float normalize_yaw(float yaw) {
	while (yaw > 180.0f) yaw -= 360.0f;
	while (yaw < -180.0f) yaw += 360.0f;
	return yaw;
}

namespace raicu::cheats::aimbot {

	void smooth(c_user_cmd* cmd, q_angle& angle) {
		if (!raicu::globals::settings::aimbot::smooth || raicu::globals::settings::aimbot::silent)
			return;

		angle.clamp();
		q_angle delta = angle - cmd->view_angles;
		delta.clamp();
		angle = cmd->view_angles + delta / raicu::globals::settings::aimbot::smooth;
	}

	bool check_hitbox_group(int group) {
		switch (raicu::globals::settings::aimbot::hitbox) {
		case 0: return group == HITGROUP_HEAD;
		case 1: return group == HITGROUP_CHEST;
		case 2: return group == HITGROUP_STOMACH;
		default: return true;
		}
	}

	bool get_hit_position(c_base_entity* local_player, c_base_entity* entity, c_vector& shoot_pos, std::shared_ptr<matrix3x4[]> bone_to_world) {
		if (!entity || !entity->get_client_renderable())
			return false;

		void* model = entity->get_client_renderable()->get_model();
		if (!model)
			return false;

		studiohdr_t* hdr = interfaces::model_info->get_studio_model(model);
		if (!hdr)
			return false;

		mstudiohitboxset_t* hitbox_set = hdr->hitbox_set(entity->hitbox_set());
		if (!hitbox_set)
			return false;

		if (!bone_to_world) {
			auto bone_array = std::shared_ptr<matrix3x4[]>(new matrix3x4[hdr->num_bones](), std::default_delete<matrix3x4[]>());
			if (!entity->get_client_renderable()->setup_bones(bone_array.get(), hdr->num_bones, BONE_USED_BY_ANYTHING, interfaces::global_vars->curtime))
				return false;

			bone_to_world = bone_array;
		}

		for (int i = 0; i < hitbox_set->num_hitboxes; i++) {
			mstudiobbox_t* hitbox = hitbox_set->hitbox(i);
			if (!hitbox || !check_hitbox_group(hitbox->group))
				continue;

			c_vector mins, maxs;
			math::vector_transform(hitbox->bb_min, bone_to_world[hitbox->bone], mins);
			math::vector_transform(hitbox->bb_max, bone_to_world[hitbox->bone], maxs);
			shoot_pos = (mins + maxs) * 0.5f;

			if (penetrate_walls::can_hit(local_player, entity, shoot_pos))
				return true;
		}

		// fallback
		c_vector pos = entity->get_abs_origin();
		c_vector mins = pos + entity->get_collidable()->mins();
		c_vector maxs = pos + entity->get_collidable()->maxs();
		shoot_pos = (mins + maxs) * 0.5f;

		return penetrate_walls::can_hit(local_player, entity, shoot_pos);
	}

	target_info find_best_target(c_user_cmd* cmd, c_base_entity* local_player) {
		target_info target;
		priority_info& priority = target.priority_info;

		q_angle view_angles = cmd->view_angles;

		c_vector eye_pos;
		local_player->get_eye_position(eye_pos);

		int max_clients = interfaces::engine->get_max_clients();
		for (int i = 1; i < max_clients; i++) {
			c_base_entity* entity = interfaces::entity_list->get_entity(i);
			if (!entity || !entity->is_player() || !entity->is_alive() || entity == local_player)
				continue;

			float sim_time = entity->get_simulation_time();
			int distance = local_player->get_abs_origin().distance_to(entity->get_abs_origin());
			int health = entity->get_health();
			c_vector shoot_pos;

			if (!get_hit_position(local_player, entity, shoot_pos)) {
				if (!raicu::globals::settings::aimbot::backtrack)
					continue;

				lag_record record;
				if (!history::get_latest_record(i, record))
					continue;

				if (!get_hit_position(local_player, entity, shoot_pos, record.bone_to_world))
					continue;

				sim_time = record.simulation_time;
				distance = record.origin.distance_to(entity->get_abs_origin());
			}

			if (!shoot_pos.is_valid())
				continue;

			c_vector screen;
			if (!utilities::world_to_screen(shoot_pos, &screen))
				continue;

			ImGuiIO& io = ImGui::GetIO();
			float screen_w = io.DisplaySize.x;
			float screen_h = io.DisplaySize.y;

			float dx = screen.x - (screen_w / 2.0f);
			float dy = screen.y - (screen_h / 2.0f);
			float screen_dist = sqrtf(dx * dx + dy * dy);

			float fov_deg = raicu::globals::settings::other::fovSize;
			float cam_fov_deg = raicu::globals::settings::fov;

			float fov_rad = math::deg2rad(fov_deg);
			float cam_fov_rad = math::deg2rad(cam_fov_deg);

			float fov_radius = tanf(fov_rad / 2.0f) / tanf(cam_fov_rad / 2.0f) * (screen_w / 2.0f);

			if (screen_dist > fov_radius * 0.98f)
				continue;

			q_angle shoot_angle = math::calc_angle(eye_pos, shoot_pos);

			bool is_target = false;
			player_info_t info;
			if (interfaces::engine->get_player_info(i, &info)) {
				const auto& players = raicu::globals::settings::whitelist["players"];
				is_target = std::any_of(players.begin(), players.end(),
					[&info](const nlohmann::json& player) {
						return player["name"] == info.name;
					});
			}

			bool skip = false;
			switch (raicu::globals::settings::aimbot::priority) {
				case 0:
					if (is_target) {
						priority.fov = 0.0f;
						skip = false;
					} else {
						skip = fov_radius > priority.fov;
					}
					break;
				case 1:
					skip = distance > priority.distance;
					break;
				case 2:
					skip = health > priority.health;
					break;
			}

			if (skip)
				continue;

			priority.fov = fov_radius;
			priority.distance = distance;
			priority.health = health;

			target.entity = entity;
			target.shoot_pos = shoot_pos;
			target.shoot_angle = shoot_angle;
			target.simulation_time = sim_time;
		}

		return target;
	}

	void run(c_user_cmd* cmd) {
		target = {};
		if (!raicu::globals::settings::aimbot::enabled)
			return;

		c_base_entity* local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());
		if (!local_player || !local_player->is_alive())
			return;

		c_base_combat_weapon* weapon = local_player->get_active_weapon();
		if (!weapon || !weapon->can_fire() || weapon->is_holding_tool())
			return;

		target = find_best_target(cmd, local_player);
		if (!target.entity || raicu::globals::settings::open || cmd->is_typing || cmd->is_world_clicking)
			return;

		smooth(cmd, target.shoot_angle);
		cmd->view_angles = target.shoot_angle;

		if (!raicu::globals::settings::aimbot::silent)
			interfaces::engine->set_view_angles(target.shoot_angle);

		if (raicu::globals::settings::aimbot::automatic_fire)
			cmd->buttons |= IN_ATTACK;
	}
}