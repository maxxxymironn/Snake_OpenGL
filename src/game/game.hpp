#pragma once
#include "apple.hpp"
#include "cell.hpp"
#include "field.hpp"
#include "snake.hpp"

enum class GameStatus {
    MENU,
    GAME,
    WIN,
    LOOSE
};

enum class GameMode {
    DEFAULT,
    THROUGH_WALLS
};

class Game {
    Field m_field;
    Snake m_snake;
    Apple m_apple;

    GameStatus m_status;
    GameStatus m_prevStatus = GameStatus::GAME;
    GameMode m_mode;

    bool generateApple;

    bool checkLoose(Cell& nextHeadPos, const Cell& fieldSize);

public:
    Game(const int& weight = 20, const int& height = 20, 
            const std::array<int, 3>& evenCellColor = { 30, 30, 30 }, 
            const std::array<int, 3>& oddCellColor = { 35, 35, 35 });
    
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }

    const Apple& apple() const { return m_apple; }
    const Field& field() const { return m_field; }
    const Snake& snake() const { return m_snake; }

    Apple& apple() { return m_apple; }
    Snake& snake() { return m_snake; }

    void update();
    void reset();
    void changeStatus();
};