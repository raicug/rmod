#include "Visuals.h"
#include "globals/settings.h"
#include "logging/logger.h"
#include "cheats/Drawing/Drawing.h"
#include <SDK/utils/utilities.h>
#include <SDK/utils/lua_utilities.h>
#include <SDK/Interface.h>
#include <algorithm>
#include <cheats/aimbot/aimbot.h>

#include "aimbot/backtrack/history.h"

using raicu::cheats::Visuals;
using namespace raicu::globals::settings;
using raicu::globals::Drawing;

void Visuals::Render() {
	static auto last_log_time = std::chrono::steady_clock::now();

	ImGuiIO &io = ImGui::GetIO();

	if (!interfaces::engine->is_in_game() || !interfaces::engine->is_connected()) {
		return;
	};

	c_base_entity *local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());

	if (!local_player) {
		logger::Log(logger::LOGGER_LEVEL_ERROR, "Failed to get local player!");
		return;
	}

	c_vector origin = local_player->get_abs_origin();

	ImVec2 center(io.DisplaySize.x / 2, io.DisplaySize.y / 2);

	if (other::enableFov)
		DrawAimbotFOV(other::fovSize);
	if (crosshairValues::enabled)
		DrawCrosshair(
			crosshairValues::width, // Width
			crosshairValues::height, // Height
			crosshairValues::offset, // Offset
			Drawing::ToColor(&crosshairValues::color), // Main Color
			crosshairValues::rounding, // Rounding
			crosshairValues::outlineEnabled, // Outline?
			Drawing::ToColor(&crosshairValues::outlineColor), // Outline Color
			crosshairValues::outlineThickness); // Outline Thickness

	if (!espValues::enabled) return;
	for (size_t i = 0; i <= interfaces::engine->get_max_clients(); i++) {
		c_base_entity *entity = interfaces::entity_list->get_entity(static_cast<int>(i));
		if (!entity) {
			continue;
		}

		if (entity->is_player()) {
			if (!entity->is_alive()) continue;
			if (entity == local_player) continue;

			float offset = 0;
			float distance = origin.distance_to(entity->get_abs_origin());

			float alpha = std::clamp((raicu::globals::settings::espValues::render_distance - distance) / 100.f, 0.f,
			                         1.f);
			if (alpha <= 0.0f)
				continue;

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

			player_info_t pinfo;
			interfaces::engine->get_player_info(i, &pinfo);

			if (strlen(pinfo.name) == 0) {
				return;
			}

			bool is_whitelisted = false;
			if (whitelist.contains("players")) {
				const auto& players = whitelist["players"];
				is_whitelisted = std::any_of(players.begin(), players.end(),
					[&pinfo](const nlohmann::json& player) {
						return player["name"] == pinfo.name;
					});
			}

			if (is_whitelisted) {
				return;
			}

			if (espValues::snapline)
				Visuals::DrawSnapline(static_cast<int>(i),
				                      Drawing::ToColor(&espValues::snapLineColor));
			if (espValues::origin) Visuals::DrawOrigin(static_cast<int>(i), Drawing::ToColor(&espValues::originColor));
			if (espValues::showTeam) Visuals::DrawTeam(static_cast<int>(i), offset);
			if (espValues::name) Visuals::DrawName(static_cast<int>(i), offset);
			if (espValues::distance) Visuals::DrawDistance(static_cast<int>(i), offset, distance);
			if (espValues::box) Visuals::Drawbox(static_cast<int>(i));
			if (espValues::health) Visuals::DrawHealth(static_cast<int>(i));
			if (globals::settings::aimbot::visualise_target_line) Visuals::DrawLineToTarget();
			if (espValues::skeleton) Visuals::DrawSkeleton(static_cast<int>(i));
			if (globals::settings::aimbot::backtrackEnabled) Visuals::DrawBacktrack(static_cast<int>(i));

			ImGui::PopStyleVar();
		}
	}
}

