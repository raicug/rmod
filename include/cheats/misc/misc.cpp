#include "misc.h"
#include <globals/settings.h>

using raicu::cheats::misc;

void misc::third_person(c_view_setup& view)
{
	/*c_base_entity* local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());
	if (!local_player) return;

	if (!raicu::globals::settings::other::third_person::enable)
		return;

	interfaces::input->camera_in_third_person = true;

	trace_t trace;
	c_trace_filter filter;
	filter.skip = local_player;

	c_vector dir;
	math::angle_to_vector(view.angles, dir);

	c_vector origin = view.origin + ((dir * -1) * raicu::globals::settings::other::third_person::distance);

	ray_t ray(view.origin, origin);
	interfaces::engine_trace->trace_ray(ray, MASK_SOLID, &filter, &trace);

	view.origin = trace.end;*/
}