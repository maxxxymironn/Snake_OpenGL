#include "drawer.hpp"

#include "rectangle.hpp"
#include "texture_enum.hpp"
#include "src/config/draw_config.hpp"
#include "src/game/enums.hpp"

#include <string>

namespace {
    constexpr float PI = 3.14159265359f;
    constexpr float PI2 = PI / 2.f;

    constexpr float CELL_SIZE = 40.f;
    constexpr vec2f PANEL_SIZE = { 800.f, 160.f };

    namespace Depth {
        constexpr float FIELD = 0.9f;
        constexpr float SNAKE_TAIL = 0.8f;
        constexpr float HIDE_FIELD = 0.7f;
        constexpr float SNAKE_BODY = 0.6f;
        constexpr float APPLE = 0.5f;
        constexpr float SNAKE_HEAD = 0.4f;
        constexpr float EYE_PLACE = 0.3f;
        constexpr float EYE = 0.2f;
        constexpr float HIDE_HEAD = 0.15f;
        constexpr float PANEL = 0.1f;
        constexpr float PANEL_ICONS = 0.0f;
    }
    
    vec2f getThroughPos(vec2f position, const vec2i direction, const vec2f fieldSize) {
        if (direction.x != 0)
            position.x += fieldSize.x * (direction.x > 0 ? -1.f : 1.f);
        else
            position.y += fieldSize.y * (direction.y > 0 ? -1.f : 1.f);

        return position;
    }

    float getRotateAngle(const vec2i v1, const vec2i v2) {
        vec2i diff(v1 - v2);
        if (diff.x != 0)
            return diff.x == -1 || diff.x > 1 ? PI2 : -PI2;
        else
            return diff.y == -1 || diff.y > 1 ? PI : 0.f;
    }

    vec2f rotatePoint(const vec2f point, const float rotateAngle) {
        return {
            point.x * cosf(rotateAngle) - point.y * sinf(rotateAngle),
            point.x * sinf(rotateAngle) + point.y * cosf(rotateAngle)
        };
    }
}

Drawer::Drawer()
    : _zenMode(DrawConfig::zenMode)
    , _updateStaticBuffer(true)
    , _isPrevStatusMenu(true)
    , _isPravednovHead(false)
    , _contentScale(DrawConfig::contentScale)
    , _themeColor(DrawConfig::themeColor)
    , _auxThemeColor({ _themeColor.x - 0.05f, _themeColor.y - 0.05f, _themeColor.z - 0.05f, _themeColor.w })
    , _snakeColor(DrawConfig::snakeColor) {}

void Drawer::updateSnakeColor(const vec4f color) { _snakeColor = color; }

void Drawer::updateThemeColor(const vec4f color) {
    _themeColor = color;
    _auxThemeColor = { color.x - 0.05f, color.y - 0.05f, color.z - 0.05f, color.w };
    _rendererHandler.setClearColor({ color.x - 0.1f, color.y - 0.1f, color.z - 0.1f, color.w });
}

void Drawer::updateViewSize(const vec2f size) {
    _viewSize = size;
    _updateStaticBuffer = true;
}

void Drawer::updateZenMode(const bool zenMode) { _zenMode = zenMode; }

void Drawer::updateContentScale(const float scaleCoeff) { _contentScale = scaleCoeff; }

void Drawer::changePravednovStatus() {
    _isPravednovHead = !_isPravednovHead;
    _snakeColor = _isPravednovHead ? vec4f{ 0.74f, 0.58f, 0.62f, 1.f }
                                   : DrawConfig::snakeColor;
}

