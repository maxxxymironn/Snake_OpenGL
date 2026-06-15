#pragma once
#include "action.hpp"
#include <queue>

class InputManager {
    bool currentKeys[static_cast<int>(Action::COUNT)]{0};
    bool prevKeys[static_cast<int>(Action::COUNT)]{0};

    std::queue<Action> buffer;
    bool isBufferWorking;

public:
    void update();
    void setKey(Action key, bool pressed);

    bool isKeyDown(Action key) const;
    bool isKeyPressed(Action key) const;
    bool isKeyReleased(Action key) const;

    bool isBufferEmpty() const { return buffer.empty(); }

    Action getBufferKey() {
        Action key = buffer.front();
        buffer.pop();
        return key;
    }

    void turnOnBuffer() { isBufferWorking = true; }
    void turnOffBuffer() {
        while (!buffer.empty())
            buffer.pop();
        isBufferWorking = false;
    }

    void changeWorkStatus() { isBufferWorking = !isBufferWorking; }
};
