#include <SDK/Interface.h>
#include "../../hooks.h"
#include <globals/settings.h>
#include <cheats/misc/misc.h>

void __fastcall raicu::hooks::handles::render_view(i_view_render* view_render, c_view_setup& view, int flags, int to_draw){

    if (globals::settings::other::custom_view_model_fov)
        view.fov_view_model = globals::settings::other::custom_view_model_fov_value;

    misc_cheats::third_person(view);
    raicu::globals::settings::view_origin = view.origin;

    originals::render_view(view_render, view, flags, to_draw);
}