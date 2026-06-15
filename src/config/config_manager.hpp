#pragma once
#include <filesystem>

class ConfigManager {
    std::filesystem::path file;

    bool readFile();
    std::filesystem::path getFilePath();
public:
    ConfigManager();
    ConfigManager(const ConfigManager&) = delete;

    ConfigManager& operator=(const ConfigManager&) = delete;

    ~ConfigManager();

    bool saveFile();
};