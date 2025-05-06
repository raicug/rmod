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
  * Ignore friends/team
  * Fix skeleton ESP
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

LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo) {
    char error_msg[4096];
    char detailed_log[8192];
    const char* exception_type = "Unknown Exception";

    switch (pExceptionInfo->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
            exception_type = "Access Violation";
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            exception_type = "Array Bounds Exceeded";
            break;
        case EXCEPTION_BREAKPOINT:
            exception_type = "Breakpoint";
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            exception_type = "Datatype Misalignment";
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            exception_type = "Float Denormal Operand";
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            exception_type = "Float Divide by Zero";
            break;
        case EXCEPTION_FLT_INEXACT_RESULT:
            exception_type = "Float Inexact Result";
            break;
        case EXCEPTION_FLT_INVALID_OPERATION:
            exception_type = "Float Invalid Operation";
            break;
        case EXCEPTION_FLT_OVERFLOW:
            exception_type = "Float Overflow";
            break;
        case EXCEPTION_FLT_STACK_CHECK:
            exception_type = "Float Stack Check";
            break;
        case EXCEPTION_FLT_UNDERFLOW:
            exception_type = "Float Underflow";
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            exception_type = "Illegal Instruction";
            break;
        case EXCEPTION_IN_PAGE_ERROR:
            exception_type = "In Page Error";
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            exception_type = "Integer Divide by Zero";
            break;
        case EXCEPTION_INT_OVERFLOW:
            exception_type = "Integer Overflow";
            break;
        case EXCEPTION_INVALID_DISPOSITION:
            exception_type = "Invalid Disposition";
            break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            exception_type = "Noncontinuable Exception";
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            exception_type = "Privileged Instruction";
            break;
        case EXCEPTION_STACK_OVERFLOW:
            exception_type = "Stack Overflow";
            break;
    }

    time_t now = time(nullptr);
    char timestamp[26];
    ctime_s(timestamp, sizeof(timestamp), &now);
    timestamp[24] = '\0';

    snprintf(error_msg, sizeof(error_msg),
        "Fatal Error Detected!\n"
        "Time: %s\n"
        "Type: %s (0x%08X)\n"
        "Location: 0x%p\n",
        timestamp,
        exception_type,
        pExceptionInfo->ExceptionRecord->ExceptionCode,
        pExceptionInfo->ExceptionRecord->ExceptionAddress
    );

    if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        const char* operation = (pExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 0) ? "Reading" :
                              (pExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 1) ? "Writing" : "Executing";

        snprintf(error_msg + strlen(error_msg), sizeof(error_msg) - strlen(error_msg),
            "Access Violation: %s location 0x%p\n",
            operation,
            (void*)pExceptionInfo->ExceptionRecord->ExceptionInformation[1]
        );
    }

    #ifdef _WIN64
    snprintf(error_msg + strlen(error_msg), sizeof(error_msg) - strlen(error_msg),
        "\nRegister State:\n"
        "RAX: 0x%016llX\n"
        "RBX: 0x%016llX\n"
        "RCX: 0x%016llX\n"
        "RDX: 0x%016llX\n"
        "RSI: 0x%016llX\n"
        "RDI: 0x%016llX\n"
        "RBP: 0x%016llX\n"
        "RSP: 0x%016llX\n"
        "RIP: 0x%016llX\n"
        "R8:  0x%016llX\n"
        "R9:  0x%016llX\n"
        "R10: 0x%016llX\n"
        "R11: 0x%016llX\n"
        "R12: 0x%016llX\n"
        "R13: 0x%016llX\n"
        "R14: 0x%016llX\n"
        "R15: 0x%016llX\n"
        "FLAGS: 0x%016llX\n",
        pExceptionInfo->ContextRecord->Rax,
        pExceptionInfo->ContextRecord->Rbx,
        pExceptionInfo->ContextRecord->Rcx,
        pExceptionInfo->ContextRecord->Rdx,
        pExceptionInfo->ContextRecord->Rsi,
        pExceptionInfo->ContextRecord->Rdi,
        pExceptionInfo->ContextRecord->Rbp,
        pExceptionInfo->ContextRecord->Rsp,
        pExceptionInfo->ContextRecord->Rip,
        pExceptionInfo->ContextRecord->R8,
        pExceptionInfo->ContextRecord->R9,
        pExceptionInfo->ContextRecord->R10,
        pExceptionInfo->ContextRecord->R11,
        pExceptionInfo->ContextRecord->R12,
        pExceptionInfo->ContextRecord->R13,
        pExceptionInfo->ContextRecord->R14,
        pExceptionInfo->ContextRecord->R15,
        pExceptionInfo->ContextRecord->EFlags
    );
    #else
    snprintf(error_msg + strlen(error_msg), sizeof(error_msg) - strlen(error_msg),
        "\nRegister State:\n"
        "EAX: 0x%08X\n"
        "EBX: 0x%08X\n"
        "ECX: 0x%08X\n"
        "EDX: 0x%08X\n"
        "ESI: 0x%08X\n"
        "EDI: 0x%08X\n"
        "EBP: 0x%08X\n"
        "ESP: 0x%08X\n"
        "EIP: 0x%08X\n"
        "FLAGS: 0x%08X\n",
        pExceptionInfo->ContextRecord->Eax,
        pExceptionInfo->ContextRecord->Ebx,
        pExceptionInfo->ContextRecord->Ecx,
        pExceptionInfo->ContextRecord->Edx,
        pExceptionInfo->ContextRecord->Esi,
        pExceptionInfo->ContextRecord->Edi,
        pExceptionInfo->ContextRecord->Ebp,
        pExceptionInfo->ContextRecord->Esp,
        pExceptionInfo->ContextRecord->Eip,
        pExceptionInfo->ContextRecord->EFlags
    );
    #endif

    snprintf(detailed_log, sizeof(detailed_log),
        "%s\n"
        "Additional Information:\n"
        "Number of Parameters: %lu\n",
        error_msg,
        pExceptionInfo->ExceptionRecord->NumberParameters
    );

    logger::Log(logger::LOGGER_LEVEL_FATAL, detailed_log);

    MessageBoxA(nullptr, error_msg, "raicu - Fatal Error", MB_OK | MB_ICONERROR);

    return EXCEPTION_EXECUTE_HANDLER;
}


void SetupInternal(const HMODULE instance) {
    SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

    try {
        if (!logger::Initialize()) {
            MessageBoxA(nullptr, "Logger initialization failed!", "Error", MB_OK);
            UninjectSelf(instance);
            return;
        }

        while (!utilities::game_is_full_loaded()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        interfaces::initialize();
        raicu::gui::Setup();
        raicu::hooks::Setup();

        while (!GetAsyncKeyState(VK_END)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

    } catch (const std::exception& e) {
        logger::Log(logger::LOGGER_LEVEL_FATAL, e.what());
        MessageBeep(MB_ICONERROR);
        MessageBoxA(nullptr, e.what(), "raicu - Error", MB_OK | MB_ICONEXCLAMATION);
        UninjectSelf(instance);
        return;
    } catch (...) {
        const char* error_msg = "Unknown exception occurred";
        logger::Log(logger::LOGGER_LEVEL_FATAL, error_msg);
        MessageBoxA(nullptr, error_msg, "raicu - Error", MB_OK | MB_ICONERROR);
        UninjectSelf(instance);
        return;
    }

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