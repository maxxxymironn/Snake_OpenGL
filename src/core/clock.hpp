#pragma once

#include "../config/core_config.hpp"

#include <chrono>
#include <cmath>

class Clock {
    std::chrono::steady_clock::time_point _startTime;
    std::chrono::steady_clock::time_point _lastTime;
    std::chrono::steady_clock::time_point _curTime;
    std::chrono::duration<float> _delta;

    // game speed
    float _stepTime;
    float _gameStepAccumulator;
    float _eyeBlinkAccumulator;

    float _appleBreathingCoeff;
    float _snakeMovingCoeff;

    // fps
    float _fpsAccumulator;
    unsigned int _frames;
    unsigned int _prevFrames;

    bool _onPause;
    bool _isSnakeFreezed;
    bool _blinking;

public:
    Clock()
        : _stepTime(CoreConfig::gameSpeed),
          _gameStepAccumulator(0.f),
          _eyeBlinkAccumulator(0.f),
          _appleBreathingCoeff(0.f),
          _snakeMovingCoeff(1.f),
          _fpsAccumulator(0.f),
          _frames(0),
          _prevFrames(0),
          _onPause(false),
          _isSnakeFreezed(true),
          _blinking(false) {}
          
    ~Clock() {
        CoreConfig::gameSpeed = _stepTime;
    }

    void start() { _startTime = _lastTime = std::chrono::steady_clock::now(); }

    void calculate() {
        // game step
        _curTime = std::chrono::steady_clock::now();
        _delta = _curTime - _lastTime;
        _lastTime = _curTime;

        // fps
        _fpsAccumulator += _delta.count();
        ++_frames;

        // eye blink
        _eyeBlinkAccumulator += _delta.count();
        if (!_blinking && _eyeBlinkAccumulator >= 6.f) {
            _blinking = true;
            _eyeBlinkAccumulator = 0.f;
        }
        else if (_blinking && _eyeBlinkAccumulator > 0.2f) {
            _blinking = false;
            _eyeBlinkAccumulator = 0.f;
        }

        if (!_onPause) {
            _gameStepAccumulator += _delta.count();
            if (!_isSnakeFreezed) {
                float newSnakeMovingCoeff = _gameStepAccumulator / _stepTime;
                _snakeMovingCoeff = newSnakeMovingCoeff < 1.f ? newSnakeMovingCoeff : 1.f;
            }
        }

        _delta = _curTime - _startTime;
        _appleBreathingCoeff = 1.f + 0.07f * std::cos(_delta.count() * 4.f);
    }

    bool updateFPS() {
        bool isFpsChanged = false;

	    if (_fpsAccumulator >= 1.f) {
	    	if (_frames != _prevFrames) {
	    		_prevFrames = _frames;
	    		isFpsChanged = true;
	    	}
	    	_fpsAccumulator -= 1.f;
	        _frames = 0;
	    }
	    return isFpsChanged;
    }

    float getAppleBreathingCoeff() const { return _appleBreathingCoeff; }
    float getSnakeMovingCoeff() const { return _snakeMovingCoeff; }
    float getPrevFrames() const { return _prevFrames; }

    bool isUpdateTime(float time=CoreConfig::gameSpeed) const { return _gameStepAccumulator >= time; }
    bool isBlinkTime() const { return _blinking; }
    bool isPauseTime() const { return _onPause; }

    void updateGameStepAccumulator() { _gameStepAccumulator -= _stepTime; }
    void resetGameStepAccumulator() { _gameStepAccumulator = 0.f; }

    void updatePauseStatus() { _onPause = !_onPause; }
    void freezeSnake() { _isSnakeFreezed = true; }
    void unfreezeSnake() { _isSnakeFreezed = false; }
};
