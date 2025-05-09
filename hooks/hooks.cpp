#include "hooks.h"
#include <stdexcept>
#include <intrin.h>

#include <ext/minhooks/minhook.h>
#include <ext/imgui/imgui.h>
#include <ext/imgui/backends/imgui_impl_dx9.h>
#include <ext/imgui/backends/imgui_impl_win32.h>

#include <logging/logger.h>

#include <SDK/Interface.h>
#include <globals/settings.h>
#include <../gui/gui.h>

#include "handles/view_render/render_view.h"
#include "handles/panel/paint_traverse.h"
#include "handles/lua_interface/run_string_ex.h"
#include "handles/model_render/draw_model_execute.h"
#include "handles/hl_client/create_move.h"
#include "handles/hl_client/frame_stage_notify.h"
#include "handles/hl_client/write_user_cmd_delta_to_buffer.h"
#include "handles/net_channel/send_datagram.h"
#include "handles/net_channel/send_net_msg.h"
#include <cheats/Visuals.h>

#include "handles/d3d9/present.h"
#include "handles/d3d9/reset.h"

#include "handles/html_panel/paint.h"
#include "handles/html_panel/load_url.h"
#include "handles/client_mode_shared/override_view.h"

#if defined(__GNUC__)
#define GET_RETURN_ADDRESS() __builtin_return_address(0)
#elif defined(_MSC_VER)
#include <intrin.h>
#define GET_RETURN_ADDRESS() _ReturnAddress()
#endif

void raicu::hooks::Setup() {
    if (MH_Initialize() != MB_OK) {
        logger::Log(logger::LOGGER_LEVEL_ERROR, "Unable to initialize minhook");
        throw std::runtime_error("Unable to initialize minhook");
    };

    logger::Log(logger::LOGGER_LEVEL_SUCCESS, "Minhook initialized");

	if (MH_CreateHook((LPVOID)memory::pattern_scanner(xorstr("gameoverlayrenderer64.dll"), xorstr("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 81 EC ? ? ? ? 4C 8B A4 24 ? ? ? ?")), &handles::present, (LPVOID*)&handles::originals::present))
		throw std::runtime_error("Unable to hook Present");
	if (MH_CreateHook((LPVOID)memory::pattern_scanner(xorstr("gameoverlayrenderer64.dll"), xorstr("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 50 48 8B F2 48 8B F9 48 8B D1")), &handles::reset, (LPVOID*)&handles::originals::reset))
		throw std::runtime_error("Unable to hook Reset");

    try {
		// create_move
		if (MH_CreateHook(
			reinterpret_cast<LPVOID>(memory::get_virtual(reinterpret_cast<PVOID**>(interfaces::hl_client), 21)),
			reinterpret_cast<LPVOID>(&handles::create_move),
			reinterpret_cast<LPVOID*>(&handles::originals::create_move)) != MH_OK)
			throw std::runtime_error("Unable to hook create_move from hl_client");

		// write_user_cmd_delta_to_buffer
		if (MH_CreateHook(
			reinterpret_cast<LPVOID>(memory::get_virtual(reinterpret_cast<PVOID**>(interfaces::hl_client), 23)),
			reinterpret_cast<LPVOID>(&handles::write_user_cmd_delta_to_buffer),
			reinterpret_cast<LPVOID*>(&handles::originals::write_user_cmd_delta_to_buffer)) != MH_OK)
			throw std::runtime_error("Unable to hook write_user_cmd_delta_to_buffer from hl_client");

		// frame_stage_notify
		if (MH_CreateHook(
			reinterpret_cast<LPVOID>(memory::get_virtual(reinterpret_cast<PVOID**>(interfaces::hl_client), 35)),
			reinterpret_cast<LPVOID>(&handles::frame_stage_notify),
			reinterpret_cast<LPVOID*>(&handles::originals::frame_stage_notify)) != MH_OK)
			throw std::runtime_error("Unable to hook frame_stage_notify from hl_client");

		// render_view
		if (MH_CreateHook(
			reinterpret_cast<LPVOID>(memory::get_virtual(reinterpret_cast<PVOID**>(interfaces::view_render), 6)),
			reinterpret_cast<LPVOID>(&handles::render_view),
			reinterpret_cast<LPVOID*>(&handles::originals::render_view)) != MH_OK)
			throw std::runtime_error("Unable to hook render_view");

		// paint_traverse
		if (MH_CreateHook(
			reinterpret_cast<LPVOID>(memory::get_virtual(reinterpret_cast<PVOID**>(interfaces::panel), 41)),
			reinterpret_cast<LPVOID>(&handles::paint_traverse),
			reinterpret_cast<LPVOID*>(&handles::originals::paint_traverse)) != MH_OK)
			throw std::runtime_error("Unable to hook paint_traverse");

		// run_string_ex
		if (MH_CreateHook(
			reinterpret_cast<LPVOID>(memory::pattern_scanner(
				xorstr("lua_shared.dll"),
				xorstr("40 55 53 56 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B F1"))),
			reinterpret_cast<LPVOID>(&handles::run_string_ex),
			reinterpret_cast<LPVOID*>(&handles::originals::run_string_ex)) != MH_OK)
			throw std::runtime_error("Unable to hook run_string_ex from lua_shared");

		// draw_model_execute
		if (MH_CreateHook(
			reinterpret_cast<LPVOID>(memory::get_virtual(reinterpret_cast<PVOID**>(interfaces::model_render), 20)),
			reinterpret_cast<LPVOID>(&handles::draw_model_execute),
			reinterpret_cast<LPVOID*>(&handles::originals::draw_model_execute)) != MH_OK)
			throw std::runtime_error("Unable to hook draw_model_execute from model_render");

    	if (MH_CreateHook(
			reinterpret_cast<LPVOID>(memory::get_virtual(reinterpret_cast<PVOID**>(interfaces::client_mode_shared), 16)),
			reinterpret_cast<LPVOID>(&handles::override_view),
			reinterpret_cast<LPVOID*>(&handles::originals::override_view)))
    			throw std::runtime_error("Unable to hook draw_model_execute from model_render");

		// send_net_msg
		if (MH_CreateHook(
			reinterpret_cast<LPVOID>(memory::pattern_scanner(
				xorstr("engine.dll"),
				xorstr("48 89 5C 24 ? 48 89 6C 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 48 8B D9 45 0F B6 F1"))),
			reinterpret_cast<LPVOID>(&handles::send_net_msg),
			reinterpret_cast<LPVOID*>(&handles::originals::send_net_msg)) != MH_OK)
			throw std::runtime_error("Unable to hook send_net_msg from engine");

		// menu paint + load URL
		if (MH_CreateHook(
			    reinterpret_cast<LPVOID>(memory::pattern_scanner(
				    xorstr("menusystem.dll"),
				    xorstr("40 57 48 83 EC 40 48 8B F9"))),
			    reinterpret_cast<LPVOID>(&handles::html_panel_paint),
			    reinterpret_cast<LPVOID *>(&handles::originals::html_panel_paint)) != MH_OK)
			throw std::runtime_error("Unable to hook menu paint from menusystem");

		if (MH_CreateHook(reinterpret_cast<LPVOID>(
			                  memory::relative_to_absolute(
				                  (uintptr_t) memory::pattern_scanner("menusystem.dll",
				                                                      "E8 ? ? ? ? EB 17 48 8B 0D ? ? ? ?"),
				                  1, 5
			                  )), &handles::html_panel_load_url, (LPVOID *) &handles::originals::html_panel_load_url))
			throw std::runtime_error("Unable to hook menu load url from menusystem");

    } catch (const std::exception &error) {
	    logger::Log(logger::LOGGER_LEVEL_FATAL, error.what());
		MessageBeep(MB_ICONERROR);
		MessageBoxA(0, error.what(), "FATAL ERROR", MB_OK | MB_ICONEXCLAMATION);
	}

    if (MH_EnableHook(MH_ALL_HOOKS))
        throw std::runtime_error("Unable to enable all hooks");

    logger::Log(5, "All hooks created");
}


