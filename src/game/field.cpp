#include "field.hpp"

Field::Field(const int& weight, const int& height) 
        : m_weight(weight), m_height(height),
          m_freeCells(weight * height, { 0, 0 }
) {
    unsigned short count = 0;
    for (int y = 0; y < m_weight; ++y) {
        for (int x = 0; x < m_height; ++x) {
            m_freeCells[count] = {x, y};
            ++count;
        }
    }
}

void Field::addFreeCell(const Cell& cell) { m_freeCells.push_back(cell); }

void Field::removeFreeCell(const Cell& cell) {
    for (auto it = m_freeCells.begin(); it != m_freeCells.end(); ++it) {
        if (*it == cell) {
            *it = m_freeCells.back();
            m_freeCells.pop_back();
            return;
        }
    }
}