#ifndef GUI_H
#define GUI_H

#include <d3d9.h>
#include <string>
#include <iostream>
#include <windows.h>


namespace raicu::gui {
    inline bool open = true;

    inline bool setup = false;
    inline bool imguiInitialized = false;
    inline HWND window = nullptr;
    inline WNDCLASSEX windowClass = { };
    inline WNDPROC originalWindowProcess = nullptr;

    inline LPDIRECT3DDEVICE9 device = nullptr;
    inline LPDIRECT3D9 d3d9 = nullptr;

    bool SetupWindowClass(const char* windowClassName) noexcept;
    void DestroyWindowClass() noexcept;

    bool SetupWindow(const char* windowName) noexcept;
    void DestroyWindow() noexcept;

    bool SetupDirectX() noexcept;
    void DestroyDirectX() noexcept;

    void Setup();

    void SetupMenu(LPDIRECT3DDEVICE9 device) noexcept;
    void Destroy() noexcept;

    void Render() noexcept;

    void AddToLog(const int level, const std::string& message);

    bool WriteToRegistry(const std::wstring& valueName, const std::wstring& value);
    bool ReadFromRegistry(const std::wstring& valueName, std::wstring& value);

    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    bool loginRequest(const std::string& email, const std::string& password, std::string& responseBody);
}



#endif //GUI_H
