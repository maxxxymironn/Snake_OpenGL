#pragma once
#include "apple.hpp"
#include "../core/vector.hpp"
#include "enums.hpp"
#include "field.hpp"
#include "snake.hpp"

class Game {
    Field m_field;
    Snake m_snake;
    Apple m_apple;

    vec4f _snakeColor;
    vec4f _themeColor;

    GameStatus m_status;
    GameMode m_mode;

    int m_score;

    bool generateApple;

    bool checkLoose(vec2i& newHead);
public:
    Game();
    ~Game();
    
    GameStatus status() const { return m_status; }
    GameMode mode() const { return m_mode; }

    vec4f getSnakeColor() const { return _snakeColor; }
    vec4f getThemeColor() const { return _themeColor; }

    const Apple& apple() const { return m_apple; }
    const Field& field() const { return m_field; }
    const Snake& snake() const { return m_snake; }

    Apple& apple() { return m_apple; }
    Snake& snake() { return m_snake; }

    void update();
    void reset();
    
    void updateStatus(GameStatus status);
    void saveStats();
};