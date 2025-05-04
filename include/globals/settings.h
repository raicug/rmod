#ifndef SETTINGS_H
#define SETTINGS_H

#include <nlohmann/json.h>
#include <ext/imgui/imgui.h>
#include <d3d9.h>
#include <sdk/Interface.h>

namespace raicu::globals::settings {
    inline std::string version = "1.0.0a";
    inline bool open = false;

    inline D3DMATRIX world_to_screen;

    inline c_vector view_origin;
    inline q_angle view_angles;
    inline float fov = 0;

    inline v_panel mat_system_top_panel = false;
    inline v_panel overlay_popup_panel = false;
    inline v_panel focus_overlay_panel = false;

    inline ImFont *defaultFont = nullptr;
    inline bool consoleOpen = false;
    inline bool loggerNotifications = false;

    namespace consoleLogColours {
        inline ImVec4 infoColor = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
        inline ImVec4 warningColor = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);
        inline ImVec4 errorColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        inline ImVec4 fatalColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        inline ImVec4 successColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
        inline ImVec4 unknownColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

        namespace defaults {
            inline ImVec4 infoColor = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
            inline ImVec4 warningColor = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);
            inline ImVec4 errorColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
            inline ImVec4 fatalColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            inline ImVec4 successColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
            inline ImVec4 unknownColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        }
    }

    namespace movement {
        inline bool bhop = false;
    }

    namespace aimbot {
        inline bool enabled = false;
        inline bool silent = false;
        inline bool automatic_fire = false;
        inline bool penetrate_walls = false;
        inline int hitbox = 0;
        inline int priority = 0;

        inline bool visualise_target_line = true;

        inline bool predict_spread = false;
        inline float smooth = 0.f;
        inline float backtrack = 0.f;
        inline nlohmann::json team_list;
        inline nlohmann::json friend_list;
    }

    namespace espValues {
        inline bool chams = false;
        inline bool enabled = false;
        inline bool showNPC = false;
        inline bool showTeam = false;
        inline bool snapline = false;
        inline bool skeleton = false;
        inline bool origin = false;
        inline bool name = false;
        inline bool box = false;
        inline bool health = false;

        inline int render_distance = 15000;

        inline bool ignoreWalls = false;
        inline bool drawOriginalModel = false;

        inline int ChamsMaterialType = 0;

        inline bool distance = false;
        inline bool weapon = false;

        inline int snaplinePosition = 0; // 0 = bottom, 1 = middle, 2 = top

        inline ImVec4 snapLineColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        inline ImVec4 originColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        inline ImVec4 skeletonColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        inline ImVec4 skeletonHeadColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        inline ImVec4 nameColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        inline ImVec4 chamsColour = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        inline ImVec4 boxColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    }

    namespace crosshairValues {
        inline bool enabled = false;
        inline bool outlineEnabled = false;

        inline float outlineThickness = 1;
        inline float rounding = 1;

        inline float width = 4;
        inline float height = 4;
        inline float offset = 4;

        inline ImVec4 color = ImVec4(1.f, 1.f, 0.f, 1.f);
        inline ImVec4 outlineColor = ImVec4(1.f, 1.f, 0.f, 1.f);
    }

    namespace other {
        inline bool enableFov = false;
        inline float fovSize = 20.0f;
        inline ImVec4 fovColor = ImVec4(1.0f, 1.f, 0.f, 1.f);
    }

    namespace lua {
        inline std::string ScriptInput = "print(\"Hello World!\")";
        inline int executeState = 0;
    }
}

#endif //SETTINGS_H
