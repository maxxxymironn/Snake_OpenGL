#include "drawer.hpp"
#include "src/game/enums.hpp"
#include "src/render/texture_enum.hpp"

namespace {
    constexpr float PI = 3.14159265359f;
    constexpr float PI2 = PI / 2.f;

    constexpr float cellSize = 40.f;
    constexpr vec2f panelSize = { 800.f, 160.f };
    
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
}

void Drawer::setSnakeColor(const vec4f color) { _snakeColor = color; }

void Drawer::setThemeColor(const vec4f color) {
    _themeColor = color;
    _auxThemeColor = color - 0.05f;
    _auxThemeColor.w += 0.05f;
}

void Drawer::setViewSize(const vec2f size) { _viewSize = size; }

void Drawer::draw(const Game& game, const Clock& clock) {
    const bool isMenuStatus = game.status() == GameStatus::MENU;
    const bool isMenuStatusChanged = _isPrevStatusMenu == !isMenuStatus;

    _rendererHandler->setOrigin({0.f, 0.f});
    // render static object which here and there are
    if (_rendererHandler->needRefreshStaticBuffer() || isMenuStatusChanged) {
        if (!_rendererHandler->isZenMode())
            addPanel();
        if (isMenuStatus) {
            _rendererHandler->refreshStaticBuffer();
            _rendererHandler->refreshDynamicBuffer();
        }
    }

    if (isMenuStatus) {
        _isPrevStatusMenu = true;

        addPlayButton(clock.getAppleBreathingCoeff());
        _rendererHandler->refreshStreamBuffer();
    }
    else {
        _isPrevStatusMenu = false;

        if (_rendererHandler->needRefreshStaticBuffer() || isMenuStatusChanged) {
            addField(game.field().getFieldSize());
            addInvisibleLines();
            _rendererHandler->refreshStaticBuffer();
        }
        _rendererHandler->setOrigin(_lbFieldPos - vec2f{ cellSize * 0.5f });

        static bool straightLook = false;

        if (clock.getSnakeMovingCoeff() > 0.5f) {
            straightLook = false;
            addDynamicSnake(game.snake());
            _rendererHandler->refreshDynamicBuffer();
        }

        const bool eatingApple = game.snake().getHeadPos() == game.apple().getPosition();

        addStreamSnake(game.snake(), clock.getSnakeMovingCoeff(), eatingApple,
                       game.getPravednovBool(), game.apple().getPosition(), straightLook, clock.isBlinkTime());
        if (!(eatingApple && clock.getSnakeMovingCoeff() > 0.9f))
            addApple(game.apple().getPosition(), clock.getAppleBreathingCoeff());
        _rendererHandler->refreshStreamBuffer();
    }

    _rendererHandler->draw();
}

void Drawer::addPanel() {
    _data.size = panelSize;
    _data.pos = { _viewSize.x * 0.5f, _viewSize.y - _data.size.y * 0.5f - 20.f };
    _data.depth = 0.1f;
    _data.color = _auxThemeColor;
    _data.texType = TexType::SNAKE_BODY;
    _data.texCoord = { 0.45f, 0.45f, 0.5f, 0.5f };
    _data.rotateAngle = 0.f;

    _rendererHandler->addObject(_data);
}

void Drawer::addField(const vec2i cellsNum) {
    _data.size = 80.f;
    _data.depth = 0.9f;
    _data.color = _themeColor;
    _data.texType = TexType::FIELD;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.rotateAngle = 0.f;

    _fieldSize = static_cast<vec2f>(cellsNum) * cellSize;
    _lbFieldPos = {
        (_viewSize.x - _fieldSize.x) * 0.5f + cellSize,
        (_viewSize.y - _fieldSize.y - (!_rendererHandler->isZenMode() ? panelSize.y + 20.f : 0.f)) * 0.5f + cellSize
    };
    vec2i fieldTexsNum = {
        cellsNum.x / 2 + cellsNum.x % 2,
        cellsNum.y / 2 + cellsNum.y % 2
    };

    for (int x = 0; x < fieldTexsNum.x; ++x) {
        for (int y = 0; y < fieldTexsNum.y; ++y) {
            _data.pos = _lbFieldPos + static_cast<vec2f>(vec2i{x, y}) * _data.size;
            _rendererHandler->addObject(_data);
        }
    }
}

