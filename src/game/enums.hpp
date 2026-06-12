#pragma once

enum class GameStatus {
    MENU,
    GAME_START,
    GAME,
    WIN,
    LOOSE,
    PAUSE
};

enum class GameMode {
    DEFAULT,
    THROUGH_WALLS,
    BOUNDLESS
};

enum class SizeMap {
    TINY,
    SMALL,
    MEDIUM,
    BIG,
    HUGE
};