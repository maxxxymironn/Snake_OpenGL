#pragma once

#include <string>

class ConfigManager {
    std::string filePath = "snakeOpenGL.config";

    ConfigManager() { unload(); }

    bool unload();
public:
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    bool upload();
};