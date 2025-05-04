#include "Visuals.h"
#include "globals/settings.h"
#include "logging/logger.h"
#include "cheats/Drawing/Drawing.h"
#include <SDK/utils/utilities.h>
#include <SDK/utils/lua_utilities.h>
#include <SDK/Interface.h>
#include <algorithm>
#include <cheats/aimbot/aimbot.h>

using raicu::cheats::Visuals;
using namespace raicu::globals::settings;
using raicu::globals::Drawing;

void ImU32ToFloat4(ImU32 color, float out[4]) {
	out[0] = ((color >> 0) & 0xFF) / 255.0f; // R
	out[1] = ((color >> 8) & 0xFF) / 255.0f; // G
	out[2] = ((color >> 16) & 0xFF) / 255.0f; // B
	out[3] = ((color >> 24) & 0xFF) / 255.0f; // A
}

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

			if (espValues::snapline) Visuals::DrawSnapline(static_cast<int>(i),
			                                               Drawing::ToColor(&espValues::snapLineColor));
			if (espValues::origin) Visuals::DrawOrigin(static_cast<int>(i), Drawing::ToColor(&espValues::originColor));
			//if (espValues::weapon) Visuals::DrawWeapon(static_cast<int>(i), offset);
			if (espValues::name) Visuals::DrawName(static_cast<int>(i), offset);
			if (espValues::distance) Visuals::DrawDistance(static_cast<int>(i), offset, distance);
			if (espValues::box) Visuals::Drawbox(static_cast<int>(i));
			if (espValues::health) Visuals::DrawHealth(static_cast<int>(i));
			if (globals::settings::aimbot::visualise_target_line) Visuals::DrawLineToTarget();

			ImGui::PopStyleVar();
		}
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

		int length = strlen(pinfo.name);
		float width = (length * 13) / 2.f;
		Drawing::Text(pinfo.name, worldtoscreenent.x - (width / 2.f), worldtoscreenent.y,
		              Drawing::ToColor(&raicu::globals::settings::espValues::nameColor), offset);
	}
}

void Visuals::DrawLineToTarget() {
	if (!aimbot::target.entity) {
		return;
	}
	if (!aimbot::target.entity->is_alive()) {
		return;
	}

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
	if (!entity)
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
	             Drawing::ToColor(&raicu::globals::settings::espValues::nameColor));
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

/* CHAMS VISUALS */

namespace raicu::cheats::Chams {
	i_material *create_material(const char *name, bool flat, bool wireframe, bool add_shine) {
		std::string material_data = std::format(R"("{}"
    {{
        "$basetexture" "vgui/white_additive"
        "$envmap" "{}"
        "$normalmapalphaenvmapmask" "{}"
        "$envmapcontrast" "{}"
        "$model" "1"
        "$flat" "1"
        "$nocull" "0"
        "$selfillum" "1"
        "$halflambert" "1"
        "$nofog" "0"
        "$ignorez" "0"
        "$znearer" "0"
        "$wireframe" "{}"
    }}
    )", flat || wireframe ? "UnlitGeneric" : "VertexLitGeneric", add_shine ? "env_cubemap" : "", add_shine ? 1 : 0,
		                                        add_shine ? 1 : 0, wireframe ? 1 : 0);


		void *kv = key_values::key_values();
		if (!kv) return nullptr;

		key_values::initialize(kv, name);
		if (!key_values::load_from_buffer(kv, name, material_data.c_str()))
			return nullptr;

		i_material *created_material = interfaces::material_system->create_material(name, kv);
		if (!created_material)
			return nullptr;

		created_material->increment_reference_count();

		return created_material;
	}

	void push_material_override(ImU32 color, int material_type) {
		static i_material *textured = Chams::create_material(xorstr("textured_material"), false, false, false);
		static i_material *metal = Chams::create_material(xorstr("metal_material"), false, false, true);
		static i_material *wireframe = Chams::create_material(xorstr("wireframe_material"), false, true, false);
		static i_material *flat = Chams::create_material(xorstr("flat_material"), true, false, false);

		if (!textured || !metal || !wireframe || !flat)
			return;

		interfaces::model_render->suppress_engine_lighting(true);
		interfaces::model_render->setup_lighting(c_vector());

		float col[4];

		ImU32ToFloat4(color, col);

		interfaces::render_view->set_color_modulation(col);
		interfaces::render_view->set_blend(col[3]);

		i_material *material = nullptr;
		switch (material_type) {
			case 0:
				material = textured;
				break;
			case 1:
				material = metal;
				break;
			case 2:
				material = wireframe;
				break;
			case 3:
				material = flat;
				break;
			case 4:
				break;
		}

		interfaces::model_render->forced_material_override(material);
	}

	void pop_material_override() {
		static float clear_color[3] = {
			1.f, 1.f, 1.f
		};

		interfaces::model_render->suppress_engine_lighting(false);

		interfaces::render_view->set_color_modulation(clear_color);
		interfaces::render_view->set_blend(1.f);

		interfaces::model_render->forced_material_override(nullptr);
	}

	void push_ignore_z(bool ignore_z) {
		c_mat_render_context *render_context = interfaces::material_system->get_render_context();
		if (!render_context) return;

		if (ignore_z)
			render_context->depth_range(0.f, 0.f);
	}

	void pop_ignore_z() {
		c_mat_render_context *render_context = interfaces::material_system->get_render_context();
		if (!render_context)
			return;

		render_context->depth_range(0.f, 1.f);
	}
}
