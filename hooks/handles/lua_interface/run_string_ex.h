#include <SDK/Interface.h>
#include "../../hooks.h"
#include <globals/settings.h>
#include <cheats/misc/misc.h>

int __fastcall raicu::hooks::handles::run_string_ex(c_lua_interface* lua, const char* filename, const char* path, const char* string_to_run, bool run, bool show_errors, bool dont_push_errors, bool no_returns) {

	return originals::run_string_ex(lua, filename, path, string_to_run, run, show_errors, dont_push_errors, no_returns);
}