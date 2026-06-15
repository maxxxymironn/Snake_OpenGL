#pragma once
#include "../game/enums.hpp"

struct GameConfigVariables {
    inline static int maxScore = 0;

    inline static int wins = 0;
    inline static int tinyWins = 0;
    inline static int smallWins = 0;
    inline static int mediumWins = 0;
    inline static int bigWins = 0;
    inline static int hugeWins = 0;

    inline static int deaths = 0;
    inline static int timeAtGame = 0;     // TODO: change variable type on std::chrono::duration?
    
    inline static GameMode gameMode = GameMode::DEFAULT;
    inline static FieldSize fieldSize = FieldSize::BIG;
};