#pragma once
#include "logging/logger.h"

namespace key_values
{
    inline void *key_values()
{
    using key_values_fn = void *(__fastcall *)(unsigned int);
    static void* cached_key_values = nullptr;

    if (!cached_key_values) {
        void* pattern_addr = memory::pattern_scanner(xorstr("client.dll"), xorstr("E8 ? ? ? ? 4C 63 F7"));
        if (!pattern_addr) {
            logger::Log(logger::LOGGER_LEVEL_ERROR, "Failed to get pattern");
            return nullptr;
        }

        key_values_fn key_values = (key_values_fn)memory::relative_to_absolute((uintptr_t)pattern_addr, 1, 6);

        if (!key_values) {
            logger::Log(logger::LOGGER_LEVEL_ERROR, "Failed to get key_values");
            return nullptr;
        }

        cached_key_values = key_values;
    }

    static bool logged = false;
    if (!logged && cached_key_values) {
        logger::Log(logger::LOGGER_LEVEL_INFO, "Successfully gotten key value");
        logged = true;
    }

    return cached_key_values ? ((key_values_fn)cached_key_values)(0x48) : nullptr;
}

    inline void *initialize(void *kv, const char *name)
    {
        using init_fn = void *(__fastcall *)(void *, const char *);

        static init_fn initialize = (init_fn)memory::relative_to_absolute((uintptr_t)memory::pattern_scanner(xorstr("client.dll"), xorstr("E8 ? ? ? ? 48 89 45 1F")), 1, 6);
        if (!initialize)
            throw std::runtime_error("Failed to initialize key_values");

        return initialize(kv, name);
    }

    inline bool load_from_buffer(void *kv, char const *resource_name, const char *buffer, void *file_system = NULL, const char *path_id = NULL)
    {
        using load_from_buffer_fn = bool(__fastcall *)(void *, char const *, const char *, void *, const char *, void *);

        static load_from_buffer_fn load_from_buffer = (load_from_buffer_fn)memory::relative_to_absolute((uintptr_t)memory::pattern_scanner(xorstr("client.dll"), xorstr("E8 ? ? ? ? 0F B6 D8 FF 15 ? ? ? ?")), 1, 6);
        if (!load_from_buffer)
            throw std::runtime_error("Failed to get load_from_buffer");

        return load_from_buffer(kv, resource_name, buffer, file_system, path_id, 0);
    }
}