void Drawer::draw(const Game& game, const Clock& clock) {
    const bool isMenuStatus = game.status() == GameStatus::MENU;
    const bool isMenuStatusChanged = !_isPrevStatusMenu == isMenuStatus;
    _updateStaticBuffer = _updateStaticBuffer || isMenuStatusChanged;

    _origin = {0.f, 0.f};
    // render static object which here and there are
    if (_updateStaticBuffer) {
        if (!_zenMode) {
            _lbPanelPos = _addPanel();
        }
        if (isMenuStatus) {
            _updateStaticBuffer = false;
            _rendererHandler.refreshStaticBuffer();
            _rendererHandler.refreshDynamicBuffer();
        }
    }
    _origin = {0.f, 0.f};


    if (isMenuStatus) {
        _isPrevStatusMenu = true;

        _addPlayButton(clock.getAppleBreathingCoeff());
        _rendererHandler.refreshStreamBuffer();
    }
    else {
        _isPrevStatusMenu = false;

        if (_updateStaticBuffer) {
            _addField(game.field().size());
            _addInvisibleLines();
            _rendererHandler.refreshStaticBuffer();
            _updateStaticBuffer = false;
        }
        _origin = _lbFieldPos - vec2f{ CELL_SIZE * 0.5f };

        static bool straightLook = false;

        if (clock.getSnakeMovingCoeff() > 0.5f) {
            straightLook = false;
            _addDynamicSnake(game.snake());
            _rendererHandler.refreshDynamicBuffer();
        }

        const bool eatingApple = game.snake().getHeadPos() == game.apple().getPosition();

        const vec2i applePos = game.apple().getPosition().has_value()
            ? game.apple().getPosition().value()
            : game.snake().getHeadPos();

        _addStreamSnake(game.snake(), clock.getSnakeMovingCoeff(), eatingApple,
                       applePos, straightLook, clock.isBlinkTime());
        if (!(eatingApple && clock.getSnakeMovingCoeff() > 0.9f) && game.apple().getPosition().has_value())
            _addApple(game.apple().getPosition().value(), clock.getAppleBreathingCoeff());

        if (!_zenMode) {
            _origin = _lbPanelPos;
            const char* mode =
                game.mode() == GameMode::DEFAULT
                    ? "DEFAULT"
                    : (game.mode() == GameMode::THROUGH_WALLS
                        ? "THROUGH"
                        : "BOUNDLESS");
            _addPanelIcons(clock.getPrevFrames(), game.getScore(), game.getBestScore(),
                          game.field().size(), mode);
        }

        _rendererHandler.refreshStreamBuffer();
    }

    _rendererHandler.draw();
}

vec2f Drawer::_addPanel() {
    _data.size = PANEL_SIZE;
    _data.pos = { _viewSize.x * 0.5f, _viewSize.y - _data.size.y * 0.5f - 20.f };
    _data.depth = Depth::PANEL;
    _data.color = _auxThemeColor;
    _data.texType = TexType::SNAKE_BODY;
    _data.texCoord = { 0.45f, 0.45f, 0.5f, 0.5f };
    _data.rotateAngle = 0.f;

    _add(_data);

    return _data.pos - _data.size * 0.5f;
}

