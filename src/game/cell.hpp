#pragma once

struct Cell {
    int x, y;

    constexpr bool operator==(const Cell& other) const noexcept { 
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Cell& other) const noexcept {
        return !(*this == other);
    }

    constexpr Cell operator+(const Cell& other) const noexcept {
        return { other.x + this->x, other.y + this->y };
    }

    constexpr Cell operator-(const Cell& other) const noexcept {
        return { this->x - other.x, this->y - other.y };
    }

    constexpr Cell& operator+=(const Cell& other) noexcept {
        this->x = this->x + other.x;
        this->y = this->y + other.y;
        return *this;
    }
};