#pragma once

#include "src/core/vector.hpp"

struct DrawConfig {
    inline static int contentScale = 100;
    inline static bool zenMode = false;

    inline static vec4f snakeColor = { 0.15f, 0.40f, 0.20f, 1.f };
    inline static vec4f themeColor = { 0.20f, 0.20f, 0.20f, 1.f };
};