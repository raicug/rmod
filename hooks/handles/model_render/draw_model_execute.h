#include <SDK/Interface.h>
#include "../../hooks.h"
#include <globals/settings.h>
#include <cheats/Visuals.h>
#include <cheats/Drawing/Drawing.h>
#include <cheats/aimbot/backtrack/history.h>

#include "cheats/Visuals/Chams.h"
#include "SDK/utils/utilities.h"

using namespace raicu::cheats;
using namespace raicu::globals;

void __fastcall raicu::hooks::handles::draw_model_execute(c_model_render* model_render, void* state, model_render_info_t& info, matrix3x4* bone_to_world) {

	if (obs_bypass::in_streamproof_view)
		return originals::draw_model_execute(model_render, state, info, bone_to_world);

	c_base_entity* local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());
	if (!local_player) 
		return originals::draw_model_execute(model_render, state, info, bone_to_world);

	c_base_entity* entity = interfaces::entity_list->get_entity(info.entity_index);
	if (!entity)
		return originals::draw_model_execute(model_render, state, info, bone_to_world);

	if (entity == local_player) {
	} else if (entity->is_player() && entity->is_alive()) {
		if (settings::aimbot::backtrackEnabled) {
			if (history::records.count(info.entity_index) > 0) {  // Check if entity index exists in records
			    auto& track = history::records[info.entity_index];
			    if (!track.empty()) {
			        float current_time = utilities::ticks_to_time(interfaces::global_vars->tick_count);

			        for (const auto& record : track) {
			            // Validate record data
			            if (!record.bone_to_world || !record.bone_to_world.get()) {
			                continue;
			            }

			            float time_difference = current_time - record.arrive_time;
			            if (time_difference > globals::settings::aimbot::backtrack)
			                continue;

			            if (!history::can_restore_to_simulation_time(record.simulation_time))
			                continue;

			            float alpha = 1.0f - (time_difference / globals::settings::aimbot::backtrack);
			            alpha = std::clamp(alpha, 0.2f, 1.0f);

			            ImColor color = Drawing::ToColor(&settings::aimbot::backtrackColor);
			            color.Value.w = alpha;

			            // Additional safety check before rendering
			            if (interfaces::model_render && model_render) {
			                Chams::push_material_override(color, settings::aimbot::backtrackMaterial);
			                hooks::handles::originals::draw_model_execute(model_render, state, info, record.bone_to_world.get());
			                Chams::pop_material_override();
			            }
			        }
			    }
			}
		}

		if (settings::chams::enabled) {
			Chams::push_ignore_z(settings::chams::ignore_walls);

			if (settings::chams::draw_original_model)
				hooks::handles::originals::draw_model_execute(model_render, state, info, bone_to_world);

			Chams::push_material_override(Drawing::ToColor(&settings::chams::playerColor),
			                              settings::chams::material_type);
			hooks::handles::originals::draw_model_execute(model_render, state, info, bone_to_world);
			Chams::pop_material_override();

			Chams::pop_ignore_z();

			return;
		}
	} else if (entity == interfaces::entity_list->get_entity_from_handle(local_player->get_hands()) || entity == interfaces::entity_list->get_entity_from_handle(local_player->get_view_model())) {
		if (settings::chams::localplr::hands) {
			if (settings::chams::localplr::hands_draw_original_model)
				hooks::handles::originals::draw_model_execute(model_render, state, info, bone_to_world);

			Chams::push_material_override(Drawing::ToColor(&settings::chams::localplr::hands_color), settings::chams::localplr::hands_material_type);
			hooks::handles::originals::draw_model_execute(model_render, state, info, bone_to_world);
			Chams::pop_material_override();

			return;
		}
	}

	originals::draw_model_execute(model_render, state, info, bone_to_world);
}