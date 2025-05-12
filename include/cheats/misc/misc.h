#pragma once
#include "../../../hooks/hooks.h"

namespace misc_cheats {
    void run();
    void third_person(c_view_setup& view);
    void free_cam(c_view_setup& setup, c_vector& camera_position);
}