#include "config_manager.hpp"

#include "core_config.hpp"
#include "game_config.hpp"
#include "window_config.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <combaseapi.h>
#include <shlobj.h>
#include <winerror.h>
#include <windows.h>
#endif

static void printError(const std::string& description) {
    const static std::string preamble = "ERROR::CONFIG_MANAGER: ";
    std::cerr << preamble << description << "\n";
}

static bool tryRead(int& iValue, const std::string& sValue, const std::string& configVarName) {
    try {
        iValue = std::stoi(sValue);
    } catch (const std::exception& e) {
        printError("\"" + configVarName + "\" has invalid value");
        return false;
    }
    return true;
}

ConfigManager::ConfigManager() : file(getFilePath()) {
    if (!readFile())
        printError("config file cannot read and be created. Setting are set to default");
}

ConfigManager::~ConfigManager() {
    if (!saveFile())
        printError("\n");
}

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
        } else if (!(ss >> value)) {
            printError("missing \"" + configVar + "\" value");
            continue;
        }

        if (configVar == "fullscreen") {
            if (value == "true") {
                WindowConfigVariables::fullscreen = true;
            } else if (value == "false") {
                WindowConfigVariables::fullscreen = false;
            } else {
                printError("\"fullscreen\" value should be \"true\"/\"false\"");
            }
        }
        else if (configVar == "windowSize") {
            int iX, iY;

            if (tryRead(iX, value, configVar)) {
                if (!(ss >> value)) {
                    printError("missing second \"" + configVar + "\" value");
                    continue;
                } else if (tryRead(iY, value, configVar)) {
                    if (iX < 800 || iY < 800) {
                        printError("\"" + configVar + "\" value is too small");
                    } else {
                        WindowConfigVariables::windowWidth = iX;
                        WindowConfigVariables::windowHeight = iY;
                    }  
                }
            }
        }
        else if (configVar == "maxScore") {
            int maxScore;

            if (tryRead(maxScore, value, configVar)) {
                if (maxScore < 0 || maxScore > 5000)
                    printError(configVar + " value " + (maxScore < 0 ? "cannot be negative" : "is too big"));
                else
                    GameConfigVariables::maxScore = maxScore;
            }
        }
        else if (configVar == "wins") {
            int wins;

            if (tryRead(wins, value, configVar)) {
                if (wins < 0)
                    printError("\"" + configVar + "\" value cannot be negative");
                else
                    GameConfigVariables::wins = wins;
            }
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
            
            if (tryRead(deaths, value, configVar)) {
                if (deaths < 0)
                    printError("\"" + configVar + "\" value cannot be negative");
                else
                    GameConfigVariables::deaths = deaths;
            }
        } 
        else if (configVar == "timeAtGame") {

        } 
        else if (configVar == "gameMode") {
            if (value == "DEFAULT")
                GameConfigVariables::gameMode = GameMode::DEFAULT;
            else if (value == "THROUGH_WALLS")
                GameConfigVariables::gameMode = GameMode::THROUGH_WALLS;
            else
                printError("\"" + configVar + "\" has invalid value");
        } 
        else if (configVar == "fieldSize") {
            if (value == "TINY")
                GameConfigVariables::fieldSize = FieldSize::TINY;
            else if (value == "SMALL")
                GameConfigVariables::fieldSize = FieldSize::SMALL;
            else if (value == "MEDIUM")
                GameConfigVariables::fieldSize = FieldSize::MEDIUM;
            else if (value == "BIG")
                GameConfigVariables::fieldSize = FieldSize::BIG;
            else if (value == "HUGE")
                GameConfigVariables::fieldSize = FieldSize::HUGE;
            else
                printError("\"" + configVar + "\" has invalid value");
        } 
        else if (configVar == "gameSpeed") {
            if (value == "VERY_SLOW")
                CoreConfigVariables::gameSpeed = 0.6f;
            else if (value == "SLOW")
                CoreConfigVariables::gameSpeed = 0.3f;
            else if (value == "MEDIUM")
                CoreConfigVariables::gameSpeed = 0.15f;
            else if (value == "FAST")
                CoreConfigVariables::gameSpeed = 0.1f;
            else
                printError("\"" + configVar + "\" has invalid value");
        } 
        else {
            printError("\"" + configVar + "\" is unknown configuration variable");
            return false;
        }
    }

    return true;
}

bool ConfigManager::saveFile() {
    std::ofstream out(file);
    if (!out.is_open())
        return false;

    std::string strFullscreen = WindowConfigVariables::fullscreen ? "true" : "false";

    std::string strGameMode;
    switch (GameConfigVariables::gameMode) {
        case GameMode::DEFAULT: strGameMode = "DEFAULT"; break;
        case GameMode::BOUNDLESS: strGameMode = "BOUNDLESS"; break;
        case GameMode::THROUGH_WALLS: strGameMode = "THROUGH_WALLS"; break;
    }

    std::string strFieldSize;
    switch (GameConfigVariables::fieldSize) {
        case FieldSize::TINY: strFieldSize = "TINY"; break;
        case FieldSize::SMALL: strFieldSize = "SMALL"; break;
        case FieldSize::MEDIUM: strFieldSize = "MEDIUM"; break;
        case FieldSize::BIG: strFieldSize = "BIG"; break;
        case FieldSize::HUGE: strFieldSize = "HUGE"; break;
    }

    std::string strGameSpeed;
    if (CoreConfigVariables::gameSpeed == 0.1f) {
        strGameSpeed = "FAST";
    } else if (CoreConfigVariables::gameSpeed == 0.15f) {
        strGameSpeed = "MEDIUM";
    } else if (CoreConfigVariables::gameSpeed == 0.3f) {
        strGameSpeed = "SLOW";
    } else if (CoreConfigVariables::gameSpeed == 0.6f) {
        strGameSpeed = "VERY_SLOW";
    }

    out << 
    "#Window information\n" \
    "fullscreen = " << strFullscreen << "\n" \
    "windowSize = " << WindowConfigVariables::windowWidth << " " 
                    << WindowConfigVariables::windowHeight << "\n" \
    "\n#Game information\n" \
    "maxScore = " << GameConfigVariables::maxScore << "\n" \
    "wins = " << GameConfigVariables::wins << "\n" \
    "tinyWins = " << GameConfigVariables::tinyWins << "\n" \
    "mediumWins = " << GameConfigVariables::mediumWins << "\n" \
    "bigWins = " << GameConfigVariables::bigWins << "\n" \
    "hugeWins = " << GameConfigVariables::hugeWins << "\n" \
    "deaths = " << GameConfigVariables::deaths << "\n" \
    "timeAtGame = " << GameConfigVariables::timeAtGame << "\n" \
    "gameMode = " << strGameMode << "\n" \
    "fieldSize = " << strFieldSize <<  "\n" \
    "\n#Core information\n"
    "gameSpeed = " << strGameSpeed << "\n";

    return true;
}
