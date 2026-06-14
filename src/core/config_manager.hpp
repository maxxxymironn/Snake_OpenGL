#pragma once
#include <filesystem>

class ConfigManager {
    std::filesystem::path file;

    ConfigManager();
    ~ConfigManager();

    bool readFile();
    std::filesystem::path getFilePath();
public:
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    bool saveFile();
};