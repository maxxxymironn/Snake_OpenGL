#include "config_manager.hpp"

#include "../window/window_variables.hpp"
#include "../game/game_variables.hpp"
#include "core_variables.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

bool ConfigManager::upload() {
    std::ofstream outFile(filePath);

    if (!outFile.is_open()) {
        std::cerr << "ERROR: upload failed";
        return false;
    }

    constexpr const char* defaultFile = 
R"(# Window information
fullscreen = false
windowSize = 800 800

# Game information
maxLength = 0
deathsNumber = 0
winsNumber = 0
tinyWins = 0
smallWins = 0
mediumWins = 0
bigWins = 0
hugeWins = 0
timeAtGame = 0

# Game config
gameMode = THROUGH_WALLS
fieldSize = BIG
gameSpeed = STANDARD

)";

    outFile << defaultFile;

    return true;
}

bool ConfigManager::unload() {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        std::cerr << "ERROR: can't open file\n";
        upload();
        return false;
    }

    std::string line;
    while (inFile) {
        getline(inFile, line);

        if (line.find_first_not_of(" \t\n\r") == std::string::npos || line.front() == '#') {
            continue;
        }

        std::stringstream ss(line);

        std::string configVar;
        ss >> configVar;

        std::string eq;
        ss >> eq;
        if (eq != "=") {
            std::cerr << "ERROR: missing \"=\" after config variable\n";
            return false;
        }

        std::string value;
        ss >> value;

        if (configVar == "fullscreen") {
            if (value == "true") {
                WindowConfigVariables::fullscreen = true;
            } else if (value == "false") {
                WindowConfigVariables::fullscreen = false;
            } else {
                std::cerr << "ERROR: fullscreen value is incorrect\n";
            }
        } else if (configVar == "windowSize") {
            // wrap all this in try catch
            std::string x = value, y;
            int iX, iY;
            ss >> y;
            iX = std::stoi(x);
            iY = std::stoi(y);

            WindowConfigVariables::windowWidth = iX;
            WindowConfigVariables::windowHeight = iY;
        } else if (configVar == "maxLength") {
            int maxLength = std::stoi(value);
            GameConfigVariables::snakeMaxLength = maxLength;
        } else if (configVar == "winsNumber") {
            int winsCount = std::stoi(value);
            GameConfigVariables::countOfWins = winsCount;
        } else if (configVar == "tinyWins") {

        } else if (configVar == "smallWins") {

        } else if (configVar == "mediumWins") {

        } else if (configVar == "bigWins") {

        } else if (configVar == "hugeWins") {

        } else if (configVar == "deathsNumber") {
            int deathCount = std::stoi(value);
            GameConfigVariables::countOfDeaths = deathCount;
        } else if (configVar == "timeAtGame") {

        } else if (configVar == "gameMode") {
            if (value == "DEFAULT") {
                GameConfigVariables::gameMode = GameMode::DEFAULT;
            } else if (value == "THROUGH_WALLS") {
                GameConfigVariables::gameMode = GameMode::THROUGH_WALLS;
            }
        } else if (configVar == "fieldSize") {
            if (value == "TINY") {
                GameConfigVariables::cellCountX = GameConfigVariables::cellCountY = 7;
            } else if (value == "SMALL") {
                GameConfigVariables::cellCountX = GameConfigVariables::cellCountY = 10;
            } else if (value == "MEDIUM") {
                GameConfigVariables::cellCountX = GameConfigVariables::cellCountY = 14;
            } else if (value == "BIG") {
                GameConfigVariables::cellCountX = GameConfigVariables::cellCountY = 20;
            } else if (value == "HUGE") {
                GameConfigVariables::cellCountX = GameConfigVariables::cellCountY = 28;
            } else if (value == "BIGGYBIG") {
                GameConfigVariables::cellCountX = GameConfigVariables::cellCountY = 40;
            } else {
                std::cerr << "ERROR: lastSizeMap value is incorrect\n";
            }
        } else if (configVar == "gameSpeed") {
            if (value == "VERY_SLOW") {
                CoreConfigVariables::gameSpeed = 0.6f;
            } else if (value == "SLOW") {
                CoreConfigVariables::gameSpeed = 0.3f;
            } else if (value == "STANDARD") {
                CoreConfigVariables::gameSpeed = 0.15f;
            } else if (value == "FAST") {
                CoreConfigVariables::gameSpeed = 0.1f;
            }
        } else {
            std::cerr << "ERROR: unknown config variable \"" << configVar << "\"\n";
            return false;
        }
    }

    return true;
}
