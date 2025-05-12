#include "lua.h"
#include <globals/offsets.h>
#include <logging/logger.h>
#include <globals/settings.h>

using namespace raicu;

namespace lua {
    ExecutionData executionData;

    void execute() {
        if (!executionData.waiting.load()) {
            return;
        }

        std::string scriptCopy;
        {
            std::lock_guard<std::mutex> lock(executionData.mutex);
            scriptCopy = executionData.script;
            executionData.waiting.store(false);
        }

        int stateInt = raicu::globals::settings::lua::executeState;
        if (stateInt != 0 && stateInt != 2) {
            logger::Log(logger::LOGGER_LEVEL_ERROR, "Invalid execute state");
            return;
        }

        lua_type_t lua_type = static_cast<lua_type_t>(stateInt);

        c_lua_interface* lua = interfaces::lua_shared->get_interface(lua_type);
        if (!lua) {
            return;
        }

        lua->run_string("", "", scriptCopy.c_str());
    }

    void dumper(const std::string& filename, const std::string& string_to_run) {
        if (!globals::settings::other::dumper)
            return;

        c_net_channel* net_c = interfaces::engine->get_net_channel();
        if (!net_c)
            return;

        std::string address = net_c->get_address();
        std::replace(address.begin(), address.end(), ':', '_');
        std::replace(address.begin(), address.end(), '.', '-');

        std::regex forbidden(xorstr("[^a-zA-Z0-9_\\-]"));
        std::regex_replace(filename, forbidden, "");

        std::filesystem::path path = xorstr("C:/R-GMOD/lua-dumps/") + address + xorstr("/");
        path /= filename;

        if (!std::filesystem::exists(path.parent_path()) || !std::filesystem::is_directory(path.parent_path()))
            std::filesystem::create_directories(path.parent_path());

        std::ofstream file(path);

        if (!file.is_open())
            return;

        file << string_to_run << std::endl;
        file.close();
    }
}
