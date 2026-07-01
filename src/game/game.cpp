#include "game.hpp"
#include "snake.hpp"
#include "../config/game_config.hpp"

bool checkHeadThroughWalls(const Cell newHead, const Cell fieldSize) {
    return newHead.x == fieldSize.x || newHead.x == -1 || newHead.y == fieldSize.y || newHead.y == -1;
}

void correctNewHead(Cell& newHead, Cell fieldSize) {
    if (newHead.x == fieldSize.x)
        newHead.x = 0;
    else if (newHead.x == -1)
        newHead.x = fieldSize.x - 1;
    else if (newHead.y == fieldSize.y)
        newHead.y = 0;
    else
        newHead.y = fieldSize.y - 1;
}

Game::Game()
    : m_field(
        static_cast<int>(GameConfigVariables::fieldSize),
        static_cast<int>(GameConfigVariables::fieldSize)
      ),
      m_snake(static_cast<int>(GameConfigVariables::fieldSize), static_cast<int>(GameConfigVariables::fieldSize)),
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

bool Game::checkLoose(Cell& newHead) {
    const Cell fieldSize = m_field.getFieldSize();

    if (checkHeadThroughWalls(newHead, fieldSize)) {
        if (m_mode == GameMode::DEFAULT)
            return true;

        correctNewHead(newHead, fieldSize);
    }
    
    std::vector<Cell> snakeBody = m_snake.getBody();
    for (auto it = ++snakeBody.rbegin(); it + 1 != snakeBody.rend(); ++it) {
        if (newHead == *it)
            return true;
    }

    return false;
}

void Game::update() {
    Cell newHead = m_snake.getHeadPos() + m_snake.getDirection();

    if (generateApple) {
        m_apple.generateApple(m_field.getFreeCells());
        m_field.removeFreeCell(m_apple.getPosition());
        generateApple = false;
    }

    if (checkLoose(newHead)) {
        m_status = GameStatus::LOOSE;
        return;
    }

    m_snake.move(newHead);
    if (newHead == m_apple.getPosition()) {
        m_snake.increase();
        if (m_field.getFreeCellsSize() == 0) {
            m_status = GameStatus::WIN;
            return;
        }
        generateApple = true;
    }
    else {
        m_field.addFreeCell(m_snake.getPrevTail());
        m_field.removeFreeCell(newHead);
    }
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

void Game::updateStatus(GameStatus status) {
    if (status == GameStatus::PAUSE) {
        if (m_status != GameStatus::PAUSE) {
            m_prevStatus = m_status;
            m_status = status;
        } else {
            m_status = m_prevStatus;
            m_prevStatus = status;
        }
    } else if (status == GameStatus::GAME && m_status == GameStatus::GAME_START) {
        m_status = GameStatus::GAME;
    }
}