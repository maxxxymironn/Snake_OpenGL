#pragma once
#include <chrono>
struct Clock {
    inline static constexpr float stepTime = 0.15f;
    inline static float gameUpdateAccumulator = 0.f;
    inline static float tempAccum = 0.f;
    
    inline static float alpha = 0.f;

    inline static float fpsAccumulator = 0.f;
    inline static unsigned int frames = 0;

    inline static unsigned int counter = 0;

    inline static bool secondHolding = true;

    inline static std::chrono::steady_clock::time_point lastTime;
    inline static std::chrono::steady_clock::time_point currentTime;
    inline static std::chrono::duration<float> delta;

    static void startTime() {
        lastTime = std::chrono::steady_clock::now();
    }

    static void calculateTime() {
        currentTime = std::chrono::steady_clock::now();
        delta = currentTime - lastTime;
        lastTime = currentTime;

        alpha = gameUpdateAccumulator / stepTime;

        gameUpdateAccumulator += delta.count();
        fpsAccumulator += delta.count();
        ++frames;
    }

    static void memorizeAccum() {
        tempAccum = gameUpdateAccumulator;
    }

    static void restoreAccum() {
        gameUpdateAccumulator = tempAccum;
    }
};