void Visuals::DrawCrosshair(float width, float length, float offset, ImU32 color, float rounding, bool outline,
                            ImU32 outlineColor, float outlineThickness) {
	ImGuiIO &io = ImGui::GetIO();
	float w = io.DisplaySize.x;
	float h = io.DisplaySize.y;

	float x = w / 2.f, y = h / 2.f;

	if (crosshairValues::outlineEnabled) {
		Drawing::OutlineFilledBox(x - width / 2.f, y - offset - length, width, length, color, outlineColor,
		                          outlineThickness, rounding);
		Drawing::OutlineFilledBox(x + offset, y - width / 2.f, length, width, color, outlineColor, outlineThickness,
		                          rounding);
		Drawing::OutlineFilledBox(x - width / 2.f, y + offset, width, length, color, outlineColor, outlineThickness,
		                          rounding);
		Drawing::OutlineFilledBox(x - offset - length, y - width / 2.f, length, width, color, outlineColor,
		                          outlineThickness, rounding);
	} else {
		Drawing::BoxFilled(x - width / 2.f, y - offset - length, width, length, color, rounding);
		Drawing::BoxFilled(x + offset, y - width / 2.f, length, width, color, rounding);
		Drawing::BoxFilled(x - width / 2.f, y + offset, width, length, color, rounding);
		Drawing::BoxFilled(x - offset - length, y - width / 2.f, length, width, color, rounding);
	}
}

void Visuals::DrawName(int CurrentEnt, float &offset) {
    c_vector out;
    player_info_t pinfo;
    c_vector worldtoscreenent, currentOrg;

    c_base_entity *current = interfaces::entity_list->get_entity(CurrentEnt);
    currentOrg = current->get_abs_origin();

    if (utilities::world_to_screen(currentOrg, &worldtoscreenent)) {
        interfaces::engine->get_player_info(CurrentEnt, &pinfo);

        if (strlen(pinfo.name) == 0) {
            return;
        }

        bool is_friend = false;
        if (raicu::globals::settings::friend_list.contains("players")) {
            const auto& friends = raicu::globals::settings::friend_list["players"];
            is_friend = std::any_of(friends.begin(), friends.end(),
                [&pinfo](const nlohmann::json& player) {
                    return player["name"] == pinfo.name;
                });
        }

    	bool is_target = false;
    	if (target_list.contains("players")) {
    		const auto& targets = target_list["players"];
    		is_target = std::any_of(targets.begin(), targets.end(),
    			[&pinfo](const nlohmann::json& player) {
    				return player["name"] == pinfo.name;
    			});
    	}

        char display_name[256];
        if (is_friend) {
	        snprintf(display_name, sizeof(display_name), "[FRIEND] %s", pinfo.name);
        }else if (is_target) {
	        snprintf(display_name, sizeof(display_name), "[TARGET] %s", pinfo.name);
        }else {
            strncpy(display_name, pinfo.name, sizeof(display_name) - 1);
            display_name[sizeof(display_name) - 1] = '\0';  // Ensure null termination
        }

        int length = strlen(display_name);
        float width = (length * 13) / 2.f;
    	if (is_target) {
    		ImVec4 redColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    		Drawing::Text(display_name, worldtoscreenent.x - (width / 2.f), worldtoscreenent.y,
						 Drawing::ToColor(&redColor), offset);
    	} else {
    		Drawing::Text(display_name, worldtoscreenent.x - (width / 2.f), worldtoscreenent.y,
						 Drawing::ToColor(&raicu::globals::settings::espValues::nameColor), offset);
    	}
    }
}

