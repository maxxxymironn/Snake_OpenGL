#include "game.hpp"

#include "src/config/game_config.hpp"
#include "enums.hpp"

namespace {
    bool checkLoose(const GameMode mode, vec2i& newHead, const std::vector<vec2i>& snakeBody, const vec2i fieldSize) {
        bool headThroughWall = newHead.x == fieldSize.x
                             || newHead.y == fieldSize.y
                             || newHead.x == -1
                             || newHead.y == -1;
        if (headThroughWall) {
            if (mode == GameMode::DEFAULT)
                return true;

            if (newHead.x == fieldSize.x)
                newHead.x = 0;
            else if (newHead.x == -1)
                newHead.x = fieldSize.x - 1;
            else if (newHead.y == fieldSize.y)
                newHead.y = 0;
            else
                newHead.y = fieldSize.y - 1;
        }

        for (auto it = ++snakeBody.rbegin(); it + 1 != snakeBody.rend(); ++it) {
            if (newHead == *it)
                return true;
        }

        return false;
    }
}

Game::Game()
    : _field({ static_cast<int>(GameConfig::xFieldSize)
             , static_cast<int>(GameConfig::yFieldSize) })
    , _snake({ static_cast<int>(GameConfig::xFieldSize)
             , static_cast<int>(GameConfig::yFieldSize) })
    , _status(GameStatus::MENU)
    , _prevStatus(_status)
    , _mode(GameConfig::gameMode)
    , _score(0)
    , _bestScore(GameConfig::maxScore)
    , _needToGenApple(false)
{
    for (const vec2i bodyEl : _snake.getBody())
        _field.removeFreeCell(bodyEl);

    _apple.generate(_field.getFreeCells());
    _field.removeFreeCell(_apple.getPosition().value());
}

Game::~Game() { saveStats(); }

void Game::update() {
    vec2i newHead = _snake.getHeadPos() + _snake.getDirection();

    if (_needToGenApple) {
        if (++_score > _bestScore)
            _bestScore = _score;

        if (_field.getFreeCellsSize() == 0) {
            _snake.move(newHead);
            _apple.deletePosition();
            updateStatus(GameStatus::WIN);
            return;
        }

        _apple.generate(_field.getFreeCells());
        _field.removeFreeCell(_apple.getPosition().value());
        _needToGenApple = false;
    }

    if (checkLoose(_mode, newHead, _snake.getBody(), _field.size())) {
        updateStatus(GameStatus::LOOSE);
        _snake.move(newHead);
        return;
    }

    _snake.move(newHead);

    if (newHead == _apple.getPosition().value()) {
        _snake.increase();
        _needToGenApple = true;
    }
    else {
        _field.addFreeCell(_snake.getPrevTail());
        _field.removeFreeCell(newHead);
    }
}

void Game::reset() {
    _field.reset();
    _snake.reset(_field.size());

    for (const vec2i& bodyCell : _snake.getBody())
        _field.removeFreeCell(bodyCell);
    _apple.generate(_field.getFreeCells());
    _field.removeFreeCell(_apple.getPosition().value());

    _score = 0;
    _needToGenApple = false;
}

void Game::updateStatus(const GameStatus newStatus) {
    _prevStatus = _status;
    _status = newStatus;
}

void Game::saveStats() {
    GameConfig::maxScore = _bestScore;

    switch (_status) {
        case GameStatus::LOOSE: ++GameConfig::deaths; break;
        case GameStatus::WIN:   ++GameConfig::wins; break;
        default: break;
    }
}
