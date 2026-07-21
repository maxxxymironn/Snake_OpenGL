#include "config_manager.hpp"

#include "../core/logger.hpp"
#include "core_config.hpp"
#include "game_config.hpp"
#include "window_config.hpp"
#include "renderer_config.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <string_view>

// headers for correctly search path on windows
#ifdef _WIN32
#include <combaseapi.h>
#include <shlobj.h>
#include <winerror.h>
#include <windows.h>
#endif

void printError(const std::string& description) {
    Logger::getInstance().printError("CONFIG_MANAGER", description.c_str());
}

bool isSuccessRead(int& iValue, const std::string& sValue, const std::string& configVarName) {
    try {
        iValue = std::stoi(sValue);
    } catch (const std::exception& e) {
        printError("\"" + configVarName + "\" has invalid value");
        return false;
    }
    return true;
}

ConfigManager::ConfigManager() : file(getFilePath()), readyToSaveFile(true) {
    if (!readFile())
        printError("file not readed. Settings set default");
}

ConfigManager::~ConfigManager() {
    if (!saveFile())
        printError("file not saved");
}

#ifdef _WIN32
std::filesystem::path ConfigManager::getFilePath() {
    PWSTR pszPath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pszPath);

    std::filesystem::path filePath;
    if (SUCCEEDED(hr)) {
        filePath = std::filesystem::path(pszPath) / "Snake_OpenGL"; 

        CoTaskMemFree(pszPath);

        if (!std::filesystem::exists(filePath))
            std::filesystem::create_directory(filePath);
    } else {
        filePath = std::filesystem::current_path();
    }

    return filePath / "config.txt";
}
#elif __linux__
std::filesystem::path ConfigManager::getFilePath() {
    namespace fs = std::filesystem;

    fs::path dirPath = fs::current_path();

    const char* dirPathStr = std::getenv("XDG_DATA_HOME");
    if (dirPathStr && dirPathStr[0] != '\0')
        dirPath = fs::path(dirPathStr) / "Snake_OpenGL";
    else {
        dirPathStr = std::getenv("HOME");
        if (dirPathStr && dirPathStr[0] != '\0')
            dirPath = fs::path(dirPathStr) / ".local" / "share" / "Snake_OpenGL";
    }

    if (!fs::exists(dirPath))
        fs::create_directories(dirPath);

    return dirPath / "config";
}
#endif

bool ConfigManager::readFile() {
    std::ifstream in(file);
    if (!in.is_open())
        return saveFile();

    std::string configVar;
    std::string equalSign;
    std::string value;

    std::string line;
    while (in) {
        getline(in, line);

        if (line.find_first_not_of(" \t\n\r") == std::string::npos || line.front() == '#')
            continue;

        std::stringstream ss(line);
        ss >> configVar;
        
        if (!(ss >> equalSign) || equalSign != "=") {
            printError("missing \"=\" after \"" + configVar + "\" variable");
            continue;
        } 
        else if (!(ss >> value)) {
            printError("missing \"" + configVar + "\" value");
            continue;
        }

        if (configVar == "fullscreen") {
            if (value == "true")
                WindowConfig::fullscreen = true;
            else if (value == "false")
                WindowConfig::fullscreen = false;
            else
                printError("\"fullscreen\" value should be \"true\"/\"false\"");
        }
        else if (configVar == "windowSize") {
            int iX, iY;

            if (!isSuccessRead(iX, value, configVar)) {
                continue;
            }
            if (!(ss >> value)) {
                printError("missing second \"" + configVar + "\" value");
            }
            else if (!isSuccessRead(iY, value, configVar)) {
                continue;
            }
            else if (iX < 800 || iY < 800) {
                printError("\"" + configVar + "\" is too small (size must be > 800 800)");
            }
            else {
                WindowConfig::windowWidth = iX;
                WindowConfig::windowHeight = iY;
            }
        }
        else if (configVar == "maxScore") {
            int maxScore;

            if (!isSuccessRead(maxScore, value, configVar))
                continue;
            else if (maxScore < 0 || maxScore > 5000)
                printError(configVar + " value " + (maxScore < 0 ? "cannot be negative" : "is too big"));
            else
                GameConfig::maxScore = maxScore;
        }
        else if (configVar == "wins") {
            int wins;

            if (!isSuccessRead(wins, value, configVar))
                continue;
            else if (wins < 0)
                printError("\"" + configVar + "\" value cannot be negative");
            else
                GameConfig::wins = wins;
        }
        else if (configVar == "tinyWins") {

        } 
        else if (configVar == "smallWins") {

        } 
        else if (configVar == "mediumWins") {

        } 
        else if (configVar == "bigWins") {

        } 
        else if (configVar == "hugeWins") {

        } 
        else if (configVar == "deaths") {
            int deaths;
            
            if (!isSuccessRead(deaths, value, configVar))
                continue;
            else if (deaths < 0)
                printError("\"" + configVar + "\" value cannot be negative");
            else
                GameConfig::deaths = deaths;
        } 
        else if (configVar == "timeAtGame") {

        } 
        else if (configVar == "gameMode") {
            if (value == "DEFAULT")
                GameConfig::gameMode = GameMode::DEFAULT;
            else if (value == "THROUGH_WALLS")
                GameConfig::gameMode = GameMode::THROUGH_WALLS;
            else
                printError("\"" + configVar + "\" has invalid value");
        } 
        else if (configVar == "fieldSize") {
            if (value == "TINY") {
                GameConfig::xFieldSize = 7;
                GameConfig::yFieldSize = 7;
            }
            else if (value == "SMALL") {
                GameConfig::xFieldSize = 10;
                GameConfig::yFieldSize = 10;
            }
            else if (value == "MEDIUM") {
                GameConfig::xFieldSize = 14;
                GameConfig::yFieldSize = 14;
            }
            else if (value == "BIG") {
                GameConfig::xFieldSize = 20;
                GameConfig::yFieldSize = 20;
            }
            else if (value == "HUGE") {
                GameConfig::xFieldSize = 28;
                GameConfig::yFieldSize = 28;
            }
            else {
                int iX, iY;

                if (!isSuccessRead(iX, value, configVar)) {
                    continue;
                }
                else if (!(ss >> value)) {
                    printError("missing second \"" + configVar + "\" value");
                } 
                else if (!isSuccessRead(iY, value, configVar)) {
                    continue;
                }
                else if (iX < 7 || iY < 7) {
                    printError("\"" + configVar + "\" is too small (size must be > 7)");
                } 
                else if (iX > 40 || iY > 40) {
                    printError("\"" + configVar + "\" is too big (size must be < 40)");
                } 
                else {
                    GameConfig::xFieldSize = iX;
                    GameConfig::yFieldSize = iY;
                }  
            }
        } 
        else if (configVar == "gameSpeed") {
            if (value == "VERY_SLOW")
                CoreConfig::gameSpeed = 0.6f;
            else if (value == "SLOW")
                CoreConfig::gameSpeed = 0.3f;
            else if (value == "MEDIUM")
                CoreConfig::gameSpeed = 0.15f;
            else if (value == "FAST")
                CoreConfig::gameSpeed = 0.1f;
            else
                printError("\"" + configVar + "\" has invalid value");
        } 
        else if (configVar == "renderMode") {
            if (value == "TEXTURE")
                RenderConfig::isTextureMode = true;
            else if (value == "PRIMITIVE")
                RenderConfig::isTextureMode = false;
            else
                printError("\"" + configVar + "\" has invalid value");
        }
        else {
            printError("\"" + configVar + "\" is unknown configuration variable");
        }
    }

    return true;
}

