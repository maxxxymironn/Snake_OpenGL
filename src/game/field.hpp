#pragma once
#include "cell.hpp"
#include <vector>

class Field {
    int m_weight;
    int m_height;

    std::vector<Cell> m_freeCells;

public:
    Field(const int weight, const int height);

    Cell getFieldSize() const { return {m_weight, m_height}; }

    const std::vector<Cell>& getFreeCells() const { return m_freeCells; }
    std::size_t getFreeCellsSize() const { return m_freeCells.size(); }
    
    void addFreeCell(const Cell cell);
    void removeFreeCell(const Cell cell);
};