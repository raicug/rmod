#ifndef HOOKS_H
#define HOOKS_H

#include "../gui/gui.h"
#include <sdk/Interface.h>

namespace raicu::hooks {
    void Setup();
    void Destroy() noexcept;

    constexpr void* VirtualFunction(void* thisptr, size_t index) noexcept {
		return (*static_cast<void***>(thisptr))[index];
    }

    using EndSceneFn = long(__thiscall*)(void*, IDirect3DDevice9*) noexcept;
    inline EndSceneFn EndSceneOriginal = nullptr;
    long __stdcall EndScene(IDirect3DDevice9* device) noexcept;

    using PresentFn = HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
    inline PresentFn PresentOriginal = nullptr;
    HRESULT __stdcall Present(IDirect3DDevice9* device, const RECT* src, const RECT* dst, HWND hwnd, const RGNDATA* dirty);

    using ResetFn = HRESULT(__thiscall*)(void*, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*) noexcept;
    inline ResetFn ResetOriginal = nullptr;
    HRESULT __stdcall Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept;

    namespace handles {
        inline void __fastcall render_view(i_view_render* view_render, c_view_setup& view, int flags, int to_draw);
        inline void __fastcall paint_traverse(i_panel* panel, v_panel v_panel, bool force_repaint, bool allow_force);
        inline int __fastcall run_string_ex(c_lua_interface* lua, const char* filename, const char* path, const char* string_to_run, bool run, bool show_errors, bool dont_push_errors, bool no_returns);
        inline void __fastcall draw_model_execute(c_model_render* model_render, void* state, model_render_info_t& info, matrix3x4* bone_to_world);
        inline void __fastcall create_move(c_hl_client* client, int sequence_number, float input_sample_frametime, bool active);
        inline void __fastcall frame_stage_notify(c_hl_client* client, client_frame_stage_t stage);
        inline bool write_user_cmd_delta_to_buffer(c_hl_client* client, void* buf, int from, int to, bool is_new_command);
        inline int __fastcall send_datagram(c_net_channel* net_channel, void* datagram);
        inline bool __fastcall send_net_msg(c_net_channel* net_channel, i_net_message& msg, bool force_reliable, bool voice);

        namespace originals {
            inline void(__thiscall* render_view)(i_view_render*, c_view_setup&, int, int);
            inline void(__thiscall* paint_traverse)(i_panel*, v_panel, bool, bool);
            inline int(__thiscall* run_string_ex)(c_lua_interface*, const char*, const char*, const char*, bool, bool, bool, bool);
            inline void(__thiscall* draw_model_execute)(c_model_render*, void*, model_render_info_t&, matrix3x4*);
            inline void(__thiscall* create_move)(c_hl_client*, int, float, bool);
            inline void(__thiscall* frame_stage_notify)(c_hl_client*, client_frame_stage_t);
            inline bool(*write_user_cmd_delta_to_buffer)(c_hl_client*, void*, int, int, bool);
            inline int(__thiscall* send_datagram)(c_net_channel*, void*);
            inline bool(__thiscall* send_net_msg)(c_net_channel*, i_net_message&, bool, bool);
        }
    }
}

#endif //HOOKS_H