#pragma once
#include <ext/imgui/imgui.h>
#include <string>
#include "sdk/Interface.h"

namespace raicu::cheats {
	namespace Chams {
		i_material* create_material(const char* name, bool flat, bool wireframe, bool add_shine);
		void push_material_override(ImU32 color, int material_type);
		void pop_material_override();

		void push_ignore_z(bool ignore_z);
		void pop_ignore_z();
	};
}