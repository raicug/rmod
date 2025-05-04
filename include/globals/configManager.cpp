#include "configManager.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.h>
#include <ext/imgui/imgui.h>
#include <string>

#include "settings.h"
#include "logging/logger.h"

using json = nlohmann::json;
using namespace raicu::globals::settings;

namespace {
    const std::string CONFIG_DIR = "C:\\R-GMOD\\configs\\";

    json ToJson(const ImVec4& color) {
        return json{ color.x, color.y, color.z, color.w };
    }

    void FromJson(const json& j, ImVec4& color) {
        if (j.is_array() && j.size() >= 4) {
            color.x = j[0].get<float>();
            color.y = j[1].get<float>();
            color.z = j[2].get<float>();
            color.w = j[3].get<float>();
        }
    }
}

void ConfigManager::Save(const std::string& fileName) {
    using namespace raicu::globals::settings;

    json j;

    j["consoleOpen"] = consoleOpen;
    j["loggerNotifications"] = loggerNotifications;

    j["consolelogColors"] = {
        {"infoColor", ToJson(consoleLogColours::infoColor)},
        {"warningColor", ToJson(consoleLogColours::warningColor)},
        {"errorColor", ToJson(consoleLogColours::errorColor)},
        {"fatalColor", ToJson(consoleLogColours::fatalColor)},
        {"successColor", ToJson(consoleLogColours::successColor)},
        {"unknownColor", ToJson(consoleLogColours::unknownColor)}
    };

    j["espValues"] = {
        {"enabled", espValues::enabled},
        {"showNPC", espValues::showNPC},
        {"showTeam", espValues::showTeam},
        {"snapline", espValues::snapline},
        {"skeleton", espValues::skeleton},
        {"origin", espValues::origin},
        {"name", espValues::name},
        {"snapLineColor", ToJson(espValues::snapLineColor)},
        {"originColor", ToJson(espValues::originColor)},
        {"skeletonColor", ToJson(espValues::skeletonColor)},
        {"skeletonHeadColor", ToJson(espValues::skeletonHeadColor)}
    };

    j["crosshairValues"] = {
        {"enabled", crosshairValues::enabled},
        {"outlineEnabled", crosshairValues::outlineEnabled},
        {"outlineThickness", crosshairValues::outlineThickness},
        {"rounding", crosshairValues::rounding},
        {"width", crosshairValues::width},
        {"height", crosshairValues::height},
        {"offset", crosshairValues::offset},
        {"color", ToJson(crosshairValues::color)}
    };

    j["other"] = {
        {"enableFov", other::enableFov},
        {"fovSize", other::fovSize},
        {"fovColor", ToJson(other::fovColor)}
    };

    j["lua"] = {
        {"ScriptInput", lua::ScriptInput},
        {"executeState", lua::executeState}
    };

    std::filesystem::create_directories(CONFIG_DIR);

    std::ofstream file(CONFIG_DIR + fileName);
    if (file.is_open()) {
        file << j.dump(4);
        logger::Log(5, "Saved settings to JSON");
    }
    else {
        logger::Log(3, ("Failed to open file for saving: " + CONFIG_DIR + fileName).c_str());
    }
}


void ConfigManager::Load(const std::string& fileName) {
    using namespace raicu::globals::settings;

    logger::Log(1, ("Loading config [" + fileName + "]").c_str());

    std::ifstream file(CONFIG_DIR + fileName);
    if (!file.is_open()) {
        logger::Log(3, ("Failed to open file for loading: " + CONFIG_DIR + fileName).c_str());
        return;
    }

    json j;
    try {
        file >> j;
    }
    catch (const std::exception& e) {
        logger::Log(3, ("Error reading JSON: " + std::string(e.what())).c_str());
        return;
    }

    consoleOpen = j.value("consoleOpen", consoleOpen);
    loggerNotifications = j.value("loggerNotifications", loggerNotifications);

    if (j.contains("consolelogColors")) {
        auto& c = j["consolelogColors"];
        FromJson(c["infoColor"], consoleLogColours::infoColor);
        FromJson(c["warningColor"], consoleLogColours::warningColor);
        FromJson(c["errorColor"], consoleLogColours::errorColor);
        FromJson(c["fatalColor"], consoleLogColours::fatalColor);
        FromJson(c["successColor"], consoleLogColours::successColor);
        FromJson(c["unknownColor"], consoleLogColours::unknownColor);
    }

    if (j.contains("espValues")) {
        auto& e = j["espValues"];
        espValues::enabled = e.value("enabled", espValues::enabled);
        espValues::showNPC = e.value("showNPC", espValues::showNPC);
        espValues::showTeam = e.value("showTeam", espValues::showTeam);
        espValues::snapline = e.value("snapline", espValues::snapline);
        espValues::skeleton = e.value("skeleton", espValues::skeleton);
        espValues::origin = e.value("origin", espValues::origin);
        espValues::name = e.value("name", espValues::name);
        FromJson(e["snapLineColor"], espValues::snapLineColor);
        FromJson(e["originColor"], espValues::originColor);
        FromJson(e["skeletonColor"], espValues::skeletonColor);
        FromJson(e["skeletonHeadColor"], espValues::skeletonHeadColor);
    }

    if (j.contains("crosshairValues")) {
        auto& c = j["crosshairValues"];
        crosshairValues::enabled = c.value("enabled", crosshairValues::enabled);
        crosshairValues::outlineEnabled = c.value("outlineEnabled", crosshairValues::outlineEnabled);
        crosshairValues::outlineThickness = c.value("outlineThickness", crosshairValues::outlineThickness);
        crosshairValues::rounding = c.value("rounding", crosshairValues::rounding);
        crosshairValues::width = c.value("width", crosshairValues::width);
        crosshairValues::height = c.value("height", crosshairValues::height);
        crosshairValues::offset = c.value("offset", crosshairValues::offset);
        FromJson(c["color"], crosshairValues::color);
    }

    if (j.contains("other")) {
        auto& o = j["other"];
        other::enableFov = o.value("enableFov", other::enableFov);
        other::fovSize = o.value("fovSize", other::fovSize);
        FromJson(o["fovColor"], other::fovColor);
    }

    if (j.contains("lua")) {
        auto& l = j["lua"];
        lua::executeState = l.value("executeState", lua::executeState);
        lua::ScriptInput = l.value("ScriptInput", lua::ScriptInput);
    }
}

std::vector<std::string> ConfigManager::GetAllConfigs() {
    std::vector<std::string> configs;
    namespace fs = std::filesystem;

    if (!fs::exists(CONFIG_DIR) || !fs::is_directory(CONFIG_DIR))
        return configs;

    for (const auto& entry : fs::directory_iterator(CONFIG_DIR)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            configs.push_back(entry.path().filename().string());
        }
    }

    return configs;
}