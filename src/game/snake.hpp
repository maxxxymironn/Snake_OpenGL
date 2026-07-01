#pragma once
#include "cell.hpp"

#include <vector>

struct Direction {
    static constexpr Cell    UP = { 0, 1 };
    static constexpr Cell  DOWN = { 0,-1 };
    static constexpr Cell RIGHT = { 1, 0 };
    static constexpr Cell  LEFT = {-1, 0 };
};

class Snake {
private:
    std::vector<Cell> m_body;
    std::vector<Cell>::iterator m_headIt;
    Cell m_prevTailCoord;
    Cell m_direction;

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
    void move(const Cell newHead);

    const std::vector<Cell>& getBody() const { return m_body; }

    Cell getDirection() const { return m_direction; }
    Cell getPrevTail() const { return m_prevTailCoord; }
    Cell getHeadPos() const { return *m_headIt; }
    
    std::size_t getLength() const { return m_body.size(); }

    void setDirection(const Cell newDirection) { m_direction = newDirection; }

    void reset(const int width, const int height);
};