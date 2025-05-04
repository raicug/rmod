#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <nlohmann/json.h>
#include <fstream>
#include <string>
#include <vector>

namespace ConfigManager {
    void Save(const std::string& fileName);
    void Load(const std::string& fileName);
    std::vector<std::string> GetAllConfigs();
}

#endif //CONFIGMANAGER_H
