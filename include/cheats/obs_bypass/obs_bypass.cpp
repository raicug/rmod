#include "obs_bypass.h"

#include "globals/settings.h"

void obs_bypass::init() {
    if (render_texture)
        return;

    int width, height;
    interfaces::engine->get_screen_size(width, height);

    interfaces::material_system->begin_render_target_allocation();
    render_texture = interfaces::material_system->create_named_render_target_texture_ex(
        xorstr("_rt_obs_bypass_texture"),
        width,
        height,
        rt_size_full_frame_buffer,
        interfaces::material_system->get_back_buffer_format(),
        material_rt_depth_shared,
        textureflags_clamps | textureflags_clamps | textureflags_rendertarget,
        0
    );
    interfaces::material_system->end_render_target_allocation();
}

void obs_bypass::frame_stage_notify(client_frame_stage_t stage) {
    if (!raicu::globals::settings::other::obs_bypass) return;
    static bool did_init = false;

    if (!did_init && stage == frame_start)
    {
        init();
        did_init = true;
        return;
    }

    // Check if it's Discord's game capture
    HWND foreground = GetForegroundWindow();
    char window_title[256];
    GetWindowTextA(foreground, window_title, sizeof(window_title));

    bool is_discord_capture = strstr(window_title, "Discord") != nullptr;

    if (is_discord_capture || !render_texture || !interfaces::engine->is_in_game())
        return;

    if (stage == frame_render_start)
    {
        c_mat_render_context* render_context = interfaces::material_system->get_render_context();
        if (!render_context)
            return;

        render_context->copy_render_target_to_texture(render_texture);
    }
}

void obs_bypass::render_view(i_view_render* view_render, c_view_setup& view, int flags, int to_draw)
{
    if (!render_texture)
        return;

    if (!interfaces::engine->is_in_game())
        return;

    c_mat_render_context* render_context = interfaces::material_system->get_render_context();
    if (!render_context)
        return;

    render_context->copy_render_target_to_texture(render_texture);
    render_context->begin_render();
    render_context->set_render_target(nullptr);

    in_streamproof_view = true;
    raicu::hooks::handles::originals::render_view(view_render, view, flags, to_draw);
    in_streamproof_view = false;

    render_context->end_render();
    render_context->set_render_target(render_texture);
}