void Visuals::DrawTeam(int CurrentEnt, float &offset) {
	c_vector out;
    player_info_t pinfo;
    c_vector worldtoscreenent, currentOrg;

    c_base_entity *current = interfaces::entity_list->get_entity(CurrentEnt);
    currentOrg = current->get_abs_origin();

    if (utilities::world_to_screen(currentOrg, &worldtoscreenent)) {
        interfaces::engine->get_player_info(CurrentEnt, &pinfo);

        if (strlen(pinfo.name) == 0) {
            return;
        }
    	const char* TeamName = lua_utilities::get_team_name(current);
    	int length = strlen(TeamName);
        float width = (length * 13) / 2.f;

    	char* modifiable = new char[strlen(TeamName) + 1];
    	strcpy(modifiable, TeamName);

    	Drawing::Text(modifiable, worldtoscreenent.x - (width / 2.f), worldtoscreenent.y,
						 Drawing::ToColor(&raicu::globals::settings::espValues::nameColor), offset);

		delete[] modifiable;
    }
}

void Visuals::DrawLineToTarget() {
	if (!aimbot::target.entity) {
		return;
	}
	if (!aimbot::target.entity->is_alive()) {
		return;
	}
	if (!raicu::globals::settings::aimbot::hotkey.check())
		return;

	const c_vector origin = aimbot::target.shoot_pos;
	c_vector screen_pos;

	if (!utilities::world_to_screen(origin, &screen_pos))
		return;

	ImGuiIO &io = ImGui::GetIO();

	float snaplineY = io.DisplaySize.y / 2.0f;

	Drawing::Line(screen_pos.x, screen_pos.y, io.DisplaySize.x / 2.0f, snaplineY,
	              Drawing::ToColor(&espValues::snapLineColor), 1.0f);
}

void Visuals::DrawDistance(int CurrentEnt, float &offset, float distance) {
	c_base_entity *entity = interfaces::entity_list->get_entity(CurrentEnt);
	if (!entity)
		return;

	const c_vector origin = entity->get_abs_origin();
	c_vector screen_pos;

	if (utilities::world_to_screen(origin, &screen_pos)) {
		std::string str = std::to_string((int) distance) + "m";

		int length = strlen(str.c_str());
		float width = (length * 13) / 2.f;

		char *newStr = new char[str.size() + 1];
		std::strcpy(newStr, str.c_str());

		Drawing::Text(newStr, screen_pos.x - (width / 2.f), screen_pos.y,
		              Drawing::ToColor(&raicu::globals::settings::espValues::nameColor), offset);

		delete[] newStr;
	}
}

