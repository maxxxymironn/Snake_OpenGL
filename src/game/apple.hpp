#pragma once
#include "cell.hpp"
#include <vector>

class Apple {
    Cell m_position;
    bool m_isNew;

public:
    Apple(): m_isNew(false) {}
    void generateApple(const std::vector<Cell>& freeCells);

    const Cell& getPosition() const { return m_position; }

    bool isNew() const { return m_isNew; }
    void setOld() { m_isNew = false; }
};