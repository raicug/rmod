#include <windows.h>
#include <thread>
#include "logging/logger.h"
#include "gui/gui.h"
#include "hooks/hooks.h"
#include "globals/settings.h"
#include <SDK/utils/utilities.h>

#ifdef BUILDING_DLL
    #define DLLEXPORT __declspec(dllexport)
#else
    #define DLLEXPORT __declspec(dllimport)
#endif


/**
 *
 * TODO:
  * Bug fixes
 *
 * ------------- MENU FEATURES:
 * Undetectable visuals
 *
 * ------------- Cheats:
 * Aimbot
 * -> Silent
 * -> Lock
 * -> Backtrack (???)
 * -> Automatic fire
 * -> Line to shoot pos
 * -> Smoothing
 * -> Ignore friends (TODO)
 * -> Ignore team (TODO)
 * -> Predict Spread
 * -> Hitbox type (Head, Chest, Stomach, Hitscan)
 * -> Priority (FOV, Distance, Health)
 *
 * ESP
 * -> Name
 * -> Origin
 * -> Chams (disabled, doesn't work)
 * -> Box
 * -> Health
 * -> Skeleton (TODO)
 * -> Distance
 * -> Snapline (Top, Center, Bottom of screen)
 * -> Render Distance (def: 15000)
 *
 * Crosshair
 * FOV
 *
 * LUA
 * -> LUA State (Client, Menu)
 *
 */
HANDLE mainThread = nullptr;
bool running = true;

void UninjectSelf(HMODULE instance) {
    running = false;
    raicu::hooks::Destroy();
    raicu::gui::Destroy();
    logger::Log(1, "DLL Uninjected");

    if (mainThread) {
        CloseHandle(mainThread);
        mainThread = nullptr;
    }

    FreeLibraryAndExitThread(instance, 0);
}

void SetupInternal(const HMODULE instance) {
    if (!logger::Initialize()) {
        MessageBoxA(nullptr, "Logger initialization failed!", "Error", MB_OK);
        UninjectSelf(instance);
    }

    while (!utilities::game_is_full_loaded()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    try {
        interfaces::initialize();
        raicu::gui::Setup();
        raicu::hooks::Setup();
    } catch (const std::exception& e) {
        logger::Log(logger::LOGGER_LEVEL_FATAL, e.what());
        MessageBeep(MB_ICONERROR);
        MessageBoxA(nullptr, e.what(), "raicu - Error", MB_OK | MB_ICONEXCLAMATION);
        UninjectSelf(instance);
    }

    while (!GetAsyncKeyState(VK_END)) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    UninjectSelf(instance);
}

DWORD WINAPI Setup(LPVOID instance) {
   SetupInternal(static_cast<HMODULE>(instance));
    return 1;
}

extern "C" {
DLLEXPORT BOOL APIENTRY DllMain(HINSTANCE hModule, const DWORD reason, LPVOID reserved) {
    switch (reason) {
            case DLL_PROCESS_ATTACH: {
                DisableThreadLibraryCalls(hModule);
                // Remove logger::Initialize() from here
                
                mainThread = CreateThread(
                    nullptr, 0,
                    reinterpret_cast<LPTHREAD_START_ROUTINE>(Setup),
                    hModule, 0, nullptr);

                if (!mainThread) {
                    MessageBoxA(nullptr, "Failed to create main thread", "raicu - Error", MB_OK | MB_ICONEXCLAMATION);
                    return FALSE;
                }
                break;
            }

            case DLL_PROCESS_DETACH: {
                running = false;
                if (mainThread) {
                    WaitForSingleObject(mainThread, INFINITE);
                    CloseHandle(mainThread);
                    mainThread = nullptr;
                }
                break;
            }
        }
        return TRUE;
    }
}