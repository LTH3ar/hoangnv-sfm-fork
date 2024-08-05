#pragma once

#include <string>
#include "lib/nlohmann/json.hpp"

using namespace std;

namespace GlobalConfig {
    void loadConfig(const std::string& filePath = "data/configuration.json");
    int getRunMode();
    int getGraphicsMode();
    int getTimeRatio();
    int getJsonOutput();
    nlohmann::json getConfig();

    extern nlohmann::json configData;
}
