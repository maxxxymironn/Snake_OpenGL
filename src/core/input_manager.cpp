#include "input_manager.hpp"
#include <cstring>

void InputManager::update() {
    std::memcpy(prevKeys, currentKeys, sizeof(currentKeys));
}

void InputManager::setKey(Action key, bool pressed) { 
    currentKeys[static_cast<int>(key)] = pressed;

    if (isBufferWorking
        && isKeyPressed(key)
        && (key == Action::MoveUp || key == Action::MoveLeft 
        || key == Action::MoveDown || key == Action::MoveRight) 
        ) 
    {
        if (buffer.size() < 2) {
            if (!buffer.empty()) {
                if (buffer.back() != key)
                    buffer.push(key);
                return;
            }
            buffer.push(key);
        }
    }
    
}

bool InputManager::isKeyDown(Action key) const { return  currentKeys[static_cast<int>(key)]; }
bool InputManager::isKeyPressed(Action key) const { return  currentKeys[static_cast<int>(key)] && !prevKeys[static_cast<int>(key)]; }
bool InputManager::isKeyReleased(Action key) const { return !currentKeys[static_cast<int>(key)] && prevKeys[static_cast<int>(key)]; }