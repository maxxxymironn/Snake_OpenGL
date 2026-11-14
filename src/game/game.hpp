#pragma once
#include "apple.hpp"
#include "enums.hpp"
#include "field.hpp"
#include "snake.hpp"

class Game {
  public:
    Game();
    ~Game();

    auto status() const -> GameStatus { return _status; }
    auto mode() const -> GameMode { return _mode; }

    auto getScore() const -> int { return _score; }
    auto getBestScore() const -> int { return _bestScore; }

    auto apple() const -> const Apple& { return _apple; }
    auto field() const -> const Field& { return _field; }
    auto snake() const -> const Snake& { return _snake; }

    auto apple() -> Apple& { return _apple; }
    auto snake() -> Snake& { return _snake; }

    void update();
    void reset();

    void updateStatus(GameStatus status);

    void saveStats();

  private:
    Field _field;
    Snake _snake;
    Apple _apple;

    GameStatus _status;
    GameStatus _prevStatus;
    GameMode _mode;

    int _score;
    int _bestScore;

    bool _needToGenApple;
};