void raicu::hooks::Destroy() noexcept {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);

    if (raicu::globals::settings::overlay_popup_panel)
    {
        interfaces::panel->set_key_board_input_enabled(raicu::globals::settings::overlay_popup_panel, false);
        interfaces::panel->set_mouse_input_enabled(raicu::globals::settings::overlay_popup_panel, false);
    }

    const char *script = xorstr(R"(
		UpdateNewsList([
	      {
	        "Date": "2025-05-09T03:12:00",
	        "ShortName": "may-2025-patch",
	        "Title": "May 2025 Patch",
	        "HeaderImage": "https://files.facepunch.com/rubat/2025/May09-1347-HairyMudpuppy.png",
	        "SummaryHtml": "Stability/security patch",
	        "Url": "https://gmod.facepunch.com/blog/may-2025-patch/",
	        "Tags": "Update"
	      },
	      {
	        "Date": "2025-04-30T03:42:00",
	        "ShortName": "april-2025-patch",
	        "Title": "April 2025 Patch",
	        "HeaderImage": "https://files.facepunch.com/rubat/2025/May02-1336-OutlandishAntelopegroundsquirrel.png",
	        "SummaryHtml": "A small patch",
	        "Url": "https://gmod.facepunch.com/blog/april-2025-patch/",
	        "Tags": "update"
	      },
	      {
	        "Date": "2025-03-26T03:00:00",
	        "ShortName": "march-2025-update",
	        "Title": "March 2025 Update",
	        "HeaderImage": "https://files.facepunch.com/rubat/2025/March26-1211-BlueDuiker.jpg",
	        "SummaryHtml": "Shaders, new content and many other improvements.",
	        "Url": "https://gmod.facepunch.com/blog/march-2025-update/",
	        "Tags": "Update"
	      },
	      {
	        "Date": "2024-10-29T03:00:00",
	        "ShortName": "october-2024-update",
	        "Title": "October 2024 Update",
	        "HeaderImage": "https://files.facepunch.com/rubat/2024/October28-698-PracticalAmericanquarterhorse.jpg",
	        "SummaryHtml": "Optimizations, Half-Life 2 animation fixes, and more.",
	        "Url": "https://gmod.facepunch.com/blog/october-2024-update/",
	        "Tags": "update"
	      },
	      {
	        "Date": "2024-07-31T02:00:00",
	        "ShortName": "july-2024-update",
	        "Title": "July 2024 Update",
	        "HeaderImage": "https://files.facepunch.com/rubat/2024/July29-384-BothQueenconch.jpg",
	        "SummaryHtml": "Crash fixes, new features and much more.",
	        "Url": "https://gmod.facepunch.com/blog/july-2024-update/",
	        "Tags": "update"
	      }
	    ], false)
	)");

    utilities::run_javascript(globals::settings::menu_panel, script);

    MH_Uninitialize();
}