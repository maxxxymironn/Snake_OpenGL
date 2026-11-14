#pragma once

#include "../core/vector.hpp"

#include <vector>
#include <optional>

class Apple {
  public:
    void generate(const std::vector<vec2i>& freeCells);
    void deletePosition() { _position = std::nullopt; }
    auto getPosition() const -> std::optional<vec2i> { return _position; }
  private:
    std::optional<vec2i> _position;
};
