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
}
