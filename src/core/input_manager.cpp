#include "input_manager.hpp"
#include "action.hpp"

void InputManager::update() { _prevKeys = _curKeys; }

void InputManager::setKey(Action key, bool pressed, Mod mods) {
    if (key == Action::SCALE_UP || key == Action::SCALE_DOWN || key == Action::ZEN_MODE || key == Action::PRAVEDNOV) {
        if (mods == Mod::CTRL)
            _curKeys[static_cast<int>(key)] = pressed;
        return;
    }

    if (mods != Mod::EMPTY)
        return;

    _curKeys[static_cast<int>(key)] = pressed;

    if (_isBufferWorking
        && isKeyPressed(key)
        && (key == Action::MOVE_UP || key == Action::MOVE_LEFT
        || key == Action::MOVE_DOWN || key == Action::MOVE_RIGHT)
    ) {
        if (_buffer.size() < 2) {
            if (!_buffer.empty()) {
                if (_buffer.back() != key)
                    _buffer.push(key);
                return;
            }
            _buffer.push(key);
        }
    }
}

bool InputManager::isKeyDown(Action key) const { return _curKeys[static_cast<int>(key)]; }
bool InputManager::isKeyPressed(Action key) const {
    return _curKeys[static_cast<int>(key)] && !_prevKeys[static_cast<int>(key)];
}
bool InputManager::isKeyReleased(Action key) const {
    return !_curKeys[static_cast<int>(key)] && _prevKeys[static_cast<int>(key)];
}
