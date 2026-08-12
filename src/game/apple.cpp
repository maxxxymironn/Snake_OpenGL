#include "apple.hpp"
#include <random>

void Apple::generateApple(const std::vector<vec2i>& freeCells) {
    // Get seed
    std::random_device seed;
    // func get seed and generate random value
    std::mt19937 rand(seed());

    std::uniform_int_distribution<int> dist(0, freeCells.size() - 1);
    _position = freeCells[dist(rand)];

    _isNew = true;
}