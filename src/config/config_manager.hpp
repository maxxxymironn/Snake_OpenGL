#pragma once
#include <filesystem>

class ConfigManager {
    std::filesystem::path file;
    bool readyToSaveFile;

    bool readFile();
    std::filesystem::path getFilePath();
public:
    ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ~ConfigManager();

    bool isReadyToSaveFile() { return readyToSaveFile; }
    void setReadyToSaveFile() { readyToSaveFile = true; }

    bool saveFile();
};