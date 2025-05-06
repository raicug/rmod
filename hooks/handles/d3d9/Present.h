#include "../../hooks.h"

HRESULT APIENTRY raicu::hooks::handles::present(IDirect3DDevice9* device, CONST RECT* src, CONST RECT* dest, HWND wnd_override, CONST RGNDATA* dirty_region) {
    if (!raicu::gui::setup) {
        raicu::gui::SetupMenu(device);
	    return originals::present(device, src, dest, wnd_override, dirty_region);
    }

    device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);

    raicu::gui::Render();

    device->SetRenderState(D3DRS_SRGBWRITEENABLE, TRUE);

    return originals::present(device, src, dest, wnd_override, dirty_region);
}