void Drawer::addInvisibleLines() {
    _data.depth = 0.5f;
    _data.color = _themeColor - vec4f{ 0.1f, 0.1f, 0.1f, 0.f };
    _data.texType = TexType::SNAKE_BODY;
    _data.texCoord = { 0.45f, 0.45f, 0.5f, 0.5f };
    _data.rotateAngle = 0.f;

    const vec2f fieldCenter = _lbFieldPos + _fieldSize * 0.5f;

    // left & right lines
    _data.size = { cellSize, _fieldSize.y };
    _data.pos = {
        fieldCenter.x - cellSize - (_fieldSize.x + cellSize) * 0.5f,
        fieldCenter.y - cellSize
    };
    _rendererHandler->addObject(_data);
    _data.pos.x += _fieldSize.x + cellSize;
    _rendererHandler->addObject(_data);

    // top & bottom lines
    _data.size = { _fieldSize.x + cellSize * 2.f, cellSize };
    _data.pos = {
        fieldCenter.x - cellSize,
        fieldCenter.y + (_fieldSize.y - cellSize) * 0.5f
    };
    _rendererHandler->addObject(_data);
    _data.pos.y -= _fieldSize.y + cellSize;
    _rendererHandler->addObject(_data);
}

void Drawer::addPlayButton(const float breathCoeff) {
    _data.size = 256.f * breathCoeff;
    _data.pos = {
        _viewSize.x * 0.5f,
        (_viewSize.y - (!_rendererHandler->isZenMode() ? panelSize.y + 20.f : 0.f)) * 0.5f
    };
    _data.depth = 0.9f;
    _data.color = (_themeColor.x < 0.7f && _themeColor.y < 0.7f && _themeColor.z < 0.7f)
                ? vec4f{ 1.f, 1.f, 1.f, 1.f }
                : vec4f{ 0.f, 0.f, 0.f, 1.f };
    _data.texType = TexType::PLAY;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.rotateAngle = 0.f;

    _rendererHandler->addObject(_data);
}

void Drawer::addApple(const vec2f position, const float breathCoeff) {
    _data.size = cellSize * breathCoeff;
    _data.pos = position * cellSize;
    _data.depth = 0.4f;
    _data.color = { 1.f, 1.f, 1.f, 1.f };
    _data.texType = TexType::APPLE;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.rotateAngle = 0.f;

    _rendererHandler->addObject(_data);
}

void Drawer::addDynamicSnake(const Snake& snake) {
    _data.size = cellSize;
    _data.depth = 0.8f;
    _data.color = _snakeColor;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };

    const std::vector<vec2i> snakeBody = snake.getBody();
    const int snakeSize = snake.getLength();

    for (size_t i = snakeSize - 2; i > 0; --i) {
        vec2i prev = snakeBody[i - 1];
        vec2i cur = snakeBody[i];
        vec2i next = snakeBody[i + 1];

        _data.pos = static_cast<vec2f>(cur) * cellSize;
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

        _rendererHandler->addObject(_data);
    }
}

