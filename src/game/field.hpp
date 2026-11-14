#pragma once

#include "src/core/vector.hpp"

#include <vector>

class Field {
  public:
    Field(const vec2i size);

    void reset();

    auto size() const -> vec2i { return _size; }
    auto getFreeCells() const -> const std::vector<vec2i>& { return _freeCells; }
    auto getFreeCellsSize() const -> std::size_t { return _freeCells.size(); }
    void addFreeCell(const vec2i cell) { _freeCells.push_back(cell); }
    void removeFreeCell(const vec2i cell);

  private:
    vec2i _size;
    std::vector<vec2i> _freeCells;
};
