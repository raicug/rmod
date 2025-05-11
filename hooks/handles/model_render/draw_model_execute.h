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

	c_base_entity* local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());
	if (!local_player) 
		return originals::draw_model_execute(model_render, state, info, bone_to_world);

	c_base_entity* entity = interfaces::entity_list->get_entity(info.entity_index);
	if (!entity)
		return originals::draw_model_execute(model_render, state, info, bone_to_world);

	if (entity == local_player) {
	} else if (entity->is_player() && entity->is_alive()) {
		if (settings::aimbot::backtrackEnabled) {
			lag_record record;
			if (history::get_latest_record(info.entity_index, record)) {
				Chams::push_material_override(Drawing::ToColor(&settings::aimbot::backtrackColor),
				                              settings::aimbot::backtrackMaterial);
				hooks::handles::originals::draw_model_execute(model_render, state, info, bone_to_world);
				Chams::pop_material_override();
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
	}

	originals::draw_model_execute(model_render, state, info, bone_to_world);
}