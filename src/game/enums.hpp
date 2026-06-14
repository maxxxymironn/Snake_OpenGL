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

enum class FieldSize {
    TINY = 7,
    SMALL = 10,
    MEDIUM = 14,
    BIG = 20,
    HUGE = 28
};
