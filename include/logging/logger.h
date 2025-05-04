

#ifndef LOGGER_H
#define LOGGER_H
#include <minwindef.h>
#include <Windows.h>

class logger {
public:
    enum LOGGER_LEVEL {
        LOGGER_LEVEL_DEBUG = 0,
        LOGGER_LEVEL_INFO = 1,
        LOGGER_LEVEL_WARNING = 2,
        LOGGER_LEVEL_ERROR = 3,
        LOGGER_LEVEL_FATAL = 4,
        LOGGER_LEVEL_SUCCESS = 5,
        LOGGER_LEVEL_UNKNOWN
    };


    static bool Initialize();

    static void Log(const int level, const char* message);
    static void Log(LOGGER_LEVEL level, const char* message);

    ~logger();

private:
    static void DebugPrint(const char* message, WORD color);
    static const char* GetLevelString(int level);
};



#endif //LOGGER_H
