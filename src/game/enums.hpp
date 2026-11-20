#pragma once

enum class GameStatus: char {
    MENU,
    MENU_SETTINGS,
    GAME,
    GAME_START,
    GAME_WIN,
    GAME_LOOSE,
    GAME_SURE_EXIT,
};

enum class GameMode: char {
    DEFAULT,
    THROUGH_WALLS,
    BOUNDLESS
};

enum class Color: char {
    WHITE,
    BLACK,
    GRAY,
    RED,
    GREEN,
    BLUE,
    AQUA,
    PURPLE,
    PINK,
    ORANGE
};
