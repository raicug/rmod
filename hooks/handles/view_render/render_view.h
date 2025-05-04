#include <SDK/Interface.h>
#include "../../hooks.h"
#include <globals/settings.h>
#include <cheats/misc/misc.h>

void __fastcall raicu::hooks::handles::render_view(i_view_render* view_render, c_view_setup& view, int flags, int to_draw){
    
    D3DMATRIX world_to_view, view_to_projection, world_to_projection, world_to_pixels;

    interfaces::render_view->get_matrices_for_view(view, &world_to_view, &view_to_projection, &world_to_projection, &world_to_pixels);

    raicu::cheats::misc::third_person(view);
    raicu::globals::settings::view_origin = view.origin;
    raicu::globals::settings::fov = view.fov;

    originals::render_view(view_render, view, flags, to_draw);
}