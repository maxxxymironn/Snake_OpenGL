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
    std::vector<vec2i> m_body;
    vec2i _head;
    vec2i _prevTail;
    vec2i m_direction;

    // bool _hasNewDir = false; // <-- render brokes when it's uncomment

public:
    Snake(const int width, const int height):
        m_body({
            { width / 2, height / 2 },
            { width / 2 - 1, height / 2 }
        }), 
        _head(m_body[0]), 
        _prevTail(m_body.back()),
        m_direction(Direction::RIGHT) {}

    void increase();
    void move(const vec2i newHead);

    const std::vector<vec2i>& getBody() const { return m_body; }

    vec2i getDirection() const { return m_direction; }
    vec2i getPrevTail() const { return _prevTail; }
    vec2i getHeadPos() const { return _head; }
    
    std::size_t getLength() const { return m_body.size(); }

    void setDirection(const vec2i newDirection) { m_direction = newDirection; }

    void reset(const int width, const int height);
};