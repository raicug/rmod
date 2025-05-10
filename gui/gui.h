#ifndef GUI_H
#define GUI_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <d3d9.h>

#include <string>
#include <iostream>
#include <windows.h>

#include "sdk/Interface.h"

struct hotkey_t;

namespace raicu::gui {
    inline bool open = true;
	static i_texture* rt = nullptr;

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

    namespace other {
        void hotkey(const char* label, hotkey_t* hotkey);
        const char* const key_names[] =
        {
            "UNK",
            "MOUSE_L",
            "MOUSE_R",
            "CANCEL",
            "MOUSE_3",
            "MOUSE_4",
            "MOUSE_5",
            "UNK",
            "BACK",
            "TAB",
            "UNK",
            "UNK",
            "CLEAR",
            "RETURN",
            "UNK",
            "UNK",
            "SHIFT",
            "CONTROL",
            "MENU",
            "PAUSE",
            "CAPITAL",
            "KANA",
            "UNK",
            "JUNJA",
            "FINAL",
            "KANJI",
            "UNK",
            "ESCAPE",
            "CONVERT",
            "NONCONVERT",
            "ACCEPT",
            "MODECHANGE",
            "SPACE",
            "PRIOR",
            "NEXT",
            "END",
            "HOME",
            "LEFT",
            "UP",
            "RIGHT",
            "DOWN",
            "SELECT",
            "PRINT",
            "EXECUTE",
            "SNAPSHOT",
            "INSERT",
            "DELETE",
            "HELP",
            "0",
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "A",
            "B",
            "C",
            "D",
            "E",
            "F",
            "G",
            "H",
            "I",
            "J",
            "K",
            "L",
            "M",
            "N",
            "O",
            "P",
            "Q",
            "R",
            "S",
            "T",
            "U",
            "V",
            "W",
            "X",
            "Y",
            "Z",
            "LWIN",
            "RWIN",
            "APPS",
            "UNK",
            "SLEEP",
            "0",
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "MULTIPLY",
            "ADD",
            "SEPARATOR",
            "SUBTRACT",
            "DECIMAL",
            "DIVIDE",
            "F1",
            "F2",
            "F3",
            "F4",
            "F5",
            "F6",
            "F7",
            "F8",
            "F9",
            "F10",
            "F11",
            "F12",
            "F13",
            "F14",
            "F15",
            "F16",
            "F17",
            "F18",
            "F19",
            "F20",
            "F21",
            "F22",
            "F23",
            "F24",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "NUMLOCK",
            "SCROLL",
            "OEM_NEC_EQUAL",
            "OEM_FJ_MASSHOU",
            "OEM_FJ_TOUROKU",
            "OEM_FJ_LOYA",
            "OEM_FJ_ROYA",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "UNK",
            "LSHIFT",
            "RSHIFT",
            "LCONTROL",
            "RCONTROL",
            "LMENU",
            "RMENU"
        };
    }
}



#endif //GUI_H