void Drawer::_addPanelIcons(const int fps, const int score, const int bestScore,
                           const vec2i fieldSize, const char* gameMode) {
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.depth = Depth::PANEL_ICONS;
    _data.rotateAngle = 0.f;

    std::string toPrint;
    const vec2f panelCenter = PANEL_SIZE * 0.5f;

    // ########## top line ##########
    // field size
    _data.size = CELL_SIZE;
    _data.texType = TexType::FIELD;
    _data.color = { 1.f, 1.f, 1.f, 1.f };
    _data.pos = { panelCenter.x - PANEL_SIZE.x * 0.5f + 40.f, panelCenter.y + 40.f };
    _add(_data);

    _data.pos.x += 40.f;
    toPrint += std::to_string(fieldSize.x);
    toPrint += ':';
    toPrint += std::to_string(fieldSize.y);
    _printText(toPrint);

    // best score
        _data.size = CELL_SIZE * 1.2;
    _data.pos = { panelCenter.x - 40.f, panelCenter.y + 40.f };
    _data.texType = TexType::STAR;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.color = { 1.f, 0.8f, 0.f, 1.f };
    _add(_data);

    _data.pos.x += 40.f;
    _data.color = { 1.f, 1.f, 1.f, 1.f };
    _printText(std::to_string(bestScore));

    // menu button
    _data.pos = { panelCenter.x + PANEL_SIZE.x * 0.5f - 80.f, panelCenter.y + 40.f };
    _printText("MENU");

    // ########## bottom line ##########
    // game mode
    _data.pos = { panelCenter.x - PANEL_SIZE.x * 0.5f + 28.f, panelCenter.y - 40.f };
    toPrint = "MODE: ";
    toPrint += gameMode;
    _printText(toPrint);

    // score
    _data.size = CELL_SIZE;
    _data.pos = { panelCenter.x - 40.f, panelCenter.y - 40.f };
    _data.color = { 1.f, 1.f, 1.f, 1.f };
    _data.texType = TexType::APPLE;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _add(_data);

    _data.pos.x += 40.f;
    _data.color = { 1.f, 1.f, 1.f, 1.f };
    _printText(std::to_string(score));

    // fps
    _data.pos = { panelCenter.x + PANEL_SIZE.x * 0.5f - 160.f, panelCenter.y - 40.f };
    toPrint = "FPS: ";
    toPrint += std::to_string(fps);
    _printText(toPrint);
}

void Drawer::_addField(const vec2i cellsNum) {
    _data.size = 80.f;
    _data.depth = Depth::FIELD;
    _data.color = _themeColor;
    _data.texType = TexType::FIELD;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.rotateAngle = 0.f;

    _fieldSize = static_cast<vec2f>(cellsNum) * CELL_SIZE;
    _lbFieldPos = {
        (_viewSize.x - _fieldSize.x) * 0.5f + CELL_SIZE,
        (_viewSize.y - _fieldSize.y - (!_zenMode ? PANEL_SIZE.y + 20.f : 0.f)) * 0.5f + CELL_SIZE
    };
    vec2i fieldTexsNum = {
        cellsNum.x / 2 + cellsNum.x % 2,
        cellsNum.y / 2 + cellsNum.y % 2
    };

    for (int x = 0; x < fieldTexsNum.x; ++x) {
        for (int y = 0; y < fieldTexsNum.y; ++y) {
            _data.pos = _lbFieldPos + static_cast<vec2f>(vec2i{x, y}) * _data.size;
            _add(_data);
        }
    }
}

void Drawer::_addInvisibleLines() {
    _data.depth = Depth::HIDE_FIELD;
    _data.color = _themeColor - vec4f{ 0.1f, 0.1f, 0.1f, 0.f };
    _data.texType = TexType::SNAKE_BODY;
    _data.texCoord = { 0.45f, 0.45f, 0.5f, 0.5f };
    _data.rotateAngle = 0.f;

    const vec2f fieldCenter = _lbFieldPos + _fieldSize * 0.5f;

    // left & right lines
    _data.size = { CELL_SIZE, _fieldSize.y };
    _data.pos = {
        fieldCenter.x - CELL_SIZE - (_fieldSize.x + CELL_SIZE) * 0.5f,
        fieldCenter.y - CELL_SIZE
    };
    _add(_data);
    _data.pos.x += _fieldSize.x + CELL_SIZE;
    _add(_data);

    // top & bottom lines
    _data.size = { _fieldSize.x + CELL_SIZE * 2.f, CELL_SIZE };
    _data.pos = {
        fieldCenter.x - CELL_SIZE,
        fieldCenter.y + (_fieldSize.y - CELL_SIZE) * 0.5f
    };
    _add(_data);
    _data.pos.y -= _fieldSize.y + CELL_SIZE;
    _add(_data);
}

