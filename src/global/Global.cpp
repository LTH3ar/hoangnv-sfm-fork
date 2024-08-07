#include "Global.h"
#include <fstream>
#include <stdexcept>

using namespace GlobalConfig;
namespace GlobalConfig {
    nlohmann::json configData;

    void loadConfig(const std::string& filePath) {
        std::ifstream configFile(filePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Could not open config file: " + filePath);
        }
        configFile >> configData;
        configFile.close();
    }

    int getRunMode() {
        return configData["runMode"]["value"];
    }

    int getGraphicsMode() {
        return configData["graphicsMode"]["value"];
    }

    int getTimeRatio() {
        return configData["timeRatio"]["value"];
    }

    int getJsonOutput() {
        return configData["jsonOutput"]["value"];
    }

    nlohmann::json getConfig() {
        return configData;
    }
}
