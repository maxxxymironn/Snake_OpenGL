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

    COUNT
};

enum class Mod {
    EMPTY,
    SHIFT,
    CTRL,
    ALT,
};