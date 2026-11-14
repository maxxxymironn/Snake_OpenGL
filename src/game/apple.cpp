#include "apple.hpp"

#include <random>

void Apple::generate(const std::vector<vec2i>& freeCells) {
    std::random_device seed;
    std::mt19937 rand(seed());

    std::uniform_int_distribution<int> dist(0, freeCells.size() - 1);
    _position = freeCells[dist(rand)];
}
