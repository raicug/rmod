#include "../../hooks.h"

#include "ext/imgui/imgui.h"
#include "ext/imgui/backends/imgui_impl_win32.h"
#include "ext/imgui/backends/imgui_impl_dx9.h"

HRESULT APIENTRY raicu::hooks::handles::reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* presentation_parameters) {
    ImGui_ImplDX9_InvalidateDeviceObjects();

    HRESULT result = originals::reset(device, presentation_parameters);

    ImGui_ImplDX9_CreateDeviceObjects();

    return result;
}
