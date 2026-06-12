#include "game.hpp"
#include "game_variables.hpp"
#include "snake.hpp"

Game::Game()
    : m_field(
        GameConfigVariables::cellCountX,
        GameConfigVariables::cellCountY
      ),
      m_snake(GameConfigVariables::cellCountX, GameConfigVariables::cellCountY),
      m_apple({-1, -1}),
      m_status(GameStatus::GAME_START),
      m_prevStatus(m_status),
      m_mode(GameConfigVariables::gameMode),
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
            m_field.addFreeCell(m_snake.getPrevTail());
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

    m_status = GameStatus::GAME_START;
}

void Game::changeStatus(GameStatus status) {
    if (status == GameStatus::PAUSE) {
        if (m_status != GameStatus::PAUSE) {
            m_prevStatus = m_status;
            m_status = status;
        } else {
            m_status = m_prevStatus;
        }
    }
    else if (status == GameStatus::GAME && m_status == GameStatus::GAME_START) {
        m_status = GameStatus::GAME;
    }
}