void Drawer::_addPlayButton(const float breathCoeff) {
    _data.size = 256.f * breathCoeff;
    _data.pos = {
        _viewSize.x * 0.5f,
        (_viewSize.y - (!_zenMode ? PANEL_SIZE.y + 20.f : 0.f)) * 0.5f
    };
    _data.depth = Depth::FIELD;
    _data.color = (_themeColor.x < 0.7f && _themeColor.y < 0.7f && _themeColor.z < 0.7f)
                ? vec4f{ 1.f, 1.f, 1.f, 1.f }
                : vec4f{ 0.f, 0.f, 0.f, 1.f };
    _data.texType = TexType::PLAY;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.rotateAngle = 0.f;

    _add(_data);
}

void Drawer::_addApple(const vec2f position, const float breathCoeff) {
    _data.size = CELL_SIZE * breathCoeff;
    _data.pos = position * CELL_SIZE;
    _data.depth = Depth::APPLE;
    _data.color = { 1.f, 1.f, 1.f, 1.f };
    _data.texType = TexType::APPLE;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.rotateAngle = 0.f;

    _add(_data);
}

void Drawer::_addDynamicSnake(const Snake& snake) {
    _data.size = CELL_SIZE;
    _data.depth = Depth::SNAKE_BODY;
    _data.color = _snakeColor;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };

    const std::vector<vec2i> snakeBody = snake.getBody();
    const int snakeSize = snake.getLength();

    for (size_t i = snakeSize - 2; i > 0; --i) {
        vec2i prev = snakeBody[i - 1];
        vec2i cur = snakeBody[i];
        vec2i next = snakeBody[i + 1];

        _data.pos = static_cast<vec2f>(cur) * CELL_SIZE;
        _data.texType = TexType::SNAKE_BODY;
        _data.rotateAngle = getRotateAngle(next, cur);

        vec2i diff = next - prev;
        if (diff.x != 0 && diff.y) {
            vec2i from = prev - cur;
            vec2i to = next - cur;

            if (from.x < -1 || from.x > 1)
                from.x = from.x < -1 ? 1 : -1;
            else if (from.y < -1 || from.y > 1)
                from.y = from.y < -1 ? 1 : -1;

            if (to.x < -1 || to.x > 1)
                to.x = to.x < -1 ? 1 : -1;
            else if (to.y < -1 || to.y > 1)
                to.y = to.y < -1 ? 1 : -1;

            bool counterclockwise = (from == vec2i{ 1, 0 } && to == vec2i{ 0,-1 })
                                 || (from == vec2i{ 0, 1 } && to == vec2i{ 1, 0 })
                                 || (from == vec2i{-1, 0 } && to == vec2i{ 0, 1 })
                                 || (from == vec2i{ 0,-1 } && to == vec2i{-1, 0 });
            if (counterclockwise)
                _data.rotateAngle += PI2;

            _data.texType = TexType::SNAKE_CORNER;
        }

        _add(_data);
    }
}

