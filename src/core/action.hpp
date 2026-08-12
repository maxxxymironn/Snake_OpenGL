#pragma once

enum class Action {
    Exit,
    Pause,

    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,

    ScaleUp,
    ScaleDown,
    ZenMode,

    COUNT
};

enum class Mod {
    EMPTY,
    SHIFT,
    CTRL,
    ALT,
};