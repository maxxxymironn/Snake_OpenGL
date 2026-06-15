#include "snake.hpp"

void Snake::increase() {
    m_body.push_back(m_prevTailCoord);
    m_headIt = m_body.begin();
}

void Snake::move(const Cell newHead) {
    auto it = m_body.rbegin();
    m_prevTailCoord = *it;
    for (it; it + 1 != m_body.rend(); ++it)
        *it = *(it + 1);

    *m_headIt = newHead;
}

void Snake::reset(const int width, const int height) {
    m_body.clear();
    m_body = std::vector<Cell>{
        { width / 2, height / 2 },
        { width / 2 - 1, height / 2 }
    };

    m_headIt = m_body.begin();
    m_prevTailCoord = *m_body.rbegin();
    m_direction = Direction::RIGHT;
}