void Drawer::_addStreamSnake(const Snake& snake, const float snakeMovingCoeff, const bool eatingApple,
                            const vec2i applePos, bool& straightLook, const bool isBlinkTime) {
    _data.size = CELL_SIZE;
    _data.color = _snakeColor;
    _data.texType = TexType::SNAKE_TAIL;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };

    const std::vector<vec2i> snakeBody = snake.getBody();
    const int snakeSize = snakeBody.size();
    const vec2i direction = snake.getDirection();

    const vec2i head = snakeBody[0];
    const bool headThroughBorder = std::abs(head.x - snakeBody[1].x) > 1
                                || std::abs(head.y - snakeBody[1].y) > 1;

    const vec2i tail = snakeBody.back();
    const vec2i prevTail = snake.getPrevTail();
    const bool tailThroughBorder = std::abs(tail.x - prevTail.x) > 1
                                || std::abs(tail.y - prevTail.y) > 1;

    static bool newTailDirection = false;

    bool drawHead = true;
    bool drawTail = true;

    if (snakeMovingCoeff <= 0.5f) {
        _data.size = CELL_SIZE;

        vec2i prev = snakeSize > 2 ? snakeBody[2] : prevTail;
        vec2i cur = snakeBody[1];
        vec2i next = head;

        vec2i diff = next - prev;
        // turning head
        if (diff.x != 0 && diff.y != 0) {
            // waiting head
            _data.pos = static_cast<vec2f>(cur) * CELL_SIZE;
            _data.depth = Depth::SNAKE_HEAD;
            _data.texCoord = {0.f, -0.1f, 1.f, 0.9f };
            _data.rotateAngle = getRotateAngle(cur, prev);
            _data.texType = TexType::SNAKE_TAIL;
            _add(_data);

            // dynamic head
            _data.pos = (static_cast<vec2f>(cur) + static_cast<vec2f>(direction) * 0.5f) * CELL_SIZE;
            _data.depth = Depth::SNAKE_HEAD - 0.01f;
            _data.rotateAngle = getRotateAngle(next + direction, next);
            _data.texCoord = { 0.f, -0.5f + snakeMovingCoeff, 1.f, 0.5f + snakeMovingCoeff};

            if (headThroughBorder) {
                _data.pos = static_cast<vec2f>(head) * CELL_SIZE;
                _data.texCoord = { 0.f, -1.f + snakeMovingCoeff, 1.f, snakeMovingCoeff };
                _add(_data);

                _data.pos = static_cast<vec2f>(snakeBody[1]) * CELL_SIZE + static_cast<vec2f>(direction) * CELL_SIZE * 0.25f;
                _data.texCoord = { 0.f, 0.0f + snakeMovingCoeff, 1.f, 0.5f + snakeMovingCoeff};
                _data.size = { CELL_SIZE, CELL_SIZE * 0.5f };
            }
            _add(_data);

            drawHead = false;
        }
        // aux straight head
        else if (snakeSize > 2) {
            _data.rotateAngle = getRotateAngle(next, cur);
            _data.pos = static_cast<vec2f>(cur) * CELL_SIZE
                    - static_cast<vec2f>(direction) * CELL_SIZE * 0.25f;
            _data.depth = Depth::SNAKE_HEAD;
            _data.size = { CELL_SIZE, CELL_SIZE * 0.5f };
            _data.texCoord = { 0.f, 0.f, 1.f, 0.5f };
            _data.texType = TexType::SNAKE_BODY;
            _add(_data);
        }

        if (newTailDirection) {
            float auxSnakeMovingCoeff = snakeMovingCoeff;

            if (!eatingApple) {
                prev = snake.getPrevPrevTail();
                cur = prevTail;
                next = tail;
            }
            else {
                auxSnakeMovingCoeff = 0.f;
                prev = prevTail;
                cur = tail;
                next = *(snakeBody.rbegin() + 1);
            }

            // waiting tail
            _data.size = CELL_SIZE;
            _data.pos = static_cast<vec2f>(cur) * CELL_SIZE;
            _data.depth = Depth::SNAKE_TAIL;
            _data.texCoord = {0.f, -0.1f, 1.f, 0.9f };
            _data.rotateAngle = getRotateAngle(cur, next);
            _data.texType = TexType::SNAKE_TAIL;
            _add(_data);

            // aux static body to fill space between head and tail in 2sized snake
            _data.pos = static_cast<vec2f>(next) * CELL_SIZE;
            _data.depth = Depth::SNAKE_TAIL;
            _data.texCoord = { 0.f, 0.5f, 1.f, 1.5f };
            _data.texType = TexType::SNAKE_BODY;
            _add(_data);

            // new step after turned tail -- little dynamic tail
            diff = cur - prev;
            if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
            else  			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;
            _data.pos = (static_cast<vec2f>(cur) - static_cast<vec2f>(diff) * 0.5f) * CELL_SIZE;
            _data.depth = Depth::SNAKE_TAIL - 0.01f;
            _data.texCoord = {0.f, -0.5f - auxSnakeMovingCoeff, 1.f, 0.5f - auxSnakeMovingCoeff};
            _data.rotateAngle = getRotateAngle(prev, cur);
            _data.texType = TexType::SNAKE_TAIL;
            _add(_data);

            drawTail = false;
            if (auxSnakeMovingCoeff > 0.1f)
                newTailDirection = false;
        }
    }
    else {
        _data.size = CELL_SIZE;

        vec2i prev = prevTail;
        vec2i cur = tail;
        vec2i next = *(snakeBody.rbegin() + 1);

        vec2i diff = next - prev;
        // turning tail
        if ((diff.x != 0 && diff.y != 0)) {
            // waiting tail
            _data.size = CELL_SIZE;
            _data.pos = static_cast<vec2f>(cur) * CELL_SIZE;
            _data.depth = Depth::SNAKE_TAIL;
            _data.texCoord = {0.f, -0.1f, 1.f, 0.9f };
            _data.rotateAngle = getRotateAngle(cur, next);
            _data.texType = TexType::SNAKE_TAIL;
            _add(_data);

            // dynamic tail
            diff = cur - prev;
            if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
            else  			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;
            _data.pos = (static_cast<vec2f>(cur) - static_cast<vec2f>(diff) * 0.5f) * CELL_SIZE;
            _data.depth = Depth::SNAKE_TAIL - 0.01f;
            _data.rotateAngle = getRotateAngle(prev, cur);
            _data.texCoord = eatingApple ? vec4f{0.f, -0.5f, 1.f, 0.5f }
                                         : vec4f{0.f, 0.5f - snakeMovingCoeff, 1.f, 1.5f - snakeMovingCoeff};

            if (tailThroughBorder && !eatingApple) {
                _data.texType = TexType::SNAKE_TAIL;
                _data.pos = static_cast<vec2f>(prevTail) * CELL_SIZE;
                _data.depth = Depth::SNAKE_TAIL;
                _data.texCoord = { 0.f, -snakeMovingCoeff , 1.f, 1.f - snakeMovingCoeff};
                _add(_data);

                _data.pos = static_cast<vec2f>(tail) * CELL_SIZE - static_cast<vec2f>(diff) * CELL_SIZE * 0.25f;
                _data.depth = Depth::SNAKE_TAIL;
                _data.texCoord = { 0.f, 1.f - snakeMovingCoeff, 1.f, 1.5f - snakeMovingCoeff };
                _data.size = { CELL_SIZE, CELL_SIZE * 0.5f };
            }
            _add(_data);

            drawTail = false;
            newTailDirection = true;
        }
        // aux straight tail
        else if (snakeSize > 2) {
            vec2i diff = next - cur;
            if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
            else 			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;

            _data.pos = static_cast<vec2f>(cur) * CELL_SIZE
                    + static_cast<vec2f>(diff) * CELL_SIZE * 0.25f;
            _data.depth = Depth::SNAKE_TAIL;
            _data.size = { CELL_SIZE, CELL_SIZE * 0.5f };
            _data.texCoord = { 0.f, 0.f, 1.f, 0.5f };
            _data.rotateAngle = getRotateAngle(cur, next);
            _data.texType = TexType::SNAKE_BODY;
            _add(_data);
        }
    }

    // straight head
    if (drawHead) {
        _data.size = CELL_SIZE;
        _data.depth = Depth::SNAKE_HEAD;
        _data.texType = TexType::SNAKE_TAIL;
        _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
        _data.rotateAngle = getRotateAngle(head + direction, head);

        vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;
        _data.pos = (static_cast<vec2f>(snakeBody[1]) + alpha) * CELL_SIZE;

        _add(_data);
        if (headThroughBorder) {
            _data.pos = getThroughPos(_data.pos, direction, _fieldSize);
            _add(_data);
        }
    }

    if (headThroughBorder) {
        vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;
        _data.pos = (static_cast<vec2f>(snakeBody[1]) + alpha) * CELL_SIZE;
        
        vec2i invisiblePrimitivesPosition = (snakeBody[1] + direction) * static_cast<int>(CELL_SIZE);
        _data.pos = invisiblePrimitivesPosition;
        _data.size = { CELL_SIZE, CELL_SIZE * 1.2f };
        _data.depth = Depth::HIDE_HEAD;
        _data.color = _themeColor - vec4f{ 0.1f, 0.1f, 0.1f, 0.f };
        _data.rotateAngle = direction.x == 0 ? PI2 : 0.f;
        _data.texType = TexType::SNAKE_BODY;
        _data.texCoord = { 0.45f, 0.45f, 0.5f, 0.5f };
        _add(_data);
        
        _data.pos = getThroughPos(_data.pos, direction, _fieldSize) - static_cast<vec2f>(direction) * CELL_SIZE;
        _add(_data);
        _data.color = _snakeColor;
    }

    // straight tail
    if (drawTail) {
        _data.size = CELL_SIZE;
        _data.depth = Depth::SNAKE_TAIL;
        _data.texType = TexType::SNAKE_TAIL;
        _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
        _data.rotateAngle = getRotateAngle(prevTail, tail);

        vec2i diff = tail - prevTail;
        if (diff.x != 0) diff.x = (diff.x == 1 || diff.x < -1) ? 1 : -1;
        else 			 diff.y = (diff.y == 1 || diff.y < -1) ? 1 : -1;

        vec2f alpha = static_cast<vec2f>(diff) * snakeMovingCoeff;

        _data.pos = (static_cast<vec2f>(prevTail) + alpha) * CELL_SIZE;

        if (eatingApple)
            _data.pos = static_cast<vec2f>(tail) * CELL_SIZE;

        _add(_data);
        if (tailThroughBorder && !eatingApple) {
            _data.pos = getThroughPos(_data.pos, diff, _fieldSize);
            _add(_data);
        }
    }

    // ---------- eyes ----------
    const vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;
    const vec2f eyePos = (static_cast<vec2f>(snakeBody[1]) + alpha) * CELL_SIZE;

    if (!_isPravednovHead) {
        const vec2f eyePlaceSize = CELL_SIZE * 0.45f;
        vec2f eye1Pos = eyePos;
        vec2f eye2Pos = eyePos;

        float coeff = (direction.x > 0 || direction.y > 0) ? -1.f : 1.f;
        if (direction.y != 0) {
            eye2Pos.x = eye1Pos.x += CELL_SIZE * 0.35f * coeff;
            eye2Pos.y = eye1Pos.y += CELL_SIZE * 0.3f * coeff;
            eye2Pos.x -= -coeff * (-CELL_SIZE + eyePlaceSize.x - 6.f);
        } else {
            eye2Pos.x = eye1Pos.x += CELL_SIZE * 0.3f * coeff;
            eye2Pos.y = eye1Pos.y += CELL_SIZE * 0.35f * coeff;
            eye2Pos.y -= -coeff * (-CELL_SIZE + eyePlaceSize.y - 6.f);
        }

        _data.size = eyePlaceSize;
        _data.pos = eye1Pos;
        _data.depth = Depth::EYE_PLACE;
        _data.color = { _snakeColor.x - 0.03f, _snakeColor.y - 0.03f, _snakeColor.z - 0.03f, _snakeColor.w };
        _data.texCoord = {0.f, 0.f, 1.f, 1.f};
        _data.texType = TexType::EYE_ORBIT;
        _data.rotateAngle = getRotateAngle(head + direction, head);

        // eye places
        _add(_data);
        _data.pos = eye2Pos;
        _add(_data);
        if (headThroughBorder) {
            _data.pos = getThroughPos(eye1Pos, direction, _fieldSize);
            _add(_data);
            _data.pos = getThroughPos(eye2Pos, direction, _fieldSize);
            _add(_data);
        }
        
        if (!(headThroughBorder && snakeMovingCoeff > 0.6f) && !isBlinkTime) {
            _data.depth = Depth::EYE;
            _data.color = { 1.f, 1.f, 1.f, 1.f };
            _data.texType = TexType::EYE;

            if (!eatingApple && !straightLook) {
                vec2f diff = eyePos - (static_cast<vec2f>(applePos) * CELL_SIZE);
                _data.rotateAngle = std::atan2(diff.x, -diff.y);
            }
            else {
                straightLook = true;
            }

            constexpr vec2f eyeSize = CELL_SIZE * 0.3f;
            _data.size = eyeSize;

            _data.pos = eye1Pos;
            _add(_data);
            _data.pos = eye2Pos;
            _add(_data);
        }
    }
    // ---------- pravednov head ----------
    else {
        _data.size = 50.f;
        _data.depth = Depth::EYE_PLACE;
        _data.color = {1.f, 1.f, 1.f, 1.f};
        _data.texType = TexType::PRAVEDNOV;
        _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
        _data.rotateAngle = getRotateAngle(head + direction, head) - PI2;
        if (direction == Direction::RIGHT)
            _data.texCoord = { 0.f, 1.f, 1.f, 0.f };

        _data.pos = eyePos;
        _add(_data);
        if (headThroughBorder) {
            _data.pos = getThroughPos(eyePos, direction, _fieldSize);
            _add(_data);
        }
    }
}

