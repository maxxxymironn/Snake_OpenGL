#pragma once
#include "../core/vector.hpp"
#include <vector>

class Field {
    int _weight;
    int _height;
    std::vector<vec2i> _freeCells;
public:
    Field(const int weight, const int height);

    vec2i getFieldSize() const { return {_weight, _height}; }

    const std::vector<vec2i>& getFreeCells() const { return _freeCells; }
    
    std::size_t getFreeCellsSize() const { return _freeCells.size(); }
    
    void addFreeCell(const vec2i cell) { _freeCells.push_back(cell); }
    
    void removeFreeCell(const vec2i cell);
};