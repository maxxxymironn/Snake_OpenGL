#include "game.hpp"

#include "../config/game_config.hpp"
#include "enums.hpp"

bool checkHeadThroughWalls(const vec2i newHead, const vec2i fieldSize) {
    return newHead.x == fieldSize.x || newHead.x == -1 || newHead.y == fieldSize.y || newHead.y == -1;
}

void correctNewHead(vec2i& newHead, const vec2i fieldSize) {
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
        static_cast<int>(GameConfig::xFieldSize),
        static_cast<int>(GameConfig::yFieldSize)),
      m_snake(
        static_cast<int>(GameConfig::xFieldSize), 
        static_cast<int>(GameConfig::yFieldSize)),
      _snakeColor(GameConfig::snakeColor),
      _themeColor(GameConfig::themeColor),
      m_status(GameStatus::MENU),
      m_mode(GameConfig::gameMode),
      m_score(0),
      generateApple(false)
        {
    for (const vec2i bodyEl : m_snake.getBody())
        m_field.removeFreeCell(bodyEl);

    m_apple.generateApple(m_field.getFreeCells());
    m_field.removeFreeCell(m_apple.getPosition());
}

Game::~Game() {
    saveStats();
}

void Game::update() {
    vec2i newHead = m_snake.getHeadPos() + m_snake.getDirection();

    if (generateApple) {
        m_apple.generateApple(m_field.getFreeCells());
        m_field.removeFreeCell(m_apple.getPosition());
        generateApple = false;
    }

    if (checkLoose(newHead)) {
        m_status = GameStatus::LOOSE;
        m_snake.move(newHead);
        return;
    }

    m_snake.move(newHead);

    if (newHead == m_apple.getPosition()) {
        ++m_score;
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
    for (const vec2i& bodyEl : m_snake.getBody()) {
        m_field.addFreeCell(bodyEl);
    }
    m_field.addFreeCell(m_apple.getPosition());
    
    vec2i size = m_field.getFieldSize();
    m_snake.reset(size.x, size.y);

    for (const vec2i& bodyEl : m_snake.getBody()) {
        m_field.removeFreeCell(bodyEl);
    }
    m_apple.generateApple(m_field.getFreeCells());
    m_field.removeFreeCell(m_apple.getPosition());

    m_score = 0;
    m_status = GameStatus::GAME_START;
}

bool Game::checkLoose(vec2i& newHead) {
    const vec2i fieldSize = m_field.getFieldSize();

    if (checkHeadThroughWalls(newHead, fieldSize)) {
        if (m_mode == GameMode::DEFAULT)
            return true;

        correctNewHead(newHead, fieldSize);
    }
    
    std::vector<vec2i> snakeBody = m_snake.getBody();
    for (auto it = ++snakeBody.rbegin(); it + 1 != snakeBody.rend(); ++it) {
        if (newHead == *it)
            return true;
    }

    return false;
}

void Game::updateStatus(GameStatus status) {
    if (status == GameStatus::GAME)
        m_status = GameStatus::GAME;
    else if (status == GameStatus::MENU)
        m_status = GameStatus::MENU;
}

void Game::saveStats() {
    if (m_score > GameConfig::maxScore)
        GameConfig::maxScore = m_score;

    if (m_status == GameStatus::LOOSE)
        ++GameConfig::deaths;
    else if (m_status == GameStatus::WIN) {
        ++GameConfig::wins;
    }

    GameConfig::gameMode = m_mode;

    vec2i fieldSize = m_field.getFieldSize();
    GameConfig::xFieldSize = fieldSize.x;
    GameConfig::yFieldSize = fieldSize.y;
}

void Game::setDefaultHead() {
    _pravednovHead = false;
    _snakeColor = GameConfig::snakeColor;
}