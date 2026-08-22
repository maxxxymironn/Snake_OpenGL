#pragma once

enum class Action {
    Exit,
    Pause,
    Space,

    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,

    ScaleUp,
    ScaleDown,
    ZenMode,

    EasterEgg,

    COUNT
};

enum class Mod {
    EMPTY,
    SHIFT,
    CTRL,
    ALT,
};