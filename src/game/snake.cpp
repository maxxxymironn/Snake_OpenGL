#include "snake.hpp"

void Snake::increase() {
    m_body.push_back(m_prevTailCoord);
    m_headIt = m_body.begin();
}

void Snake::move(const Cell newHead) {
    auto it = m_body.rbegin();
    m_prevTailCoord = *it;
    for (it; it + 1 != m_body.rend(); ++it) {
        *it = *(it + 1);
    }

    *m_headIt = newHead;

    m_canGetNewDir = true;
}

void Snake::newHeadPos() {
    Cell head = *m_headIt;
    if (head.x == -1)
        head.x = 20;
    else if (head.y == 21)
        head.x = 0;
    else if (head.y == -1)
        head.y = 20;
    else
        head.y = 0;
}

void Snake::reset(const int weight, const int height) {
    m_body.clear();
    m_body = std::vector<Cell>{{ weight / 2, height / 2 }, { weight / 2 - 1, height / 2 }};

    m_direction = Direction::RIGHT;
    m_haveNewDir = false;

    m_headIt = m_body.begin();
    m_prevTailCoord = *m_body.rbegin();
}