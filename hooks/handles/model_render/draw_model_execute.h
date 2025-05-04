#include <SDK/Interface.h>
#include "../../hooks.h"
#include <globals/settings.h>
#include <cheats/Visuals.h>
#include <cheats/Drawing/Drawing.h>

using namespace raicu::cheats::Chams;
using namespace raicu::globals;

void __fastcall raicu::hooks::handles::draw_model_execute(c_model_render* model_render, void* state, model_render_info_t& info, matrix3x4* bone_to_world) {

	c_base_entity* local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());
	if (!local_player) 
		return originals::draw_model_execute(model_render, state, info, bone_to_world);

	c_base_entity* entity = interfaces::entity_list->get_entity(info.entity_index);
	if (!entity)
		return originals::draw_model_execute(model_render, state, info, bone_to_world);

	if (entity->is_player() && entity->is_alive()) {

		/*if (raicu::globals::settings::espValues::chams) {
			push_ignore_z(raicu::globals::settings::espValues::ignoreWalls);

			push_material_override(Drawing::ToColor(&raicu::globals::settings::espValues::chamsColour), raicu::globals::settings::espValues::ChamsMaterialType);
			
			originals::draw_model_execute(model_render, state, info, bone_to_world);
			
			pop_material_override();

			pop_ignore_z();

			return;
		}*/

	}

	originals::draw_model_execute(model_render, state, info, bone_to_world);
}