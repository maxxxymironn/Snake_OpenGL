#pragma once
#include "../core/vector.hpp"
#include <vector>

struct Direction {
    static constexpr vec2i    UP{ 0, 1 };
    static constexpr vec2i  DOWN{ 0,-1 };
    static constexpr vec2i RIGHT{ 1, 0 };
    static constexpr vec2i  LEFT{-1, 0 };
};

class Snake {
private:
    vec2i _direction;
    std::vector<vec2i> _body;
    vec2i _head;
    vec2i _prevTail;
    vec2i _prevPrevTail;

    // bool _hasNewDir = false; // <-- render brokes when it's uncomment

public:
    Snake(const int width, const int height):
        _direction(Direction::RIGHT),
        _body({
            { width / 2, height / 2 },
            { width / 2 - 1, height / 2 }
        }), 
        _head(_body[0]), 
        _prevTail(_body.back() - _direction),
        _prevPrevTail(_prevTail - _direction) {}

    void increase();
    void move(const vec2i newHead);

    const std::vector<vec2i>& getBody() const { return _body; }

    vec2i getDirection() const { return _direction; }
    vec2i getPrevTail() const { return _prevTail; }
    vec2i getHeadPos() const { return _head; }
    
    std::size_t getLength() const { return _body.size(); }

    void setDirection(const vec2i newDirection) { _direction = newDirection; }

    void reset(const int width, const int height);
};