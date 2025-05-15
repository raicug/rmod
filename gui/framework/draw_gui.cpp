#define IMGUI_DEFINE_MATH_OPERATORS
#include "draw_gui.h"

#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_internal.h"
#include "ext/imgui/backends/imgui_impl_dx9.h"
#include "ext/imgui/backends/imgui_impl_win32.h"
#include <d3d9.h>
#include <ext/imgui/custom/TextEditor.h>

#include "cheats/lua/lua.h"
#include "ext/imgui/custom/imgui_notify.h"
#include "globals/configManager.h"
#include "globals/settings.h"
#include "helpers/custom.h"
#include "sdk/utils/utilities.h"

#define ALPHA    ( ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )
#define NO_ALPHA ( ImGuiColorEditFlags_NoTooltip    | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )

std::vector<std::string> configs;
static char selectedConfig[256] = "";
static char newConfigName[256] = "";
static int selected_player = -1;
static char search_buffer[128] = "";
static bool show_friends_only = false;
static bool show_whitelisted_only = false;
static bool show_targets_only = false;

static int config_selected = 0;

using namespace ImGui;

void framework::gui::draw(TextEditor &editor, bool &configLoading) {
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(510, 380));

    ImGui::Begin("RMOD", NULL,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse); {
        auto window = GetCurrentWindow();
        auto draw = window->DrawList;
        auto pos = window->Pos;
        auto size = window->Size;
        auto style = GetStyle();

        custom.m_anim = ImLerp(custom.m_anim, 1.f, 0.03f);

        draw->AddLine(pos + ImVec2(65, 40), pos + ImVec2(size.x - 15, 40), ImColor(1.f, 1.f, 1.f, 0.05f));

        switch (custom.m_tab) {
            case 0: // Visuals
                custom.topbar_setup(custom.visuals_subtabs, custom.m_visuals_subtab);

                SetCursorPos(ImVec2(65, 50));
                BeginChild("##visuals_childs", ImVec2(GetWindowWidth() - 80, GetWindowHeight() - 80)); {
                    switch (custom.m_visuals_subtab) {
                        case 0: // FOV
                            custom.begin_child("General##1", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                                    GetWindowHeight())); {
                                Checkbox("Enable", &raicu::globals::settings::other::enableFov);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##color",
                                           reinterpret_cast<float *>(&raicu::globals::settings::other::fovColor),
                                           ALPHA);
                                SliderFloat("Field of View", &raicu::globals::settings::other::fovSize, 0, 160);
                            }
                            custom.end_child();

                            break;
                        case 1: // Crosshair
                            custom.begin_child("General##2", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                                    GetWindowHeight())); {
                                Checkbox("Enable", &raicu::globals::settings::crosshairValues::enabled);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##color_main",
                                           reinterpret_cast<float *>(&raicu::globals::settings::crosshairValues::color),
                                           ALPHA);

                                Checkbox("Outlined", &raicu::globals::settings::crosshairValues::outlineEnabled);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##color_outline",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::crosshairValues::outlineColor), ALPHA);
                            }
                            custom.end_child();

                            SameLine();

                            custom.begin_child("Values", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                                GetWindowHeight())); {
                                SliderFloat("Outline Thickness",
                                            &raicu::globals::settings::crosshairValues::outlineThickness, 0, 10,
                                            "%.2f");
                                SliderFloat("Rounding", &raicu::globals::settings::crosshairValues::rounding, 0, 10,
                                            "%.2f");
                                SliderFloat("Width", &raicu::globals::settings::crosshairValues::width, 0.1, 100,
                                            "%.2f");
                                SliderFloat("Height", &raicu::globals::settings::crosshairValues::height, 0.1, 100,
                                            "%.2f");
                                SliderFloat("Offset", &raicu::globals::settings::crosshairValues::offset, -100, 100,
                                            "%.2f");
                            }
                            custom.end_child();

                            break;
                        case 2: // ESP
                            custom.begin_child("Players##3", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                                    GetWindowHeight())); {
                                Checkbox("Enable", &raicu::globals::settings::espValues::enabled);
                                Checkbox("Health", &raicu::globals::settings::espValues::health);
                                Checkbox("Distance", &raicu::globals::settings::espValues::distance);
                                Checkbox("Box", &raicu::globals::settings::espValues::box);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##Boxcolor",
                                           reinterpret_cast<float *>(&raicu::globals::settings::espValues::boxColor),
                                           ALPHA);
                                Checkbox("Origin", &raicu::globals::settings::espValues::origin);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##Origincolor",
                                           reinterpret_cast<float *>(&raicu::globals::settings::espValues::originColor),
                                           ALPHA);
                                Checkbox("Name", &raicu::globals::settings::espValues::name);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##Namecolor",
                                           reinterpret_cast<float *>(&raicu::globals::settings::espValues::nameColor),
                                           ALPHA);
                                Checkbox("Snapline", &raicu::globals::settings::espValues::snapline);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##Snaplinecolor",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::espValues::snapLineColor), ALPHA);
                                Combo("Snapline Position", &raicu::globals::settings::espValues::snaplinePosition,
                                      custom.snaplinePosition, IM_ARRAYSIZE(custom.snaplinePosition));
                                SliderInt("Render Distance", &raicu::globals::settings::espValues::render_distance,
                                          1000, 30000);
                            }
                            custom.end_child();
                            SameLine();
                            custom.begin_child("Entities", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                                  GetWindowHeight())); {
                                Checkbox("Enabled", &raicu::globals::settings::espValues::entities::enabled);
                                Checkbox("Dormant", &raicu::globals::settings::espValues::entities::dormant);
                                Checkbox("Name", &raicu::globals::settings::espValues::entities::name);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##nameColor##2",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::espValues::entities::nameColor), ALPHA);
                                Checkbox("Box", &raicu::globals::settings::espValues::entities::box);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##boxColor##2",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::espValues::entities::boxColor), ALPHA);
                                Checkbox("Distance", &raicu::globals::settings::espValues::entities::distance);
                                SliderInt("Render Distance",
                                          &raicu::globals::settings::espValues::entities::render_distance,
                                          1000, 30000);
                            }
                            custom.end_child();
                            break;

                        case 3: // CHAMS
                            custom.begin_child("Players##4",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
                                Checkbox("Enable", &raicu::globals::settings::chams::enabled);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##PlayerColor",
                                           reinterpret_cast<float *>(&raicu::globals::settings::chams::playerColor),
                                           ALPHA);
                                Checkbox("Draw original model", &raicu::globals::settings::chams::draw_original_model);
                                Checkbox("Ignore walls", &raicu::globals::settings::chams::ignore_walls);

                                Combo("Material##1", &raicu::globals::settings::chams::material_type,
                                      custom.materialList, IM_ARRAYSIZE(custom.materialList));
                            }
                            custom.end_child();

                            SameLine();

                            custom.begin_child("View Model##1",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
                                Checkbox("Enabled", &raicu::globals::settings::chams::localplr::hands);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##HandsColor",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::chams::localplr::hands_color), ALPHA);
                                Combo("Material##3", &raicu::globals::settings::chams::localplr::hands_material_type,
                                      custom.materialList, IM_ARRAYSIZE(custom.materialList));
                            }
                            custom.end_child();

                            custom.begin_child("Entities##1",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
                                TextDisabled("Look at world tab for \"Entities\" list");
                                Checkbox("Enable", &raicu::globals::settings::chams::entities::enabled);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##EntityColor",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::chams::entities::entity_color),
                                           ALPHA);
                                Checkbox("Draw original model",
                                         &raicu::globals::settings::chams::entities::draw_original_model);
                                Checkbox("Ignore walls", &raicu::globals::settings::chams::entities::ignore_walls);

                                Combo("Material##1", &raicu::globals::settings::chams::entities::material_type,
                                      custom.materialList, IM_ARRAYSIZE(custom.materialList));
                            }
                            custom.end_child();

                            break;
                    }
                }
                EndChild();

                break;

            case 1: // Appearance
                custom.topbar_setup(custom.appearance_subtabs, custom.m_apperance_subtab);

                SetCursorPos(ImVec2(65, 50));

                BeginChild("##appearance_childs", ImVec2(GetWindowWidth() - 80, GetWindowHeight() - 80)); {
                    switch (custom.m_apperance_subtab) {
                        case 0: // Main
                            custom.begin_child("General##1", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                                    GetWindowHeight())); {
                                Checkbox("Enable console", &raicu::globals::settings::consoleOpen);
                                Checkbox("Enable logging notifications",
                                         &raicu::globals::settings::loggerNotifications);
                            }
                            custom.end_child();
                            SameLine();
                            custom.begin_child("Console Colors##2", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing
                                                                           .x / 2, GetWindowHeight())); {
                                Text("INFO Color");
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##infocol",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::consoleLogColours::infoColor), ALPHA);
                                Text("WARNING Color");
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##warningcol",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::consoleLogColours::warningColor), ALPHA);
                                Text("ERROR Color");
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##errorcol",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::consoleLogColours::errorColor), ALPHA);
                                Text("FATAL Color");
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##fatalcol",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::consoleLogColours::fatalColor), ALPHA);
                                Text("SUCCESS Color");
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##successcol",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::consoleLogColours::successColor), ALPHA);
                                Text("UNKNOWN Color");
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##unknowncol",
                                           reinterpret_cast<float *>(&
                                               raicu::globals::settings::consoleLogColours::unknownColor), ALPHA);
                            }
                            custom.end_child();

                            break;
                    }
                }

                EndChild();

                break;

            case 2: // LUA
                custom.topbar_setup(custom.lua_subtabs, custom.m_lua_subtab);

                SetCursorPos(ImVec2(65, 50));
                BeginChild("##lua_childs", ImVec2(GetWindowWidth() - 80, GetWindowHeight() - 80)); {
                    switch (custom.m_lua_subtab) {
                        case 0: // Main
                            custom.begin_child("General##1", ImVec2(GetWindowWidth() - GetStyle().ItemSpacing.x / 2,
                                                                    GetWindowHeight())); {
                                float inputWidth = GetWindowWidth() - GetStyle().ItemSpacing.x - 15;
                                editor.Render("##Source", ImVec2(inputWidth, GetWindowHeight() - 30)); {
                                    raicu::globals::settings::lua::ScriptInput = editor.GetText();
                                }
                                bool executePressed = Button("Execute", ImVec2(inputWidth, 25));

                                if (executePressed) {
                                    std::lock_guard<std::mutex> lock(lua::executionData.mutex);
                                    lua::executionData.script = raicu::globals::settings::lua::ScriptInput;
                                    lua::executionData.waiting.store(true);
                                    InsertNotification({
                                        ImGuiToastType::Success, 3000, "Executed script successfully!"
                                    });
                                }
                            }
                            custom.end_child();

                            break;
                    }
                }

                EndChild();

                break;

            case 3: // Configs
                custom.topbar_setup(custom.config_subtabs, custom.m_config_subtab);
                configs = ConfigManager::GetAllConfigs();

                SetCursorPos(ImVec2(65, 50));
                BeginChild("##config_childs", ImVec2(GetWindowWidth() - 80, GetWindowHeight() - 80)); {
                    switch (custom.m_config_subtab) {
                        case 0: // Main
                            custom.begin_child("Loading##1",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
                                if (!configs.empty()) {
                                    for (int i = 0; i < configs.size(); ++i) {
                                        if (strcmp(selectedConfig, configs[i].c_str()) == 0) {
                                            config_selected = i;
                                            break;
                                        }
                                    }
                                    if (ImGui::Combo("Select Config", &config_selected,
                                                     [](void *data, int idx, const char **out_text) {
                                                         const auto *vec = static_cast<std::vector<std::string> *>(
                                                             data);
                                                         if (idx < 0 || idx >= static_cast<int>(vec->size()))
                                                             return false;
                                                         *out_text = (*vec)[idx].c_str();
                                                         return true;
                                                     }, static_cast<void *>(&configs),
                                                     static_cast<int>(configs.size()))) {
                                        strcpy_s(selectedConfig, configs[config_selected].c_str());
                                    }

                                    if (ImGui::Button("Load Config")) {
                                        if (selectedConfig[0] != '\0') {
                                            configLoading = true;
                                            ConfigManager::Load(selectedConfig);
                                        }
                                    }
                                } else {
                                    TextDisabled("You have no configs!");
                                }
                            }

                            custom.end_child();
                            SameLine();
                            custom.begin_child("Saving##1",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
                                Text("Name");
                                InputText("##ConfigName", newConfigName, IM_ARRAYSIZE(newConfigName));

                                if (Button("Save Config")) {
                                    if (newConfigName[0] != '\0') {
                                        ConfigManager::Save(std::string(newConfigName) + ".json");
                                        configs = ConfigManager::GetAllConfigs();
                                        strcpy_s(selectedConfig, newConfigName);
                                    }
                                }
                            }

                            custom.end_child();
                            break;
                    }
                }
                EndChild();

                break;

            case 4: // World
                custom.topbar_setup(custom.world_subtabs, custom.m_world_subtab);
                SetCursorPos(ImVec2(65, 50));

                BeginChild("##world_childs", ImVec2(GetWindowWidth() - 80, GetWindowHeight() - 80)); {
                    switch (custom.m_world_subtab) {
                        case 0: // Aimbot
                            custom.begin_child("General##1",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
                                Checkbox("Enabled", &raicu::globals::settings::aimbot::enabled);
                                raicu::gui::other::hotkey("Aimbot", &raicu::globals::settings::aimbot::hotkey);
                                Checkbox("Silent", &raicu::globals::settings::aimbot::silent);
                                Checkbox("Line to target", &raicu::globals::settings::aimbot::visualise_target_line);
                                Checkbox("Trigger fire", &raicu::globals::settings::aimbot::automatic_fire);
                                Checkbox("Penetrate walls", &raicu::globals::settings::aimbot::penetrate_walls);

                                Combo("Hitbox", &raicu::globals::settings::aimbot::hitbox, custom.aimbot_hitboxes,
                                      IM_ARRAYSIZE(custom.aimbot_hitboxes));
                                Combo("Priority", &raicu::globals::settings::aimbot::priority, custom.aimbot_priorities,
                                      IM_ARRAYSIZE(custom.aimbot_priorities));
                            }
                            custom.end_child();
                            SameLine();

                            custom.begin_child("Accuracy + Backtrack",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
                                Checkbox("Predict spread", &raicu::globals::settings::aimbot::predict_spread);
                                Checkbox("Disable recoil", &raicu::globals::settings::aimbot::disable_recoil);
                                SliderFloat("Smooth", &raicu::globals::settings::aimbot::smooth, 0, 20, "%.1f");

                                Separator();
                                Checkbox("Enable", &raicu::globals::settings::aimbot::backtrackEnabled);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##backtrackColor",
                                           reinterpret_cast<float *>(&raicu::globals::settings::aimbot::backtrackColor),
                                           NO_ALPHA);
                                SliderFloat("Backtrack", &raicu::globals::settings::aimbot::backtrack, 0, 1, "%.2f ms");
                                Combo("Material", &raicu::globals::settings::aimbot::backtrackMaterial,
                                      custom.materialList, IM_ARRAYSIZE(custom.materialList));
                            }
                            custom.end_child();

                            break;
                        case 1: // Movement
                            custom.begin_child("General##2",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
                                Checkbox("Bunny Hop", &raicu::globals::settings::movement::bhop);
                                Checkbox("Air Strafe", &raicu::globals::settings::movement::air_strafe);
                            }
                            custom.end_child();
                            break;
                        case 2: // Players
                            custom.begin_child("Main##4", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                                 GetWindowHeight())); {
                                Checkbox("Show Friends Only", &show_friends_only);
                                Checkbox("Show Whitelisted Only", &show_whitelisted_only);
                                Checkbox("Show Targets Only", &show_targets_only);
                                Separator();

                                Checkbox("Spectator list", &raicu::globals::settings::other::spectatorList);
                                InputTextWithHint("Search", "Search for player", search_buffer,
                                                  IM_ARRAYSIZE(search_buffer));

                                Separator();
                                if (Button("Clear friends")) {
                                    raicu::globals::settings::friend_list = {};
                                }
                                if (Button("Clear whitelist")) {
                                    raicu::globals::settings::whitelist = {};
                                }
                                if (Button("Clear targets")) {
                                    raicu::globals::settings::target_list = {};
                                }
                            }
                            custom.end_child();
                            SameLine();
                            custom.begin_child("Players##4",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
                                player_info_t pinfo;
                                BeginChild("##PlayerList", ImVec2(0, GetWindowHeight() - 30)); {
                                    for (size_t i = 0; i <= interfaces::engine->get_max_clients(); ++i) {
                                        c_base_entity *entity = interfaces::entity_list->
                                                get_entity(static_cast<int>(i));
                                        if (!entity) {
                                            continue;
                                        }

                                        if (entity->is_player()) {
                                            if (entity == interfaces::entity_list->get_entity(
                                                    interfaces::engine->get_local_player()))
                                                continue;

                                            if (interfaces::engine->get_player_info(i, &pinfo)) {
                                                if (strlen(pinfo.name) > 0) {
                                                    std::string name_lower = pinfo.name;
                                                    std::string search_lower = search_buffer;
                                                    std::transform(name_lower.begin(), name_lower.end(),
                                                                   name_lower.begin(),
                                                                   ::tolower);
                                                    std::transform(search_lower.begin(), search_lower.end(),
                                                                   search_lower.begin(), ::tolower);
                                                    if (!search_lower.empty() && name_lower.find(search_lower) ==
                                                        std::string::npos) {
                                                        continue;
                                                    }

                                                    bool is_whitelisted = false;
                                                    if (raicu::globals::settings::whitelist.contains("players")) {
                                                        const auto &players = raicu::globals::settings::whitelist[
                                                            "players"];
                                                        is_whitelisted = std::any_of(players.begin(), players.end(),
                                                            [&pinfo](const nlohmann::json &player) {
                                                                return player["name"] == pinfo.name;
                                                            });
                                                    }

                                                    bool is_friend = false;
                                                    if (raicu::globals::settings::friend_list.contains("players")) {
                                                        const auto &players = raicu::globals::settings::friend_list[
                                                            "players"];
                                                        is_friend = std::any_of(players.begin(), players.end(),
                                                            [&pinfo](const nlohmann::json &player) {
                                                                return player["name"] == pinfo.name;
                                                            });
                                                    }
                                                    bool is_target = false;
                                                    if (raicu::globals::settings::target_list.contains("players")) {
                                                        const auto &players = raicu::globals::settings::target_list[
                                                            "players"];
                                                        is_target = std::any_of(players.begin(), players.end(),
                                                            [&pinfo](const nlohmann::json &player) {
                                                                return player["name"] == pinfo.name;
                                                            });
                                                    }

                                                    bool should_display = true;
                                                    if (show_friends_only && show_whitelisted_only) {
                                                        should_display =
                                                                is_friend || is_whitelisted || (
                                                                    is_friend && is_whitelisted);
                                                    } else if (show_friends_only) {
                                                        should_display = is_friend;
                                                    } else if (show_whitelisted_only) {
                                                        should_display = is_whitelisted;
                                                    } else if (show_targets_only) {
                                                        should_display = is_target;
                                                    }

                                                    if (!should_display) {
                                                        continue;
                                                    }

                                                    ImGui::PushID(static_cast<int>(i));

                                                    std::string display_name = pinfo.name;

                                                    if (is_target) {
                                                        display_name += " [TARGET]";
                                                    }
                                                    if (is_whitelisted) {
                                                        display_name += " [WHITELISTED]";
                                                    }
                                                    if (is_friend) {
                                                        display_name += " [FRIEND]";
                                                    }

                                                    if (ImGui::Selectable(display_name.c_str(), selected_player == i)) {
                                                        selected_player = i;
                                                        ImGui::OpenPopup(display_name.c_str());
                                                    }

                                                    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(80, 65));
                                                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
                                                    if (BeginPopupContextItem(display_name.c_str())) {
                                                        if (ImGui::Selectable("Whitelist player", is_whitelisted)) {
                                                            if (is_whitelisted) {
                                                                if (raicu::globals::settings::whitelist.contains(
                                                                    "players")) {
                                                                    auto &players = raicu::globals::settings::whitelist[
                                                                        "players"];
                                                                    players.erase(
                                                                        std::remove_if(players.begin(), players.end(),
                                                                            [&pinfo](const nlohmann::json &player) {
                                                                                return player["name"] == pinfo.name;
                                                                            }
                                                                        ),
                                                                        players.end()
                                                                    );
                                                                }
                                                            } else {
                                                                if (!raicu::globals::settings::whitelist.contains(
                                                                    "players")) {
                                                                    raicu::globals::settings::whitelist["players"] =
                                                                            nlohmann::json::array();
                                                                }

                                                                nlohmann::json player_entry;
                                                                player_entry["name"] = pinfo.name;
                                                                player_entry["index"] = i;

                                                                raicu::globals::settings::whitelist["players"].
                                                                        push_back(player_entry);
                                                            }
                                                        }

                                                        if (ImGui::Selectable("Mark player as friend", is_friend)) {
                                                            if (is_friend) {
                                                                if (raicu::globals::settings::friend_list.contains(
                                                                    "players")) {
                                                                    auto &friends =
                                                                            raicu::globals::settings::friend_list[
                                                                                "players"];
                                                                    friends.erase(
                                                                        std::remove_if(friends.begin(), friends.end(),
                                                                            [&pinfo](const nlohmann::json &player) {
                                                                                return player["name"] == pinfo.name;
                                                                            }
                                                                        ),
                                                                        friends.end()
                                                                    );
                                                                }
                                                            } else {
                                                                if (!raicu::globals::settings::friend_list.contains(
                                                                    "players")) {
                                                                    raicu::globals::settings::friend_list["players"] =
                                                                            nlohmann::json::array();
                                                                }

                                                                nlohmann::json player_entry;
                                                                player_entry["name"] = pinfo.name;
                                                                player_entry["index"] = i;

                                                                raicu::globals::settings::friend_list["players"].
                                                                        push_back(player_entry);
                                                            }
                                                        }

                                                        if (ImGui::Selectable("Mark player as target", is_target)) {
                                                            if (is_target) {
                                                                if (raicu::globals::settings::target_list.contains(
                                                                    "players")) {
                                                                    auto &friends =
                                                                            raicu::globals::settings::target_list[
                                                                                "players"];
                                                                    friends.erase(
                                                                        std::remove_if(friends.begin(), friends.end(),
                                                                            [&pinfo](const nlohmann::json &player) {
                                                                                return player["name"] == pinfo.name;
                                                                            }
                                                                        ),
                                                                        friends.end()
                                                                    );
                                                                }
                                                            } else {
                                                                if (!raicu::globals::settings::target_list.contains(
                                                                    "players")) {
                                                                    raicu::globals::settings::target_list["players"] =
                                                                            nlohmann::json::array();
                                                                }

                                                                nlohmann::json player_entry;
                                                                player_entry["name"] = pinfo.name;
                                                                player_entry["index"] = i;

                                                                raicu::globals::settings::target_list["players"].
                                                                        push_back(player_entry);
                                                            }
                                                        }
                                                        EndPopup();
                                                    }
                                                    PopStyleVar(2);

                                                    if (ImGui::IsItemHovered()) {
                                                        ImGui::BeginTooltip();
                                                        ImGui::Text("Player Index: %d", i);
                                                        ImGui::EndTooltip();
                                                    }

                                                    ImGui::PopID();
                                                }
                                            }
                                        }
                                    }
                                }
                                EndChild();
                            }
                            custom.end_child();

                            break;
                        case 3: // Entities
                            custom.begin_child("Main##5", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                                 GetWindowHeight())); {
                                if (Button("Clear list")) {
                                    raicu::globals::settings::chams::entities::list = {};
                                }
                            }
                            custom.end_child();
                            SameLine();
                            custom.begin_child("Entities##4",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
                                utilities::update_entity_list(raicu::globals::settings::chams::entities::list);

                                static int id_counter = 0;
                                for (auto item: raicu::globals::settings::chams::entities::list.items()) {
                                    ImGui::PushID((item.key() + std::to_string(id_counter++)).c_str());

                                    if (ImGui::Selectable((item.key() + std::to_string(id_counter++)).c_str(),
                                                          item.value())) {
                                        ImGui::OpenPopup((item.key() + std::to_string(id_counter++)).c_str());
                                    }

                                    if (ImGui::BeginPopupContextItem(
                                        (item.key() + std::to_string(id_counter++)).c_str())) {
                                        if (ImGui::Selectable("Apply chams", item.value())) {
                                            item.value() = !item.value();
                                        }
                                        ImGui::EndPopup();
                                    }

                                    ImGui::PopID();
                                }
                                id_counter = 0;
                            }
                            custom.end_child();
                            break;
                    	case 4: //Misc
                    		custom.begin_child("Camera##1", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2, GetWindowHeight()));
	                        {
		                        Checkbox("Third Person", &raicu::globals::settings::other::third_person);
                        		raicu::gui::other::hotkey("3rd Person", &raicu::globals::settings::other::third_person_hotkey);
                        		SliderInt("Distance", &raicu::globals::settings::other::third_person_distance, 0, 200, "%.1f");

                        		Checkbox("Freecam##1", &raicu::globals::settings::other::freecam);
                        		raicu::gui::other::hotkey("Freecam", &raicu::globals::settings::other::freecam_hotkey);
                        		SliderFloat("Speed", &raicu::globals::settings::other::freecam_speed, 0, 20, "%.1f");

                        		Checkbox("FOV", &raicu::globals::settings::other::custom_fov);
                        		SliderFloat("Value", &raicu::globals::settings::other::custom_fov_value, 50, 179, "%.1f");

                        		Checkbox("Model FOV", &raicu::globals::settings::other::custom_view_model_fov);
                        		SliderFloat("Value##2", &raicu::globals::settings::other::custom_view_model_fov_value, 30, 179, "%.1f");

	                        }
                    		custom.end_child();
                            break;
                    }
                }
                EndChild();

                break;
            case 5: // Game
                custom.topbar_setup(custom.game_subtabs, custom.m_game_subtab);
                SetCursorPos(ImVec2(65, 50));
                BeginChild("##game_childs", ImVec2(GetWindowWidth() - 80, GetWindowHeight() - 80));
                {
                    switch (custom.m_game_subtab) {
                        case 0: // Main
                            custom.begin_child("Main##1", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                                 GetWindowHeight()));
                            {
                            /*Checkbox("Enabled", &raicu::globals::settings::loading_screen::enabled);
                            InputTextWithHint("Loading Screen", "asset://garrysmod/html/menu.html", reinterpret_cast<char*>(&raicu::globals::settings::loading_screen::url), IM_ARRAYSIZE(raicu::globals::settings::loading_screen::url));*/
                            Checkbox("OBS Bypass", &raicu::globals::settings::other::obs_bypass);
                            }
                            custom.end_child();
                            SameLine();
                            custom.begin_child("LUA Dumper##1", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2, GetWindowHeight()));
                            {
                                Checkbox("Dumper", &raicu::globals::settings::other::dumper);
                            }

                            custom.end_child();
                            break;
                    }
                }
                EndChild();
                break;
        }

        SetCursorPosY(0);
        custom.tab_area("##tab_area", ImVec2(50, size.y - 20), [ ]() {
            for (int i = 0; i < custom.tabs.size(); ++i)
                if (custom.tab(custom.tabs_icons.at(i), custom.tabs.at(i), custom.m_tab == i) && custom.m_tab != i)
                    custom.m_tab = i, custom.m_anim = 0.f;
        });

        // footer
        draw->AddRectFilled(pos + ImVec2(0, size.y - 20), pos + size, ImColor(15, 14, 21), style.WindowRounding,
                            ImDrawFlags_RoundCornersBottom);
        draw->AddText(pos + ImVec2(5, size.y - 18), GetColorU32(ImGuiCol_Text), "Garry's Mod x64");
        draw->AddText(pos + ImVec2(size.x - CalcTextSize("protactium").x - 5, size.y - 18), GetColorU32(ImGuiCol_Text),
                      "protactium");
    }
    ImGui::End();

    PopStyleVar(2);
}
