#pragma once
#include "../game/enums.hpp"
#include "../core/vector.hpp"

struct GameConfig {
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
    inline static int xFieldSize = 20;
    inline static int yFieldSize = 20;

    inline static vec4f snakeColor = { 0.15f, 0.40f, 0.20f, 1.f };
    inline static vec4f themeColor = { 0.20f, 0.20f, 0.20f, 1.f };
};