bool ConfigManager::saveFile() {
    // if (!readyToSaveFile)
    //     return false;
    std::ofstream out(file);
    if (!out.is_open())
        return false;

    std::string strFullscreen = WindowConfig::fullscreen ? "true" : "false";

    std::string strGameMode;
    switch (GameConfig::gameMode) {
        case GameMode::DEFAULT: strGameMode = "DEFAULT"; break;
        case GameMode::BOUNDLESS: strGameMode = "BOUNDLESS"; break;
        case GameMode::THROUGH_WALLS: strGameMode = "THROUGH_WALLS"; break;
    }

    std::string strFieldSize;
    if (GameConfig::xFieldSize == GameConfig::yFieldSize) {
        switch (GameConfig::xFieldSize) {
            case 7:  strFieldSize = "TINY"; break;
            case 10: strFieldSize = "SMALL"; break;
            case 14: strFieldSize = "MEDIUM"; break;
            case 20: strFieldSize = "BIG"; break;
            case 28: strFieldSize = "HUGE"; break;
        }
    } else
        strFieldSize = std::to_string(GameConfig::xFieldSize) + " " + std::to_string(GameConfig::yFieldSize);

    std::string strGameSpeed;
    if (CoreConfig::gameSpeed == 0.1f) {
        strGameSpeed = "FAST";
    } else if (CoreConfig::gameSpeed == 0.3f) {
        strGameSpeed = "SLOW";
    } else if (CoreConfig::gameSpeed == 0.6f) {
        strGameSpeed = "VERY_SLOW";
    } else {
        strGameSpeed = "MEDIUM";
    } 

    std::string strRenderMode;
    strRenderMode = RenderConfig::isTextureMode ? "TEXTURE" : "PRIMITIVE";

    out << 
    "# Window information\n"
    "fullscreen = " << strFullscreen << "\n"
    "windowSize = " << WindowConfig::windowWidth << " " 
                    << WindowConfig::windowHeight << "\n"

    "\n# Game information\n"
    "gameMode = " << strGameMode << "\n"
    "fieldSize = " << strFieldSize <<  "\n"

    "\n# Core information\n"
    "gameSpeed = " << strGameSpeed << "\n"

    "\n# Renderer information\n"
    "renderMode = " << strRenderMode << "\n"
    
    "\n# Statistics\n"
    "maxScore = " << GameConfig::maxScore << "\n"
    "wins = " << GameConfig::wins << "\n"
    "tinyWins = " << GameConfig::tinyWins << "\n"
    "mediumWins = " << GameConfig::mediumWins << "\n"
    "bigWins = " << GameConfig::bigWins << "\n"
    "hugeWins = " << GameConfig::hugeWins << "\n"
    "deaths = " << GameConfig::deaths << "\n"
    "timeAtGame = " << GameConfig::timeAtGame << "\n";

    readyToSaveFile = false;
    return true;
}
