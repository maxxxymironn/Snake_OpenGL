#include "input.hpp"
#include <cstring>

void Input::update() { std::memcpy(previousKeys, currentKeys, sizeof(currentKeys)); }

void Input::setKey(Action key, bool pressed) { 
    currentKeys[static_cast<int>(key)] = pressed;

    if (bufferIsWorking
        && isKeyPressed(key)
        && (key == Action::MoveUp || key == Action::MoveLeft 
        || key == Action::MoveDown || key == Action::MoveRight) 
        ) 
    {
        if (inputBuffer.size() < 2) {
            if (!inputBuffer.empty()) {
                if (inputBuffer.back() != key)
                    inputBuffer.push(key);
                return;
            }
            inputBuffer.push(key);
        }
    }
}

bool Input::isKeyDown(Action key)    { return  currentKeys[static_cast<int>(key)]; }
bool Input::isKeyPressed(Action key) { return  currentKeys[static_cast<int>(key)] && !previousKeys[static_cast<int>(key)]; }
bool Input::isKeyRelease(Action key) { return !currentKeys[static_cast<int>(key)] && previousKeys[static_cast<int>(key)]; }

Action Input::getKeyFromBuffer() {
    Action returnKey = inputBuffer.front();
    inputBuffer.pop();

    return returnKey;
}