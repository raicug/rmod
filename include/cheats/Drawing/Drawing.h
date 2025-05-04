#pragma once
#include <mutex>
#include <iostream>
#include <ext/imgui/imgui.h>
#include <ext/imgui/backends/imgui_impl_dx9.h>
#include <ext/imgui/backends/imgui_impl_win32.h>
#include <SDK/Interface.h>

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

namespace raicu::globals {
	class Drawing {
	public:
		static void Line(float x1, float y1, float x2, float y2, ImU32 color, float thickness = 1.0f);
		static void Circle(float x, float y, float rad, ImU32 color, int segments = 0, float rounding = 0.f);
		
		static void Box(float x, float y, float w, float h, ImU32 color, float thickness = 1.f, float rounding = 0.f);
		static void BoxFilled(float x, float y, float w, float h, ImU32 color, float rounding = 0.f);

		static void OutlineFilledBox(float x, float y, float w, float h, ImU32 color, ImU32 outlineColor, float outlineThickness = 1.f, float rounding = 0.f);
		static void Text(char* text, float x, float y, ImU32 color, float& text_offset);

		static ImU32 ToColor(ImVec4* color);
	};
}