#include "../../hooks.h"

LPDIRECT3DTEXTURE9 g_pRenderTarget = nullptr;
LPDIRECT3DSURFACE9 g_pRenderSurface = nullptr;

bool IsBeingCaptured() {
    static DWORD last_check = 0;
    static bool is_captured = false;

    // Only check every 500ms to avoid performance impact
    DWORD current_time = GetTickCount();
    if (current_time - last_check < 500) {
        return is_captured;
    }

    last_check = current_time;
    is_captured = false;

    auto enum_proc = [](HWND hwnd, LPARAM lParam) -> BOOL {
        char window_title[256];
        GetWindowTextA(hwnd, window_title, sizeof(window_title));

        char window_class[256];
        GetClassNameA(hwnd, window_class, sizeof(window_class));

        if (strcmp(window_class, "Discord.exe") == 0 &&
            IsWindowVisible(hwnd) &&
            (strstr(window_title, "Discord") != nullptr ||
             strstr(window_title, "Screen Share") != nullptr)) {
            RECT rect;
            if (GetWindowRect(hwnd, &rect)) {
                if ((rect.right - rect.left) > 100 && (rect.bottom - rect.top) > 100) {
                    *(bool*)lParam = true;
                    return FALSE;
                }
            }
             }
        return TRUE;
    };

    EnumWindows(enum_proc, (LPARAM)&is_captured);
    return is_captured;
}

HRESULT APIENTRY raicu::hooks::handles::present(IDirect3DDevice9* device, CONST RECT* src, CONST RECT* dest, HWND wnd_override, CONST RGNDATA* dirty_region) {
    if (!raicu::gui::setup) {
        raicu::gui::SetupMenu(device);
        return originals::present(device, src, dest, wnd_override, dirty_region);
    }

    IDirect3DStateBlock9* stateBlock = nullptr;
    device->CreateStateBlock(D3DSBT_ALL, &stateBlock);
    if (stateBlock)
        stateBlock->Capture();

    LPDIRECT3DSURFACE9 backbuffer;
    device->GetRenderTarget(0, &backbuffer);

    if (!g_pRenderTarget) {
        D3DSURFACE_DESC desc;
        backbuffer->GetDesc(&desc);

        device->CreateTexture(
            desc.Width,
            desc.Height,
            1,
            D3DUSAGE_RENDERTARGET,
            D3DFMT_A2R10G10B10,
            D3DPOOL_DEFAULT,
            &g_pRenderTarget,
            nullptr
        );

        if (g_pRenderTarget)
            g_pRenderTarget->GetSurfaceLevel(0, &g_pRenderSurface);
    }

    if (g_pRenderSurface) {
        device->SetRenderTarget(0, g_pRenderSurface);

        device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
        device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
        device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
        device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        device->SetRenderState(D3DRS_LIGHTING, FALSE);
        device->SetRenderState(D3DRS_ZENABLE, FALSE);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        raicu::gui::Render();

        device->SetRenderTarget(0, backbuffer);

        device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

        device->SetTexture(0, g_pRenderTarget);
    }

    if (stateBlock) {
        stateBlock->Apply();
        stateBlock->Release();
    }

    if (backbuffer)
        backbuffer->Release();

    return originals::present(device, src, dest, wnd_override, dirty_region);
}