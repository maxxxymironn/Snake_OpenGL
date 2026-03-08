#pragma once
#include "cell.hpp"
#include <vector>

struct Direction {
    inline static const Cell    UP = { 0, 1 };
    inline static const Cell  DOWN = { 0,-1 };
    inline static const Cell RIGHT = { 1, 0 };
    inline static const Cell  LEFT = {-1, 0 };
};

class Snake {
private:
    Cell m_direction;
    bool m_haveNewDir;

    std::vector<Cell> m_body;
    std::vector<Cell>::iterator m_headIt;
    Cell m_lastTail;

public:
    Snake(const int& weight, const int& height)
        : m_body( { { weight / 2, height / 2 }, { weight / 2 - 1, height / 2 } } ), 
        m_direction(Direction::RIGHT), m_haveNewDir(false), m_headIt(m_body.begin()), m_lastTail(*m_body.rbegin()) {}

    void increase();
    void move(const Cell& newHead);
    void newHeadPos();

    const std::vector<Cell>& getBody() const { return m_body; }

    Cell getDirection() const { return m_direction; }
    Cell getLastTail() const { return m_lastTail; }
    Cell getHeadPos() const { return *m_headIt; }
    
    std::size_t getLength() const { return m_body.size(); }

    bool haveNewDir() const { return m_haveNewDir; }

    void setDirection(const Cell newDirection) { m_direction = newDirection; m_haveNewDir = false; }
    void setHaveNewDir() { m_haveNewDir = true; }

    void reset(const int& weight, const int& height);
};