#pragma once
#include "action.hpp"
#include <queue>

class Input {
    inline static bool currentKeys[static_cast<int>(Action::COUNT)]{0};
    inline static bool previousKeys[static_cast<int>(Action::COUNT)]{0};
    inline static bool bufferIsWorking = false;

    inline static std::queue<Action> inputBuffer;

public:
    static void update();
    static void setKey(Action key, bool pressed);

    static bool isKeyDown(Action key);
    static bool isKeyPressed(Action key);
    static bool isKeyRelease(Action key);

    static bool bufferIsNotEmpty() { return !inputBuffer.empty(); }
    
    static Action getKeyFromBuffer();

    static void clearBuffer() {
        while (!inputBuffer.empty()) {
            inputBuffer.pop();
        }
        bufferIsWorking = false;
    }

    static void changeWorkingMode() { bufferIsWorking = !bufferIsWorking; }
};
