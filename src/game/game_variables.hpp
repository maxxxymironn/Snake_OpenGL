#pragma once
#include "enums.hpp"

struct GameConfigVariables {
    inline static int snakeMaxLength = 0;

    inline static int countOfWins = 0;
    inline static int tinyWins = 0;
    inline static int smallWins = 0;
    inline static int mediumWins = 0;
    inline static int bigWins = 0;
    inline static int hugeWins = 0;

    inline static int countOfDeaths = 0;
    inline static int timeAtGame = 0;     // TODO: change variable type on std::chrono::duration?
    
    inline static GameMode gameMode = GameMode::DEFAULT;
    inline static int cellCountX = 20;
    inline static int cellCountY = 20;
};