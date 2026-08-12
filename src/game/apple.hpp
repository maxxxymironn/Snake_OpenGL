#pragma once
#include "../core/vector.hpp"
#include <vector>

class Apple {
    vec2i _position;
    bool _isNew;

public:
    Apple(): _isNew(false) {}
    void generateApple(const std::vector<vec2i>& freeCells);

    const vec2i getPosition() const { return _position; }

    bool isNew() const { return _isNew; }
    void setOld() { _isNew = false; }
};