#include "logger.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <thread>
#include <Windows.h>
#include "../../gui/gui.h"

namespace fs = std::filesystem;
std::ofstream logFile;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

#if defined(__GNUC__)
#define GET_RETURN_ADDRESS() __builtin_return_address(0)
#elif defined(_MSC_VER)
#include <intrin.h>
#define GET_RETURN_ADDRESS() _ReturnAddress()
#endif

void SetConsoleColor(WORD color) {
    SetConsoleTextAttribute(hConsole, color);
}

void logger::DebugPrint(const char* message, WORD color) {
    HMODULE hModule = GetModuleHandle(NULL);
    DWORD threadId = GetCurrentThreadId();

    // Get current time
    time_t now = std::time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

    // Save current color
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    // Set new color
    SetConsoleColor(color);

    // Print in x64dbg style
    printf("%IX | %04d:%04X | %s | %-50s | %p\n",
           (uintptr_t)hModule,                    // Module base
           GetCurrentProcessId(),                 // Process ID
           threadId,                             // Thread ID
           timeStr,                              // Time
           message,                              // Message
           GET_RETURN_ADDRESS());                // Return address

    // Restore original color
    SetConsoleTextAttribute(hConsole, csbi.wAttributes);
}

bool logger::Initialize() {
    try {
        const fs::path logPath = "C:\\R-GMOD\\logs";
        if (!fs::exists(logPath)) {
            if (!fs::create_directories(logPath)) {
                MessageBoxA(nullptr, "Failed to create path for logs", "Failure", MB_OK);
                return false;
            }
        }

        time_t now = std::time(nullptr);
        struct tm timeinfo;
        localtime_s(&timeinfo, &now);

        std::ostringstream filename;
        filename << logPath.string() << "\\";
        char dateStr[32];
        strftime(dateStr, sizeof(dateStr), "%Y-%m-%d_%H-%M-%S", &timeinfo);
        filename << dateStr << ".log";

        logFile.open(filename.str(), std::ios::app);

        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename.str() << std::endl;
            return false;
        }

        Log(5, "Logger initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Failure", MB_OK);
        return false;
    }
}

void logger::Log(LOGGER_LEVEL level, const char* message) {
    Log(static_cast<int>(level), message);
}

void logger::Log(const int level, const char* message) {
    raicu::gui::AddToLog(level, message);

    if (!logFile.is_open()) {
        MessageBoxA(nullptr, "Log file is not open!", "Failure", MB_OK);
        return;
    }

    time_t now = std::time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%d/%m/%Y - %H:%M:%S", &timeinfo);

    const char* levelStr;
    switch (level) {
        case LOGGER_LEVEL_DEBUG: levelStr = "DEBUG"; break;
        case LOGGER_LEVEL_INFO: levelStr = "INFO"; break;
        case LOGGER_LEVEL_WARNING: levelStr = "WARNING"; break;
        case LOGGER_LEVEL_ERROR: levelStr = "ERROR"; break;
        case LOGGER_LEVEL_FATAL: levelStr = "FATAL"; break;
        case LOGGER_LEVEL_SUCCESS: levelStr = "SUCCESS"; break;
        default: levelStr = "UNKNOWN"; break;
    }

    logFile << timeStr << " [" << levelStr << "] " << message << std::endl;
    logFile.flush();

    std::cout << timeStr << " [" << levelStr << "] " << message << std::endl;
}

logger::~logger() {
    if (logFile.is_open()) {
        Log(1, "Logger shutting down");
        logFile.close();
    }
}