#include "snake.hpp"

Snake::Snake(const vec2i fieldSize)
    : _direction(Direction::RIGHT)
    , _body({ fieldSize / 2, fieldSize / 2 - _direction })
    , _head(_body[0])
    , _prevTail(_body.back() - _direction)
    , _prevPrevTail(_prevTail - _direction) {}

void Snake::increase() {
    _body.push_back(_prevTail);
    _head = _body.front();
    _prevTail = _prevPrevTail;
}

void Snake::move(const vec2i newHead) {
    _prevPrevTail = _prevTail;
    _prevTail = _body.back();

    for (int i = _body.size() - 1; i > 0; --i)
        _body[i] = _body[i - 1];

    _head = _body.front() = newHead;
}

void Snake::reset(const vec2i fieldSize) {
    _body.clear();
    _direction = Direction::RIGHT;
    _body = { fieldSize / 2, fieldSize / 2 - _direction };
    _head = _body[0];
    _prevTail = _body.back() - _direction;
    _prevPrevTail = _prevTail - _direction;
}
