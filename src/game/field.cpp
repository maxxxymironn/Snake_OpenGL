#include "field.hpp"

Field::Field(const vec2i size)
    : _size(size)
    , _freeCells(size.x * size.y) 
    { reset(); }

void Field::removeFreeCell(const vec2i cell) {
    for (auto it = _freeCells.begin(); it != _freeCells.end(); ++it) {
        if (*it == cell) {
            *it = _freeCells.back();
            _freeCells.pop_back();
            return;
        }
    }
}

void Field::reset() {
    _freeCells.resize(_size.x * _size.y);
    for (int y = 0, i = 0; y < _size.y; ++y) {
        for (int x = 0; x < _size.x; ++x, ++i)
            _freeCells[i] = {x, y};
    }
}
