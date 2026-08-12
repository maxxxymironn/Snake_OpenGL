#include "field.hpp"

Field::Field(const int weight, const int height) 
        : _weight(weight), _height(height),
          _freeCells(weight * height, { 0, 0 }
) {
    int i = 0;
    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _weight; ++x, ++i)
            _freeCells[i] = {x, y};
    }
}

void Field::removeFreeCell(const vec2i cell) {
    for (auto it = _freeCells.begin(); it != _freeCells.end(); ++it) {
        if (*it == cell) {
            *it = _freeCells.back();
            _freeCells.pop_back();
            return;
        }
    }
}