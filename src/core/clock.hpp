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
    float gameStepAccumulator;

    float appleBreathingCoeff;
    float snakeMovingCoeff;

    // fps
    float fpsAccumulator;
    unsigned int frames;
    unsigned int prevFrames;

    bool onPause;
    bool isSnakeFreezed;

public:
    Clock()
        : stepTime(CoreConfig::gameSpeed),
          gameStepAccumulator(0.f),
          appleBreathingCoeff(0.f),
          snakeMovingCoeff(1.f),
          fpsAccumulator(0.f),
          frames(0),
          prevFrames(0),
          onPause(false),
          isSnakeFreezed(true) {}
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
            if (!isSnakeFreezed) {
                float newSnakeMovingCoeff = gameStepAccumulator / stepTime;
                snakeMovingCoeff = newSnakeMovingCoeff < 1.f ? newSnakeMovingCoeff : 1.f;
            }
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