void Drawer::_printText(const std::string& str) {
    _data.texType = TexType::ALPHABET;
    _data.size = { 16.f, 25.f };
    _data.pos.y -= 5.f;

    int x, y;
    for (const char symbol : str) {
        if (symbol != ' ') {
            int shiftNum = symbol - 'A';
            if (shiftNum < 0) {
                constexpr int diffTo0 = 17;
                constexpr int lettersNum = 26;

                switch(symbol) {
                    case '<': shiftNum -= 1; break;
                    case '>': shiftNum -= 2; break;
                    case '?': shiftNum -= 2; break;
                    default: break;
                }

                shiftNum += diffTo0 + lettersNum;
            }
            y = shiftNum / 8;
            x = shiftNum - y * 8;

            constexpr vec2f texSymbolSize = { 0.125f, 0.1953f };
            const vec2f texSymbolPos = texSymbolSize * static_cast<vec2f>(vec2i{ x + 1, y + 1 });
            _data.texCoord = { texSymbolPos - texSymbolSize, texSymbolPos };
            _add(_data);
        }
        _data.pos.x += _data.size.x;
    }
}

void Drawer::_add(const RectangleData& data) {
    vec2f size = data.size * 0.5f * _contentScale;
    vec2f pos = data.pos * _contentScale;

    vec2f normViewSize = _viewSize * 0.5f * _contentScale;
    vec2f normOrigin = _origin * _contentScale - normViewSize;

    vec3f lb{ (rotatePoint({-size.x,-size.y }, data.rotateAngle) + pos + normOrigin) / normViewSize, data.depth };
    vec3f rb{ (rotatePoint({ size.x,-size.y }, data.rotateAngle) + pos + normOrigin) / normViewSize, data.depth };
    vec3f rt{ (rotatePoint({ size.x, size.y }, data.rotateAngle) + pos + normOrigin) / normViewSize, data.depth };
    vec3f lt{ (rotatePoint({-size.x, size.y }, data.rotateAngle) + pos + normOrigin) / normViewSize, data.depth };

    int layer = static_cast<int>(data.texType);
    _rendererHandler.addObject({
        { lb, {data.texCoord.x, data.texCoord.w}, data.color, layer },
        { rb, {data.texCoord.z, data.texCoord.w}, data.color, layer },
        { rt, {data.texCoord.z, data.texCoord.y}, data.color, layer },
        { lt, {data.texCoord.x, data.texCoord.y}, data.color, layer }
    });
}
