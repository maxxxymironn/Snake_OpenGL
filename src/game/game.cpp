#include "game.hpp"

Game::Game(
    const int& weight,
    const int& height,
    const std::array<int, 3>& evenCellColor,
    const std::array<int, 3>& oddCellColor)
    : m_field(weight, height, evenCellColor, oddCellColor),
      m_snake(weight, height),
      m_apple({-1, -1}),
      m_status(GameStatus::GAME),
      m_mode(GameMode::THROUGH_WALLS),
      generateApple(false)
{
    for (const Cell& bodyEl : m_snake.getBody())
        m_field.removeFreeCell(bodyEl);

    m_apple.generateApple(m_field.getFreeCells());
    m_field.removeFreeCell(m_apple.getPosition());
}

bool Game::checkLoose(Cell& nextHeadPos, const Cell& fieldSize) {
    if (nextHeadPos.x == fieldSize.x || nextHeadPos.x == -1 || 
        nextHeadPos.y == fieldSize.y || nextHeadPos.y == -1) {
        if (m_mode == GameMode::DEFAULT) {
            m_status = GameStatus::LOOSE;
            return false;
        }

        if (nextHeadPos.x == fieldSize.x)
            nextHeadPos.x = 0;
        else if (nextHeadPos.x == -1)
            nextHeadPos.x = fieldSize.x - 1;
        else if (nextHeadPos.y == fieldSize.y)
            nextHeadPos.y = 0;
        else
            nextHeadPos.y = fieldSize.y - 1;
    }
    
    std::vector<Cell> snakeBody = m_snake.getBody();
    for (auto it = ++snakeBody.rbegin(); it + 1 != snakeBody.rend(); ++it) {
        if (nextHeadPos == *it) {
            m_status = GameStatus::LOOSE;
            return false;
        }
    }

    return true;
}

void Game::update() {
    const Cell headPos = m_snake.getHeadPos();
    const Cell snakeDirection = m_snake.getDirection();
    const Cell fieldSize = m_field.getFieldSize();

    Cell newHeadPos = headPos + snakeDirection;
    
    if (generateApple) {
        m_apple.generateApple(m_field.getFreeCells());
        m_field.removeFreeCell(m_apple.getPosition());
        generateApple = false;
    }

    if (checkLoose(newHeadPos, fieldSize)) {
        m_snake.move(newHeadPos);
        
        if (newHeadPos == m_apple.getPosition()) {
            m_snake.increase();
            // check win
            if (m_snake.getLength() == fieldSize.x * fieldSize.y) {
                m_status = GameStatus::WIN;
                return;
            }
            generateApple = true;
        }
        else {
            m_field.addFreeCell(m_snake.getLastTail());
            m_field.removeFreeCell(newHeadPos);
        }
    }
    // TODO: else { deathAnimation }
}

void Game::reset() {
    for (const Cell& bodyEl : m_snake.getBody()) {
        m_field.addFreeCell(bodyEl);
    }
    m_field.addFreeCell(m_apple.getPosition());
    
    Cell size = m_field.getFieldSize();
    m_snake.reset(size.x, size.y);
    for (const Cell& bodyEl : m_snake.getBody()) {
        m_field.removeFreeCell(bodyEl);
    }
    m_apple.generateApple(m_field.getFreeCells());
    m_field.removeFreeCell(m_apple.getPosition());

    m_status = GameStatus::GAME;
}

void Game::changeStatus() {
    if (m_status != GameStatus::MENU) {
        m_prevStatus = m_status;
        m_status = GameStatus::MENU;
    }
    else
        m_status = m_prevStatus;
}