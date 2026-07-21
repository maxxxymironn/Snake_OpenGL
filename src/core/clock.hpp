#pragma once

#include "../config/core_config.hpp"

#include <chrono>
#include <cmath>

class Clock {
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point lastTime;
    std::chrono::steady_clock::time_point curTime;
    std::chrono::duration<float> delta;

    // game speed
    float stepTime;
    float gameStepAccumulator = 0;

    float appleBreathingCoeff = 0;
    float snakeMovingCoeff = 1; // 1 cause snake head should be on its place at startGame at first time

    // fps
    float fpsAccumulator = 0;
    unsigned int frames = 0;
    unsigned int prevFrames = 0;

    bool onPause = false;
    bool isSnakeFreezed = true;

public:
    Clock(): stepTime(CoreConfig::gameSpeed) {}
    ~Clock() {
        CoreConfig::gameSpeed = stepTime;
    }

    void start() { startTime = lastTime = std::chrono::steady_clock::now(); }

    void calculate() {
        // game step
        curTime = std::chrono::steady_clock::now();
        delta = curTime - lastTime;
        lastTime = curTime;

        // fps
        fpsAccumulator += delta.count();
        ++frames;

        if (!onPause) {
            gameStepAccumulator += delta.count();
            if (!isSnakeFreezed)
                snakeMovingCoeff = gameStepAccumulator / stepTime;
        }

        delta = curTime - startTime;
        appleBreathingCoeff = 1.f + 0.07f * std::cos(delta.count() * 4.f);
    }

    bool updateFPS() {
        bool isFpsChanged = false;

	    if (fpsAccumulator >= 1.f) {
	    	if (frames != prevFrames) {
	    		prevFrames = frames;
	    		isFpsChanged = true;
	    	}
	    	fpsAccumulator -= 1.f;
	    	frames = 0;
	    }
	    return isFpsChanged;
    }

    float getAppleBreathingCoeff() const { return appleBreathingCoeff; }
    float getSnakeMovingCoeff() const { return snakeMovingCoeff; }
    float getPrevFrames() const { return prevFrames; }

    bool isUpdateTime(float time=CoreConfig::gameSpeed) const { return gameStepAccumulator >= time; }
    void updateGameStepAccumulator() { gameStepAccumulator -= stepTime; }
    void resetGameStepAccumulator() { gameStepAccumulator = 0.f; }

    void updatePauseStatus() { onPause = !onPause; }
    void freezeSnake() { isSnakeFreezed = true; }
    void unfreezeSnake() { isSnakeFreezed = false; }
};
