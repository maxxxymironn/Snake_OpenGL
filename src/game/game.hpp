#pragma once
#include "apple.hpp"
#include "cell.hpp"
#include "field.hpp"
#include "snake.hpp"
#include "enums.hpp"

class Game {
    Field m_field;
    Snake m_snake;
    Apple m_apple;

    GameStatus m_status;
    GameStatus m_prevStatus;

    GameMode m_mode;

    bool generateApple;

    bool checkLoose(Cell& nextHeadPos, const Cell& fieldSize);

public:
    Game();
    
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }

    const Apple& apple() const { return m_apple; }
    const Field& field() const { return m_field; }
    const Snake& snake() const { return m_snake; }

    Apple& apple() { return m_apple; }
    Snake& snake() { return m_snake; }

    void update();
    void reset();
    
    void changeStatus(GameStatus status=GameStatus::PAUSE);
};