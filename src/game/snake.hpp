#pragma once
#include "src/core/vector.hpp"
#include <vector>

struct Direction {
    static constexpr vec2i    UP{ 0, 1 };
    static constexpr vec2i  DOWN{ 0,-1 };
    static constexpr vec2i RIGHT{ 1, 0 };
    static constexpr vec2i  LEFT{-1, 0 };
};

class Snake {
  public:
    Snake(const vec2i fieldSize);

    void increase();
    void move(const vec2i newHead);

    auto getBody() const -> const std::vector<vec2i>& { return _body; }
    auto getLength() const -> unsigned int { return _body.size(); }
    auto getHeadPos() const -> vec2i { return _head; }
    auto getPrevTail() const -> vec2i { return _prevTail; }
    auto getPrevPrevTail() const -> vec2i { return _prevPrevTail; }

    void setDirection(const vec2i newDirection) { _direction = newDirection; }
    auto getDirection() const -> vec2i { return _direction; }

    void reset(const vec2i fieldSize);

  private:
    vec2i _direction;
    std::vector<vec2i> _body;
    vec2i _head;
    vec2i _prevTail;
    vec2i _prevPrevTail;
};