void Visuals::DrawSkeleton(int entityIndex) {
	try {
		if (entityIndex < 0) {
			return;
		}

		c_base_entity *entity = interfaces::entity_list->get_entity(entityIndex);
		if (!entity || !entity->is_alive())
			return;

		auto* renderable = entity->get_client_renderable();
		if (!renderable)
			return;

		void *model = renderable->get_model();
		if (!model)
			return;

		studiohdr_t *studio_hdr = interfaces::model_info->get_studio_model(model);
		if (!studio_hdr)
			return;

		std::array<matrix3x4, 128> bone_matrix = {};
		if (!renderable->setup_bones(bone_matrix.data(),
		                             static_cast<int>(bone_matrix.size()),
		                             BONE_USED_BY_ANYTHING,
		                             interfaces::global_vars->curtime)) {
			return;
		}

		constexpr size_t allBoneNamesSize = sizeof(boneArray) / sizeof(boneArray[0]);

		auto GetBoneIndexByName = [&](const char *boneName) -> int {
			if (!boneName)
				return -1;

			for (size_t i = 0; i < allBoneNamesSize; ++i) {
				if (boneArray[i] && strcmp(boneArray[i], boneName) == 0) {
					if (i >= 128) return -1;

					if (bone_matrix[i][0][3] == 0.0f &&
					    bone_matrix[i][1][3] == 0.0f &&
					    bone_matrix[i][2][3] == 0.0f)
						return -1;

					return static_cast<int>(i);
				}
			}
			return -1;
		};

		for (const auto &connection: bone_connections_named) {
			if (!connection.parent || !connection.child)
				continue;

			int child_index = GetBoneIndexByName(connection.child);
			int parent_index = GetBoneIndexByName(connection.parent);

			// Extra safety check
			if (parent_index == -1 || child_index == -1 ||
			    parent_index >= 128 || child_index >= 128) {
				continue;
			}

			if (std::isnan(bone_matrix[parent_index][0][3]) ||
			    std::isnan(bone_matrix[parent_index][1][3]) ||
			    std::isnan(bone_matrix[parent_index][2][3]) ||
			    std::isnan(bone_matrix[child_index][0][3]) ||
			    std::isnan(bone_matrix[child_index][1][3]) ||
			    std::isnan(bone_matrix[child_index][2][3])) {
				continue;
			}

			c_vector start, end;
			c_vector start_screen, end_screen;

			start = c_vector(
				bone_matrix[parent_index][0][3],
				bone_matrix[parent_index][1][3],
				bone_matrix[parent_index][2][3]
			);

			end = c_vector(
				bone_matrix[child_index][0][3],
				bone_matrix[child_index][1][3],
				bone_matrix[child_index][2][3]
			);

			if (start.length() > 50000.0f || end.length() > 50000.0f) {
				continue;
			}

			if (utilities::world_to_screen(start, &start_screen) &&
			    utilities::world_to_screen(end, &end_screen)) {
				if (std::isnan(start_screen.x) || std::isnan(start_screen.y) ||
				    std::isnan(end_screen.x) || std::isnan(end_screen.y))
					continue;

				Drawing::Line(
					start_screen.x, start_screen.y,
					end_screen.x, end_screen.y,
					Drawing::ToColor(&espValues::skeletonColor),
					espValues::skeletonThickness
				);
			}
		}
	} catch (const std::exception &e) {
		logger::Log(logger::LOGGER_LEVEL_ERROR, "Exception in DrawSkeleton");
	}
	catch (...) {
		logger::Log(logger::LOGGER_LEVEL_ERROR, "Unknown exception in DrawSkeleton");
	}
}

void Visuals::DrawRPJob(int CurrentEnt, float &offset) {
	c_base_entity *entity = interfaces::entity_list->get_entity(CurrentEnt);
	if (!entity)
		return;

	const c_vector origin = entity->get_abs_origin();
	c_vector screen_pos;

	if (utilities::world_to_screen(origin, &screen_pos)) {
		auto str = lua_utilities::get_team_name(entity);

		int length = strlen(str);
		float width = (length * 13) / 2.f;

		char *newStr = new char[strlen(str) + 1];
		strcpy(newStr, str);

		Drawing::Text(newStr, screen_pos.x - (width / 2.f), screen_pos.y,
		              Drawing::ToColor(&raicu::globals::settings::espValues::nameColor), offset);

		delete[] newStr;
	}
}

