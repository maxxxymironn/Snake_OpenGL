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
    std::vector<vec2i>::iterator m_headIt;
    vec2i m_prevTailCoord;
    vec2i m_direction;

public:
    Snake(const int width, const int height):
        m_body({
            { width / 2, height / 2 },
            { width / 2 - 1, height / 2 }
        }), 
        m_headIt(m_body.begin()), 
        m_prevTailCoord(*m_body.rbegin()),
        m_direction(Direction::RIGHT) {}

    void increase();
    void move(const vec2i newHead);

    const std::vector<vec2i>& getBody() const { return m_body; }

    vec2i getDirection() const { return m_direction; }
    vec2i getPrevTail() const { return m_prevTailCoord; }
    vec2i getHeadPos() const { return *m_headIt; }
    
    std::size_t getLength() const { return m_body.size(); }

    void setDirection(const vec2i newDirection) { m_direction = newDirection; }

    void reset(const int width, const int height);
};