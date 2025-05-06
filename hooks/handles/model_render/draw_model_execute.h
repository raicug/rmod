#include <SDK/Interface.h>
#include "../../hooks.h"
#include <globals/settings.h>
#include <cheats/Visuals.h>
#include <cheats/Drawing/Drawing.h>
#include <cheats/aimbot/backtrack/history.h>

#include "SDK/utils/utilities.h"

using namespace raicu::cheats::Chams;
using namespace raicu::globals;

void __fastcall raicu::hooks::handles::draw_model_execute(c_model_render* model_render, void* state, model_render_info_t& info, matrix3x4* bone_to_world) {

	c_base_entity* local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());
	if (!local_player) 
		return originals::draw_model_execute(model_render, state, info, bone_to_world);

	c_base_entity* entity = interfaces::entity_list->get_entity(info.entity_index);
	if (!entity)
		return originals::draw_model_execute(model_render, state, info, bone_to_world);

	/* empty for now */

	originals::draw_model_execute(model_render, state, info, bone_to_world);
}