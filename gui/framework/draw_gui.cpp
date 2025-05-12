#define IMGUI_DEFINE_MATH_OPERATORS
#include "draw_gui.h"

#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_internal.h"
#include "ext/imgui/backends/imgui_impl_dx9.h"
#include "ext/imgui/backends/imgui_impl_win32.h"
#include <d3d9.h>

#include "globals/settings.h"
#include "helpers/custom.h"

#define ALPHA    ( ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )
#define NO_ALPHA ( ImGuiColorEditFlags_NoTooltip    | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )

using namespace ImGui;

void framework::gui::draw() {
    static bool bools[50]{};
    static int ints[50]{};
    vector<const char *> items = {"Head", "Chest", "Body", "Legs", "Feet"};
    static float color[4] = {1.f, 1.f, 1.f, 1.f};

    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(510, 380));

    ImGui::Begin("Hello, world!", NULL,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse); {
        auto window = GetCurrentWindow();
        auto draw = window->DrawList;
        auto pos = window->Pos;
        auto size = window->Size;
        auto style = GetStyle();

        custom.m_anim = ImLerp(custom.m_anim, 1.f, 0.03f);

        draw->AddLine(pos + ImVec2(65, 40), pos + ImVec2(size.x - 15, 40), ImColor(1.f, 1.f, 1.f, 0.05f));

        switch (custom.m_tab) {
            case 0:

                SetCursorPos(ImVec2(70, 13));
                BeginGroup();

                for (int i = 0; i < custom.visuals_subtabs.size(); ++i) {
                    if (custom.sub_tab(custom.visuals_subtabs.at(i), custom.m_visuals_subtab == i) && custom.
                        m_visuals_subtab != i)
                        custom.m_visuals_subtab = i, custom.m_anim = 0.f;

                    if (i != custom.visuals_subtabs.size() - 1)
                        SameLine();
                }

                EndGroup();

                SetCursorPos(ImVec2(65, 50));
                BeginChild("##rage_childs", ImVec2(GetWindowWidth() - 80, GetWindowHeight() - 80)); {
                    switch (custom.m_visuals_subtab) {
                        case 0:

                            custom.begin_child("General", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                                 GetWindowHeight())); {
                                Checkbox("Enable", &bools[0]);
                                SameLine(GetWindowWidth() - 33);
                                ColorEdit4("##color", color, ALPHA);
                                SliderInt("Field of View", &ints[0], 0, 100);
                                Combo("Hitbox", &ints[1], items.data(), items.size());
                                Button("Button", ImVec2(GetWindowWidth() - 20, 20));
                            }
                            custom.end_child();

                            SameLine();

                            custom.begin_child("Other", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                               GetWindowHeight())); {
                                for (int i = 1; i < 50; ++i)
                                    Checkbox(std::to_string(i).c_str(), &bools[i]);
                            }
                            custom.end_child();

                            break;
                    }
                }
                EndChild();

                break;

            case 1:

                SetCursorPos(ImVec2(70, 13));
                BeginGroup();

                for (int i = 0; i < custom.visuals_subtabs.size(); ++i) {
                    if (custom.sub_tab(custom.visuals_subtabs.at(i), custom.m_visuals_subtab == i) && custom.
                        m_visuals_subtab != i)
                        custom.m_visuals_subtab = i, custom.m_anim = 0.f;

                    if (i != custom.visuals_subtabs.size() - 1)
                        SameLine();
                }

                EndGroup();

                SetCursorPos(ImVec2(65, 50));
                BeginChild("##visuals_childs", ImVec2(GetWindowWidth() - 80, GetWindowHeight() - 80)); {
                    switch (custom.m_visuals_subtab) {
                        case 0:

                            custom.begin_child("ESP", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                             GetWindowHeight())); {
                            }
                            custom.end_child();

                            SameLine();

                            custom.begin_child("Colored models",
                                               ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2,
                                                      GetWindowHeight())); {
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
