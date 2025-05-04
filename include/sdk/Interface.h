#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

class i_view_render;
class c_engine_client;
class i_panel;
class i_render_view;
class c_entity_list;
class c_material_system;
class c_hl_client;
class c_engine_vgui;
class c_model_info;
class i_engine_trace;
class c_lua_shared;
class c_cvar;
class c_model_render;
class i_prediction;
class i_game_movement;
class i_move_helper;
class i_physics_surface_props;
class c_input;
class c_render;
class c_global_vars;
class c_client_state;
class c_uniform_random_stream;
class c_client_mode_shared;

#include <algorithm>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <intrin.h>
#include <mutex>
#include <regex>
#include <format>

#include "nlohmann/json.h"
#include "ext/minhooks/minhook.h"
#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_internal.h"
#include "ext/imgui/backends/imgui_impl_dx9.h"
#include "ext/imgui/backends/imgui_impl_win32.h"

#include "source-sdk/math/vector.h"
#include "source-sdk/math/quaternion.h"
#include "source-sdk/math/radian_euler.h"
#include "source-sdk/math/math.h"

#include "source-sdk/definitions.h"

#include "memory/memory.h"
#include "xorstr/xorstr.h"

#include "source-sdk/studio.h"
#include "source-sdk/c_studio_hdr.h"
#include "source-sdk/tier0/platform.h"
#include "source-sdk/tier1/str_tool.h"
#include "source-sdk/tier1/checksum_md5.h"
#include "source-sdk/tier1/c_utl_memory.h"
#include "source-sdk/tier1/c_utl_vector.h"
#include "source-sdk/tier1/key_values.h"
#include "source-sdk/engine/c_collideable.h"
#include "source-sdk/engine/c_render.h"
#include "source-sdk/anim_state/c_multiplayer_anim_state.h"
#include "source-sdk/anim_state/c_hl2mp_player_anim_state.h"
#include "source-sdk/entity/i_client_renderable.h"
#include "source-sdk/entity/i_client_networkable.h"
#include "source-sdk/entity/c_base_combat_weapon.h"
#include "source-sdk/entity/c_base_entity.h"
#include "source-sdk/vgui/i_panel.h"
#include "source-sdk/vgui/c_engine_vgui.h"
#include "source-sdk/client/c_view_setup.h"
#include "source-sdk/client/i_view_render.h"
#include "source-sdk/client/i_render_view.h"
#include "source-sdk/client/c_client_mode_shared.h"
#include "source-sdk/client/i_game_movement.h"
#include "source-sdk/server/c_entity_list.h"
#include "source-sdk/materialsystem/i_material.h"
#include "source-sdk/materialsystem/i_texture.h"
#include "source-sdk/materialsystem/c_material_system.h"
#include "source-sdk/engine/c_model_info.h"
#include "source-sdk/engine/c_model_render.h"
#include "source-sdk/engine/c_net_channel.h"
#include "source-sdk/engine/i_engine_trace.h"
#include "source-sdk/shared/c_user_cmd.h"
#include "source-sdk/shared/i_move_helper.h"
#include "source-sdk/client/c_input.h"
#include "source-sdk/client/c_hl_client.h"
#include "source-sdk/vstdlib/c_uniform_random_stream.h"
#include "source-sdk/vstdlib/c_cvar.h"
#include "source-sdk/vphysics/i_physics_surface_props.h"
#include "source-sdk/c_global_vars.h"
#include "source-sdk/c_engine_client.h"
#include "source-sdk/c_client_state.h"
#include "source-sdk/i_prediction.h"
#include "source-sdk/lua/shared.h"

namespace interfaces
{
	void initialize();

	inline c_engine_client* engine;
	inline i_panel* panel;
	inline i_render_view* render_view;
	inline c_entity_list* entity_list;
	inline c_material_system* material_system;
	inline c_hl_client* hl_client;
	inline c_engine_vgui* engine_vgui;
	inline c_model_info* model_info;
	inline i_engine_trace* engine_trace;
	inline c_lua_shared* lua_shared;
	inline c_cvar* cvar;
	inline c_model_render* model_render;
	inline i_prediction* prediction;
	inline i_game_movement* game_movement;
	inline i_move_helper* move_helper;
	inline i_physics_surface_props* physics_surface_props;
	inline i_view_render* view_render; 
	inline c_input* input;
	inline c_render* render;
	inline c_global_vars* global_vars;
	inline c_client_state* client_state;
	inline c_uniform_random_stream* random_stream;
	inline c_client_mode_shared* client_mode_shared;
	inline HWND						window;
}