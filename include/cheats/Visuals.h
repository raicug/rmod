#pragma once
#include <ext/imgui/imgui.h>
#include <string>
#include "sdk/Interface.h"

namespace raicu::cheats {
	class Visuals {
	public:
		static void Render();
	private:
		static void DrawCrosshair(float width, float length, float offset, ImU32 color, float rounding, bool outline, ImU32 outlineColor, float outlineThickness);
		static void DrawAimbotFOV(float FOV);

		static void Drawbox(int CurrentEnt);

		static void DrawHealth(int CurrentEnt);

		static void DrawName(int CurrentEnt, float& offset);
		static void DrawDistance(int CurrentEnt, float& offset, float distance);
		static void DrawRPJob(int CurrentEnt, float& offset);
		static void DrawWeapon(int CurrentEnt, float& offset);

		static void DrawSnapline(int CurrentEnt, ImU32 color);
		static void DrawOrigin(int CurrentEnt, ImU32 color);

		static void DrawLineToTarget();
	};

	namespace Chams {
		i_material* create_material(const char* name, bool flat, bool wireframe, bool add_shine);
		void push_material_override(ImU32 color, int material_type);
		void pop_material_override();

		void push_ignore_z(bool ignore_z);
		void pop_ignore_z();
	};

}