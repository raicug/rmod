#include "Drawing.h"

#include <SDK/Interface.h>
#include <logging/logger.h>
#include <globals/settings.h>

using namespace raicu::globals;

void Drawing::Line(float x1, float y1, float x2, float y2, ImU32 color, float thickness) {
	auto draw = ImGui::GetBackgroundDrawList();
	draw->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, thickness);
}

void Drawing::Circle(float x, float y, float rad, ImU32 color, int segments, float thickness) {
	auto draw = ImGui::GetBackgroundDrawList();
	draw->AddCircle(ImVec2(x,y), rad, color, segments, thickness);
}

void Drawing::Box(float x, float y, float w, float h, ImU32 color, float thickness, float rounding) {
	auto draw = ImGui::GetBackgroundDrawList();
	draw->AddRect(ImVec2(x, y), ImVec2(w, h), color, rounding, 0, thickness);
}

void Drawing::BoxFilled(float x, float y, float w, float h, ImU32 color, float rounding) {
	auto draw = ImGui::GetBackgroundDrawList();
	draw->AddRectFilled(ImVec2(x, y), ImVec2(w, h), color, rounding);
}

void Drawing::OutlineFilledBox(float x, float y, float w, float h, ImU32 color, ImU32 outlineColor, float outlineThickness, float rounding) {
	auto draw = ImGui::GetBackgroundDrawList();
	draw->AddRectFilled(ImVec2(x, y), ImVec2(w, h), color, rounding);
	draw->AddRect(ImVec2(x, y), ImVec2(w, h), color, rounding, 0, outlineThickness);
}

void Drawing::Text(char* text, float x, float y, ImU32 color, float& text_offset) {
	auto draw = ImGui::GetBackgroundDrawList();
	ImFont* font = ImGui::GetFont();

	ImVec2 size = ImGui::CalcTextSize(text);
	draw->AddText(font, 0, ImVec2(x, y - text_offset), color, text);
	text_offset += size.y + 1;
}

ImU32 Drawing::ToColor(ImVec4* color) {
	if (!color)
		return IM_COL32(255, 255, 255, 255);

	return IM_COL32(
		static_cast<int>(color->x * 255),
		static_cast<int>(color->y * 255),
		static_cast<int>(color->z * 255),
		static_cast<int>(color->w * 255)
	);
}