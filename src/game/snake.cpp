#include "snake.hpp"

void Snake::increase() {
    _body.push_back(_prevTail);
    _head = _body.front();
    _prevTail = _prevPrevTail;
}

void Snake::move(const vec2i newHead) {
    _prevPrevTail = _prevTail;
    _prevTail = _body.back();

    for (std::size_t i = _body.size() - 1; i > 0; --i)
        _body[i] = _body[i - 1];

    _head = _body.front() = newHead;
}

void Snake::reset(const int width, const int height) {
    _body.clear();
    _body = std::vector<vec2i>{
        { width / 2, height / 2 },
        { width / 2 - 1, height / 2 }
    };

    _direction = Direction::RIGHT;
    _head = _body.front();
    _prevTail = _body.back() - _direction;
    _prevPrevTail = _prevTail - _direction;
}