void Drawer::addStreamSnake(const Snake& snake, const float snakeMovingCoeff, const bool eatingApple,
                            const bool pravednovFace, const vec2i applePos, bool& straightLook, const bool isBlinkTime) {
    _data.size = cellSize;
    _data.depth = 0.6f;
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
        _data.size = cellSize;

        vec2i prev = snakeSize > 2 ? snakeBody[2] : prevTail;
        vec2i cur = snakeBody[1];
        vec2i next = head;

        vec2i diff = next - prev;
        // turning head
        if (diff.x != 0 && diff.y != 0) {
            // waiting head
            _data.pos = static_cast<vec2f>(cur) * cellSize;
            _data.texCoord = {0.f, -0.1f, 1.f, 0.9f };
            _data.rotateAngle = getRotateAngle(cur, prev);
            _data.texType = TexType::SNAKE_TAIL;
            _rendererHandler->addObject(_data);

            // dynamic head
            _data.pos = (static_cast<vec2f>(cur) + static_cast<vec2f>(direction) * 0.5f) * cellSize;
            _data.rotateAngle = getRotateAngle(next + direction, next);
            _data.texCoord = { 0.f, -0.5f + snakeMovingCoeff, 1.f, 0.5f + snakeMovingCoeff};

            if (headThroughBorder) {
                _data.pos = static_cast<vec2f>(head) * cellSize;
                _data.texCoord = { 0.f, -1.f + snakeMovingCoeff, 1.f, snakeMovingCoeff };
                _rendererHandler->addObject(_data);

                _data.pos = static_cast<vec2f>(snakeBody[1]) * cellSize + static_cast<vec2f>(direction) * cellSize * 0.25f;
                _data.texCoord = { 0.f, 0.0f + snakeMovingCoeff, 1.f, 0.5f + snakeMovingCoeff};
                _data.size = { cellSize, cellSize * 0.5f };
            }
            _rendererHandler->addObject(_data);

            drawHead = false;
        }
        // aux straight head
        else if (snakeSize > 2) {
            _data.rotateAngle = getRotateAngle(next, cur);
            _data.pos = static_cast<vec2f>(cur) * cellSize
                    - static_cast<vec2f>(direction) * cellSize * 0.25f;
            _data.size = { cellSize, cellSize * 0.5f };
            _data.texCoord = { 0.f, 0.f, 1.f, 0.5f };
            _data.texType = TexType::SNAKE_BODY;
            _rendererHandler->addObject(_data);
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
            _data.size = cellSize;
            _data.pos = static_cast<vec2f>(cur) * cellSize;
            _data.texCoord = {0.f, -0.1f, 1.f, 0.9f };
            _data.rotateAngle = getRotateAngle(cur, next);
            _data.texType = TexType::SNAKE_TAIL;
            _rendererHandler->addObject(_data);

            // aux static body to fill space between head and tail in 2sized snake
            _data.pos = static_cast<vec2f>(next) * cellSize;
            _data.texCoord = { 0.f, 0.5f, 1.f, 1.5f };
            _data.texType = TexType::SNAKE_BODY;
            _rendererHandler->addObject(_data);

            // new step after turned tail -- little dynamic tail
            diff = cur - prev;
            if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
            else  			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;
            _data.pos = (static_cast<vec2f>(cur) - static_cast<vec2f>(diff) * 0.5f) * cellSize;
            _data.texCoord = {0.f, -0.5f - auxSnakeMovingCoeff, 1.f, 0.5f - auxSnakeMovingCoeff};
            _data.rotateAngle = getRotateAngle(prev, cur);
            _data.texType = TexType::SNAKE_TAIL;
            _rendererHandler->addObject(_data);

            drawTail = false;
            if (auxSnakeMovingCoeff > 0.1f)
                newTailDirection = false;
        }
    }
    else {
        _data.size = cellSize;

        vec2i prev = prevTail;
        vec2i cur = tail;
        vec2i next = *(snakeBody.rbegin() + 1);

        vec2i diff = next - prev;
        // turning tail
        if ((diff.x != 0 && diff.y != 0)) {
            // waiting tail
            _data.size = cellSize;
            _data.pos = static_cast<vec2f>(cur) * cellSize;
            _data.texCoord = {0.f, -0.1f, 1.f, 0.9f };
            _data.rotateAngle = getRotateAngle(cur, next);
            _data.texType = TexType::SNAKE_TAIL;
            _rendererHandler->addObject(_data);

            // dynamic tail
            diff = cur - prev;
            if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
            else  			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;
            _data.pos = (static_cast<vec2f>(cur) - static_cast<vec2f>(diff) * 0.5f) * cellSize;
            _data.rotateAngle = getRotateAngle(prev, cur);
            _data.texCoord = eatingApple ? vec4f{0.f, -0.5f, 1.f, 0.5f }
                                        : vec4f{0.f, 0.5f - snakeMovingCoeff, 1.f, 1.5f - snakeMovingCoeff};

            if (tailThroughBorder && !eatingApple) {
                _data.texType = TexType::SNAKE_TAIL;
                _data.pos = static_cast<vec2f>(prevTail) * cellSize;
                _data.texCoord = { 0.f, -snakeMovingCoeff , 1.f, 1.f - snakeMovingCoeff};
                _rendererHandler->addObject(_data);

                _data.pos = static_cast<vec2f>(tail) * cellSize - static_cast<vec2f>(diff) * cellSize * 0.25f;
                _data.texCoord = { 0.f, 1.f - snakeMovingCoeff, 1.f, 1.5f - snakeMovingCoeff };
                _data.size = { cellSize, cellSize * 0.5f };
            }
            _rendererHandler->addObject(_data);

            drawTail = false;
            newTailDirection = true;
        }
        // aux straight tail
        else if (snakeSize > 2) {
            vec2i diff = next - cur;
            if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
            else 			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;

            _data.pos = static_cast<vec2f>(cur) * cellSize
                    + static_cast<vec2f>(diff) * cellSize * 0.25f;
            _data.size = { cellSize, cellSize * 0.5f };
            _data.texCoord = { 0.f, 0.f, 1.f, 0.5f };
            _data.rotateAngle = getRotateAngle(cur, next);
            _data.texType = TexType::SNAKE_BODY;
            _rendererHandler->addObject(_data);
        }
    }

    // straight head
    if (drawHead) {
        _data.size = cellSize;
        _data.texType = TexType::SNAKE_TAIL;
        _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
        _data.rotateAngle = getRotateAngle(head + direction, head);

        vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;
        _data.pos = (static_cast<vec2f>(snakeBody[1]) + alpha) * cellSize;

        _rendererHandler->addObject(_data);
        if (headThroughBorder) {
            _data.pos = getThroughPos(_data.pos, direction, _fieldSize);
            _rendererHandler->addObject(_data);
        }
    }

    // straight tail
    if (drawTail) {
        _data.size = cellSize;
        _data.texType = TexType::SNAKE_TAIL;
        _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
        _data.rotateAngle = getRotateAngle(prevTail, tail);

        vec2i diff = tail - prevTail;
        if (diff.x != 0) diff.x = (diff.x == 1 || diff.x < -1) ? 1 : -1;
        else 			 diff.y = (diff.y == 1 || diff.y < -1) ? 1 : -1;

        vec2f alpha = static_cast<vec2f>(diff) * snakeMovingCoeff;

        _data.pos = (static_cast<vec2f>(prevTail) + alpha) * cellSize;

        if (eatingApple)
            _data.pos = static_cast<vec2f>(tail) * cellSize;

        _rendererHandler->addObject(_data);
        if (tailThroughBorder) {
            _data.pos = getThroughPos(_data.pos, diff, _fieldSize);
            _rendererHandler->addObject(_data);
        }
    }

    // ---------- eyes ----------
    const vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;
    const vec2f eyePos = (static_cast<vec2f>(snakeBody[1]) + alpha) * cellSize;

    if (!pravednovFace) {
        const vec2f eyePlaceSize = cellSize * 0.45f;
        vec2f eye1Pos = eyePos;
        vec2f eye2Pos = eyePos;

        float coeff = (direction.x > 0 || direction.y > 0) ? -1.f : 1.f;
        if (direction.y != 0) {
            eye2Pos.x = eye1Pos.x += cellSize * 0.35f * coeff;
            eye2Pos.y = eye1Pos.y += cellSize * 0.3f * coeff;
            eye2Pos.x -= -coeff * (-cellSize + eyePlaceSize.x - 6.f);
        } else {
            eye2Pos.x = eye1Pos.x += cellSize * 0.3f * coeff;
            eye2Pos.y = eye1Pos.y += cellSize * 0.35f * coeff;
            eye2Pos.y -= -coeff * (-cellSize + eyePlaceSize.y - 6.f);
        }

        _data.size = eyePlaceSize;
        _data.pos = eye1Pos;
        _data.color = { _snakeColor.x - 0.03f, _snakeColor.y - 0.03f, _snakeColor.z - 0.03f, _snakeColor.w };
        _data.texCoord = {0.f, 0.f, 1.f, 1.f};
        _data.texType = TexType::EYE_ORBIT;
        _data.rotateAngle = getRotateAngle(head + direction, head);

        // eye places
        if (!headThroughBorder) {
            _data.depth = 0.2f;
            _rendererHandler->addObject(_data);
            _data.pos = eye2Pos;
            _rendererHandler->addObject(_data);
        }
        else {
            _data.depth = 0.55f;

            _rendererHandler->addObject(_data);
            _data.pos = getThroughPos(_data.pos, direction, _fieldSize);
            _rendererHandler->addObject(_data);

            _data.pos = eye2Pos;
            _rendererHandler->addObject(_data);
            _data.pos = getThroughPos(_data.pos, direction, _fieldSize);
            _rendererHandler->addObject(_data);
        }

        if (!(headThroughBorder && snakeMovingCoeff > 0.6f) && !isBlinkTime) {
            _data.depth -= 0.05f;
            _data.color = { 1.f, 1.f, 1.f, 1.f };
            _data.texType = TexType::EYE;

            if (!eatingApple && !straightLook) {
                vec2f diff = eyePos - (static_cast<vec2f>(applePos) * cellSize);
                _data.rotateAngle = std::atan2(diff.x, -diff.y);
            }
            else {
                straightLook = true;
            }

            constexpr vec2f eyeSize = cellSize * 0.3f;
            _data.size = eyeSize;

            _data.pos = eye1Pos;
            _rendererHandler->addObject(_data);
            _data.pos = eye2Pos;
            _rendererHandler->addObject(_data);
        }
    }
    // ---------- pravednov head ----------
    else {
        _data.size = 80.f;
        _data.depth = 0.35f;
        _data.color = {1.f, 1.f, 1.f, 1.f};
        _data.texType = TexType::PRAVEDNOV;
        _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
        _data.rotateAngle = getRotateAngle(head + direction, head) - PI2;
        if (direction == Direction::RIGHT)
            _data.texCoord = { 0.f, 1.f, 1.f, 0.f };

        if (headThroughBorder) {
            _data.depth = 0.55f;
            _data.pos = getThroughPos(_data.pos, direction, _fieldSize);
            _rendererHandler->addObject(_data);
        }
        _data.pos = eyePos;
        _rendererHandler->addObject(_data);
    }
}
