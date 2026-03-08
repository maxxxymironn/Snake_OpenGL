#include "snake.hpp"

void Snake::increase() {
    m_body.push_back(m_lastTail);
    m_headIt = m_body.begin();
}

void Snake::move(const Cell& newHead) {
    auto it = m_body.rbegin();
    m_lastTail = *it;
    for (it; it + 1 != m_body.rend(); ++it) {
        *it = *(it + 1);
    }

    *m_headIt = newHead;
}

void Snake::newHeadPos() {
    if ((*m_headIt).x == -1)
        (*m_headIt).x = 20;
    else if ((*m_headIt).y == 21)
        (*m_headIt).x = 0;
    else if ((*m_headIt).y == -1)
        (*m_headIt).y = 20;
    else
        (*m_headIt).y = 0;
}

void Snake::reset(const int& weight, const int& height) {
    m_body.clear();
    m_body = std::vector<Cell>{{ weight / 2, height / 2 }, { weight / 2 - 1, height / 2 }};

    m_direction = Direction::RIGHT;
    m_haveNewDir = false;

    m_headIt = m_body.begin();
    m_lastTail = *m_body.rbegin();
}