#pragma once
#include "logging/logger.h"

namespace key_values
{
    inline void* key_values()
    {
        using key_values_fn = void* (__fastcall*)(unsigned int);

        auto pattern_result = memory::pattern_scanner(xorstr("client.dll"), xorstr("E8 ? ? ? ? 4C 63 F7"));
        logger::Log(logger::LOGGER_LEVEL_DEBUG, ("Pattern scan result gotten"));

        if (!pattern_result) {
            logger::Log(logger::LOGGER_LEVEL_ERROR, "Pattern scanning failed");
            throw std::runtime_error("Pattern scanning failed");
        }

        auto abs_addr = memory::relative_to_absolute((uintptr_t)pattern_result, 1, 6);
        logger::Log(logger::LOGGER_LEVEL_DEBUG, ("Absolute address: " + std::to_string(abs_addr)).c_str());

        static key_values_fn key_values = (key_values_fn)abs_addr;
        logger::Log(logger::LOGGER_LEVEL_DEBUG, ("Got key_values function at: " + std::to_string(int((void *) key_values))).c_str());

        if (!key_values)
            throw std::runtime_error("Failed to find key_values");

        logger::Log(logger::LOGGER_LEVEL_DEBUG, "Calling key_values with parameter 0x48");
        return key_values(0x48);
    }

    inline void* initialize(void* kv, const char* name)
    {
        using init_fn = void* (__fastcall*)(void*, const char*);

        static init_fn initialize = (init_fn)memory::relative_to_absolute((uintptr_t)memory::pattern_scanner(xorstr("client.dll"), xorstr("E8 ? ? ? ? 48 89 45 1F")), 1, 6);
        if (!initialize)
            throw std::runtime_error("Failed to find key_values::initialize");

        return initialize(kv, name);
    }

    inline bool load_from_buffer(void* kv, char const* resource_name, const char* buffer, void* file_system = NULL, const char* path_id = NULL)
    {
        using load_from_buffer_fn = bool(__fastcall*)(void*, char const*, const char*, void*, const char*, void*);

        static load_from_buffer_fn load_from_buffer = (load_from_buffer_fn)memory::relative_to_absolute((uintptr_t)memory::pattern_scanner(xorstr("client.dll"), xorstr("E8 ? ? ? ? 0F B6 D8 FF 15 ? ? ? ?")), 1, 6);
        if (!load_from_buffer)
            throw std::runtime_error("Failed to find key_values::load_from_buffer");

        return load_from_buffer(kv, resource_name, buffer, file_system, path_id, 0);
    }
}
