#include <SDK/Interface.h>
#include "../../hooks.h"
#include <globals/settings.h>
#include <cheats/Visuals.h>
#include <cheats/lua/lua.h>

void __fastcall raicu::hooks::handles::paint_traverse(i_panel* panel, v_panel v_panel, bool force_repaint, bool allow_force) {
	originals::paint_traverse(panel, v_panel, force_repaint, allow_force);

	const char* name = interfaces::panel->get_name(v_panel);

	if (!raicu::globals::settings::overlay_popup_panel)
	{
		if (strstr(name, xorstr("OverlayPopupPanel")))
			raicu::globals::settings::overlay_popup_panel = v_panel;
	}
	else if (raicu::globals::settings::overlay_popup_panel == v_panel)
	{
		interfaces::panel->set_key_board_input_enabled(v_panel, raicu::globals::settings::open);
		interfaces::panel->set_mouse_input_enabled(v_panel, raicu::globals::settings::open);
	}

	if (!raicu::globals::settings::mat_system_top_panel)
	{
		if (strstr(name, xorstr("MatSystemTopPanel")))
			raicu::globals::settings::mat_system_top_panel = v_panel;
	}
	else if (raicu::globals::settings::mat_system_top_panel == v_panel)
	{
		if (interfaces::engine->is_in_game())
		{
			D3DMATRIX world_to_view, view_to_projection, world_to_pixels;
			interfaces::render_view->get_matrices_for_view(*interfaces::view_render->get_view_setup(), &world_to_view, &view_to_projection, &raicu::globals::settings::world_to_screen, &world_to_pixels);
		}
	}

	if (!raicu::globals::settings::focus_overlay_panel)
	{
		if (strstr(name, xorstr("FocusOverlayPanel")))
			raicu::globals::settings::focus_overlay_panel = v_panel;
	}
	else if (raicu::globals::settings::focus_overlay_panel == v_panel)
	{
		// cant do visuals here
		lua::execute();
	}
}