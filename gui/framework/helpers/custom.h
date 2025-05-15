#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_internal.h"

#include <string>
#include <vector>
#include <functional>

#include "fonts/IconFontAwesome6.h"

#define to_vec4(r,g,b,a) ImColor(r / 255.f,g / 255.f,b / 255.f,a / 255.f)

class c_custom {
public:
    float m_anim = 0.f;

    int m_tab = 0;
    std::vector < const char* > tabs = { "Visuals", "Appearance", "Lua", "Config", "World", "Game" }, tabs_icons = {
        ICON_FA_EYE_LOW_VISION, ICON_FA_PALETTE, ICON_FA_CODE, ICON_FA_GEAR, ICON_FA_EARTH_AMERICAS, ICON_FA_GAMEPAD
    };

    int m_visuals_subtab = 0;
    std::vector < const char* > visuals_subtabs = { "FOV", "Crosshair", "ESP", "Chams" };

    int m_apperance_subtab = 0;
    std::vector<const char *> appearance_subtabs = {"Main"};

    int m_lua_subtab = 0;
    std::vector < const char* > lua_subtabs = { "Main" };

    int m_config_subtab = 0;
    std::vector<const char *> config_subtabs = {"Main"};

    int m_world_subtab = 0;
    std::vector < const char* > world_subtabs = { "Aimbot", "Movement", "Players", "Entities", "Misc" };

    int m_game_subtab = 0;
    std::vector<const char *> game_subtabs = {"Main"};

    const char *executorLuaState[3]{ "Client", "Server", "Menu"};

    const char *materialList[4]{ "Normal (no color)", "Metal", "Wireframe", "Flat" };

    const char *snaplinePosition[3]{ "Bottom", "Middle", "Top" };

    const char *aimbot_hitboxes[4]{ "Head", "Chest", "Stomach", "Hitscan" };

    const char *aimbot_priorities[3]{ "Fov", "Distance", "Health" };

    void topbar_setup(std::vector<const char *> a, int &b);

    float col_buf[4] = { 1.f, 1.f, 1.f, 1.f };

    void cleanup() {
        if (expand_button_values) {
            expand_button_values->clear();
            delete expand_button_values;
            expand_button_values = nullptr;
        }
        if (tab_values) {
            tab_values->clear();
            delete tab_values;
            tab_values = nullptr;
        }
        if (area_values) {
            area_values->clear();
            delete area_values;
            area_values = nullptr;
        }
    }

    static void render_arrows_for_horizontal_bar(ImVec2 pos, float alpha, float width, float height);
    static bool expand_button(const char* label, bool selected, float rounding = 0.f, ImDrawFlags flags = NULL);
    static bool tab(const char* icon, const char* label, bool selected);
    bool sub_tab(const char* label, bool selected);
    static void tab_area(const char* str_id, ImVec2 size, std::function<void()> content);
    void begin_child(const char* name, ImVec2 size);
    void end_child();

    int accent_color[4] = { 140, 131, 214, 255 };
    ImColor get_accent_color(float a = 1.f) const {
        return ImColor(
            accent_color[0] / 255.0f,
            accent_color[1] / 255.0f,
            accent_color[2] / 255.0f,
            a
        );
    }
private:
    // Store static maps as pointers so we can properly control their lifetime
    inline static std::unordered_map<ImGuiID, float>* expand_button_values;
    inline static std::unordered_map<ImGuiID, float>* tab_values;
    inline static std::unordered_map<ImGuiID, float>* area_values;

};

inline c_custom custom;