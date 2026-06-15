#include "apple.hpp"
#include <random>

void Apple::generateApple(const std::vector<Cell>& freeCells) {
    // Get seed
    std::random_device seed;
    // Random func
    std::mt19937 rand(seed());

    std::uniform_int_distribution<int> dist(0, freeCells.size() - 1);
    m_position = freeCells[dist(rand)];

    m_isNew = true;
}