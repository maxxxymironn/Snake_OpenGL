#pragma once

enum class Action {
    EXIT,
    PAUSE,
    SPACE,
    YES,
    NO,

    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,

    SCALE_UP,
    SCALE_DOWN,
    ZEN_MODE,

    PRAVEDNOV,

    COUNT
};

enum class Mod {
    EMPTY,
    SHIFT,
    CTRL,
    ALT,
};