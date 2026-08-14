#include "snake.hpp"

void Snake::increase() {
    m_body.push_back(_prevTail);
    _head = m_body.front();
}

void Snake::move(const vec2i newHead) {
    _prevTail = m_body.back();

    for (std::size_t i = m_body.size() - 1; i > 0; --i)
        m_body[i] = m_body[i - 1];

    _head = m_body.front() = newHead;
    
    // _hasNewDir = false;
}

void Snake::reset(const int width, const int height) {
    m_body.clear();
    m_body = std::vector<vec2i>{
        { width / 2, height / 2 },
        { width / 2 - 1, height / 2 }
    };

    _head = m_body.front();
    _prevTail = m_body.back();
    m_direction = Direction::RIGHT;
}