#include "misc.h"
#include <globals/settings.h>

void misc_cheats::run() {
	c_base_entity* local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());
	if (!local_player || !local_player->is_alive())
		return;

	/*
	 * if (aimbot::disable_visual_recoil)
		local_player->get_punch_angle() = c_vector();
	 *
	 */
}

void misc_cheats::third_person(c_view_setup& view)
{
	c_base_entity* local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());
	if (!local_player) return;

	if (!raicu::globals::settings::other::third_person)
		return;

	if (!raicu::globals::settings::other::third_person_hotkey.check())
		return;

	interfaces::input->camera_in_third_person = true;

	trace_t trace;
	c_trace_filter filter;
	filter.skip = local_player;

	c_vector dir;
	math::angle_to_vector(view.angles, dir);

	c_vector origin = view.origin + ((dir * -1) * raicu::globals::settings::other::third_person_distance);

	ray_t ray(view.origin, origin);
	interfaces::engine_trace->trace_ray(ray, MASK_SOLID, &filter, &trace);

	view.origin = trace.end;
}

void misc_cheats::free_cam(c_view_setup &setup, c_vector &camera_position) {
	if (camera_position == c_vector(0.f,0.f,0.f))
		camera_position = setup.origin;

	auto speed = raicu::globals::settings::other::freecam_speed;

	c_vector forward, right, up;

	math::angle_to_vectors(setup.angles, forward, right, up);

	const auto cmd = raicu::globals::settings::last_cmd;

	if (cmd.buttons & IN_SPEED) speed *= 5.f;
	if (cmd.buttons & IN_DUCK) speed *= 0.5f;

	if (cmd.buttons & IN_JUMP) camera_position.z += speed;

	if (cmd.buttons & IN_FORWARD) camera_position += forward * speed;
	if (cmd.buttons & IN_BACK) camera_position -= forward * speed;
	if (cmd.buttons & IN_MOVERIGHT) camera_position += right * speed;
	if (cmd.buttons & IN_MOVELEFT) camera_position -= right * speed;

	if (camera_position.is_valid())
		setup.origin = camera_position;
}