void Visuals::DrawWeapon(int CurrentEnt, float &offset) {
	c_base_entity *entity = interfaces::entity_list->get_entity(CurrentEnt);
	if (!entity) return;

	const c_vector origin = entity->get_abs_origin();
	c_vector screen_pos;

	if (utilities::world_to_screen(origin, &screen_pos)) {
		c_base_combat_weapon *weapon = entity->get_active_weapon();
		if (!weapon) return;

		const char *weapon_id = lua_utilities::get_weapon_print_name(weapon);
		if (!weapon_id) return;

		const char *originalStr = lua_utilities::language_get_phrase(weapon_id);
		if (!originalStr || originalStr[0] == '\0') return;

		static char buffer[128];
		strncpy(buffer, originalStr, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';

		float width = (strlen(buffer) * 13) / 2.0f;

		Drawing::Text(buffer, screen_pos.x - width, screen_pos.y,
		              Drawing::ToColor(&raicu::globals::settings::espValues::nameColor), offset);
	}
}

void Visuals::DrawSnapline(int entityIndex, ImU32 color) {
	c_base_entity *entity = interfaces::entity_list->get_entity(entityIndex);
	if (!entity || !entity->is_alive())
		return;

	const c_vector origin = entity->get_abs_origin();
	c_vector screen_pos;

	if (!utilities::world_to_screen(origin, &screen_pos))
		return;

	ImGuiIO &io = ImGui::GetIO();

	float snaplineY = 0.0f;

	switch (raicu::globals::settings::espValues::snaplinePosition) {
		case 0: // Bottom
			snaplineY = io.DisplaySize.y;
			break;
		case 1: // Middle
			snaplineY = io.DisplaySize.y / 2.0f;
			break;
		case 2: // Top
			snaplineY = 0.0f;
			break;
	}

	Drawing::Line(screen_pos.x, screen_pos.y, io.DisplaySize.x / 2.0f, snaplineY, color, 1.0f);
}

void Visuals::DrawOrigin(int CurrentEnt, ImU32 color) {
	c_base_entity *entity = interfaces::entity_list->get_entity(CurrentEnt);
	if (!entity) return;

	const c_vector origin = entity->get_abs_origin();
	c_vector screen_pos;

	if (!utilities::world_to_screen(origin, &screen_pos))
		return;

	Drawing::Circle(screen_pos.x, screen_pos.y, 5.f, color, 5, 2.f);
}

void Visuals::DrawBacktrack(int entityIndex) {}

void Visuals::DrawAimbotFOV(float fovSize) {
	ImGuiIO &io = ImGui::GetIO();

	float screenWidth = io.DisplaySize.x;
	float screenHeight = io.DisplaySize.y;

	float camFovDeg = raicu::globals::settings::fov;

	float fovRad = math::deg2rad(fovSize);
	float camFovRad = math::deg2rad(camFovDeg);

	float radius = tanf(fovRad / 2.0f) / tanf(camFovRad / 2.0f) * (screenHeight / 2.0f);

	Drawing::Circle(
		screenWidth / 2.0f,
		screenHeight / 2.0f,
		radius,
		Drawing::ToColor(&other::fovColor),
		100,
		1.0f
	);
}

void Visuals::Drawbox(int CurrentEnt) {
	c_base_entity *entity = interfaces::entity_list->get_entity(CurrentEnt);
	if (!entity)
		return;

	box_t box;
	if (!utilities::get_entity_box(entity, box)) return;

	Drawing::Box(box.left, box.top, box.right, box.bottom,
	             Drawing::ToColor(&raicu::globals::settings::espValues::boxColor));
}

void Visuals::DrawHealth(int CurrentEnt) {
	c_base_entity *entity = interfaces::entity_list->get_entity(CurrentEnt);
	if (!entity)
		return;

	box_t box;
	if (!utilities::get_entity_box(entity, box))
		return;

	int health = entity->get_health();
	if (health <= 0 || health > 100)
		return;

	int bar_width = 4;
	int box_height = box.bottom - box.top;
	float health_height = (health / 100.0f) * box_height;

	int bar_left = box.left - bar_width - 2;
	int bar_right = bar_left + bar_width;
	int bar_top = box.top;
	int bar_bottom = box.bottom;
	int filled_bottom = bar_top + static_cast<int>(health_height);

	ImColor color;
	if (health > 75)
		color = ImColor(0, 255, 0); // Green?
	else if (health > 35)
		color = ImColor(255, 255, 0); // Yellow?
	else
		color = ImColor(255, 0, 0); // Red?

	Drawing::BoxFilled(bar_left, bar_top, bar_right, bar_bottom, ImColor(30, 30, 30, 200));
	Drawing::Box(bar_left, bar_top, bar_right, bar_bottom, ImColor(0, 0, 0, 255));

	Drawing::BoxFilled(bar_left, bar_top, bar_right, filled_bottom, color);
}