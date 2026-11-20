#pragma once
#include "action.hpp"
#include <queue>
#include <array>

class InputManager {
    std::array<bool, static_cast<std::size_t>(Action::COUNT)> _curKeys;
    std::array<bool, static_cast<std::size_t>(Action::COUNT)> _prevKeys;
    
    std::queue<Action> _buffer;
    bool _isBufferWorking;

public:
    InputManager(): _curKeys({}), _prevKeys({}) {}
    void update();
    void setKey(Action key, bool pressed, Mod mods);

    bool isKeyDown(Action key) const;
    bool isKeyPressed(Action key) const;
    bool isKeyReleased(Action key) const;

    bool isBufferEmpty() const { return _buffer.empty(); }

    Action getBufferKey() {
        Action key = _buffer.front();
        _buffer.pop();
        return key;
    }

    void turnOnBuffer() { _isBufferWorking = true; }
    void turnOffBuffer() {
        while (!_buffer.empty())
            _buffer.pop();
        _isBufferWorking = false;
    }

    void changeWorkStatus() { _isBufferWorking = !_isBufferWorking; }
};
