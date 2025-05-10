#pragma once
#include "../../hooks.h"

bool raicu::hooks::handles::override_view(c_client_mode_shared* client_mode_shared, c_view_setup* view) {
    bool value = originals::override_view(client_mode_shared, view);

    if (globals::settings::other::custom_fov)
        view->fov = globals::settings::other::custom_fov_value;

    settings::fov = view->fov;
    settings::view_angles = view->angles;

    return value;
}