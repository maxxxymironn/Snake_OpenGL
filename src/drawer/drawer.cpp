#include "drawer.hpp"

#include "rectangle.hpp"
#include "src/drawer/enums.hpp"
#include "src/drawer/renderer.hpp"
#include "texture_enum.hpp"
#include "src/config/draw_config.hpp"
#include "src/game/enums.hpp"

// #include <iostream>
#include <string>

namespace {
    constexpr float PI = 3.14159265359f;
    constexpr float PI2 = PI / 2.f;

    constexpr float CELL_SIZE = 40.f;
    constexpr vec2f SYMBOL_SIZE = { 16.f, 25.f };
    constexpr vec2f PANEL_SIZE = { 800.f, 160.f };
    constexpr vec2f PANEL_CENTER = PANEL_SIZE * 0.5f;

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

    vec2f calculateViewSize(const vec2f fieldSize, const bool zenMode) {
        constexpr vec2f MIN_VIEW_SIZE = { 800.f, 1000.f };
        return zenMode ? fieldSize
            : vec2f{
                (fieldSize.x <= MIN_VIEW_SIZE.x ? MIN_VIEW_SIZE.x : fieldSize.x) + 40.f,
                (fieldSize.y <= MIN_VIEW_SIZE.y - PANEL_SIZE.y ? MIN_VIEW_SIZE.y : fieldSize.y + PANEL_SIZE.y) + 20.f
            };
    }
}

Drawer::Drawer()
    : _zenMode(DrawConfig::zenMode)
    , _zenModeChanged(false)
    , _updateStaticBuffer(true)
    , _isPravednovHead(false)
    , _contentScale(DrawConfig::contentScale)
    , _origin(0.f, 0.f)
    , _themeColor(DrawConfig::themeColor)
    , _auxThemeColor({ _themeColor.x - 0.05f, _themeColor.y - 0.05f, _themeColor.z - 0.05f, _themeColor.w })
    , _snakeColor(DrawConfig::snakeColor)
{
    _rendererHandler.setClearColor({ _themeColor.x - 0.1f, _themeColor.y - 0.1f, _themeColor.z - 0.1f, _themeColor.w });
}

Drawer::~Drawer() {
    DrawConfig::contentScale = _contentScale;
    DrawConfig::zenMode = _zenMode;
    DrawConfig::snakeColor = _snakeColor;
    DrawConfig::themeColor = _themeColor;
}

void Drawer::updateSnakeColor(const vec4f color) { _snakeColor = color; }

void Drawer::updateThemeColor(const vec4f color) {
    _themeColor = color;
    _auxThemeColor = { color.x - 0.05f, color.y - 0.05f, color.z - 0.05f, color.w };
    _rendererHandler.setClearColor({ color.x - 0.1f, color.y - 0.1f, color.z - 0.1f, color.w });
}

// void Drawer::updateViewSize(const vec2f size) {
//     _viewSize = size;
//     _updateStaticBuffer = true;
// }

void Drawer::updateZenMode() {
    _zenMode = !_zenMode; _zenModeChanged = true;
    _viewSize = calculateViewSize(_fieldSize, _zenMode);
}

void Drawer::updateContentScale(const bool increase) {
    if (increase && _contentScale < 200)
        _contentScale += 10;
    else if (!increase && _contentScale > 50)
        _contentScale -= 10;
}

void Drawer::updateFieldSize(const vec2i fieldCellsNum) {
    _fieldSize = static_cast<vec2f>(fieldCellsNum) * CELL_SIZE;
    _viewSize = calculateViewSize(_fieldSize, _zenMode);
}

void Drawer::changePravednovStatus() {
    _isPravednovHead = !_isPravednovHead;
    _snakeColor = _isPravednovHead ? vec4f{ 0.74f, 0.58f, 0.62f, 1.f }
                                   : DrawConfig::snakeColor;
}

void Drawer::draw(const Game& game, const Clock& clock) {
    const bool inGame = game.status() != GameStatus::MENU && game.status() != GameStatus::MENU_SETTINGS;
    static bool prevStatusMenu = true;

    if ((prevStatusMenu && inGame) || (!prevStatusMenu && !inGame)) {
        _rendererHandler.clearBuffer(BufferType::DYNAMIC);
        prevStatusMenu = !inGame;
    }

    if (_updateStaticBuffer && !_zenMode) {
        _rendererHandler.clearBuffer(BufferType::STATIC);
        _rendererHandler.bindBuffer(BufferType::STATIC);
        _lbPanelPos = _addPanel();
        _addPanelIcons();
        _rendererHandler.refreshBuffer(BufferType::STATIC);
        _updateStaticBuffer = false;
    }

    if (_zenModeChanged) {
        _rendererHandler.setBufferDrawStatus(BufferType::STATIC, !_zenMode);
        _rendererHandler.setBufferDrawStatus(BufferType::STREAM, !_zenMode);
        _zenModeChanged = false;
    }

    if (!_zenMode) {
        _rendererHandler.clearBuffer(BufferType::STREAM);

        _rendererHandler.bindBuffer(BufferType::STREAM);
        const int score = inGame ? game.getScore() : -1;
        _addPanelIconsValues(clock.getPrevFrames(), game.field().size(), game.mode(),
                             clock.getStepTime(), game.getBestScore(), score);
    }

    if (game.status() == GameStatus::MENU) {
        _rendererHandler.bindBuffer(BufferType::STREAM);
        _addMenuButtons();
    }
    else if (game.status() == GameStatus::MENU_SETTINGS) {
        // wip
    }
    else {
        static bool straightLook = false;
        // draw field - STATIC 3
        // draw invisible lines - STATIC 3
        // draw snake - DYNAMIC/STREAM
        // draw apple - STREAM
        _rendererHandler.bindBuffer(BufferType::STREAM);
        _addField(game.field().size());
        _addInvisibleLines();
        _origin = _lbFieldPos - vec2f{ CELL_SIZE * 0.5f };

        bool eatingApple = false;
        vec2i applePos = game.snake().getHeadPos();
        if (game.apple().getPosition().has_value()) {
            applePos = game.apple().getPosition().value();
            eatingApple = game.snake().getHeadPos() == applePos;
            if (!(eatingApple && clock.getSnakeMovingCoeff() > 0.9f))
                _addApple(applePos, clock.getAppleBreathingCoeff());
        }

        if (clock.getSnakeMovingCoeff() > 0.5f) {
            straightLook = false;
            _rendererHandler.clearBuffer(BufferType::DYNAMIC);
            _rendererHandler.bindBuffer(BufferType::DYNAMIC);
            _addDynamicSnake(game.snake());
            _rendererHandler.refreshBuffer(BufferType::DYNAMIC);

            _rendererHandler.bindBuffer(BufferType::STREAM);
        }

        _addStreamSnake(game.snake(), clock.getSnakeMovingCoeff(), eatingApple,
                        applePos, straightLook, clock.isBlinkTime());

        const bool winOrLoose = game.status() == GameStatus::GAME_LOOSE || game.status() == GameStatus::GAME_WIN;
        if ((winOrLoose && clock.isMoreThan(1.f)) || game.status() == GameStatus::GAME_SURE_EXIT)
            _addAuxPanel(game.status());
    }

    _rendererHandler.refreshBuffer(BufferType::STREAM);
    _rendererHandler.draw();
    _rendererHandler.clearBuffer(BufferType::STREAM);
}

vec2f Drawer::_addPanel() {
    _origin = 0.f;

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

void Drawer::_addPanelIcons() {
    _origin = _lbPanelPos;

    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.depth = Depth::PANEL_ICONS;
    _data.rotateAngle = 0.f;

    const bool isThemeLight = _themeColor.x > 0.7 && _themeColor.y > 0.7 && _themeColor.z > 0.7;
    _data.color = isThemeLight ? vec4f{0.f, 0.f, 0.f, 1.f} : vec4f{ 1.f, 1.f, 1.f, 1.f };

    std::string toPrint;

    // ##### LEFT #####
    // field size
    _data.pos = { PANEL_CENTER.x - PANEL_SIZE.x * 0.5f + 28.f, PANEL_CENTER.y + 45.f };
    _printText("SIZE:");

    // game mode
    _data.pos = { PANEL_CENTER.x - PANEL_SIZE.x * 0.5f + 28.f, PANEL_CENTER.y };
    _printText("SPEED:");

    // snake speed
    _data.pos = { PANEL_CENTER.x - PANEL_SIZE.x * 0.5f + 28.f, PANEL_CENTER.y - 45.f };
    _printText("MODE:");

    // ##### RIGHT #####
    // fps
    _data.pos = { PANEL_CENTER.x + PANEL_SIZE.x * 0.5f - 28.f, PANEL_CENTER.y + 45.f };
    _printText("FPS", Alignment::RIGHT);

    // menu button
    _data.pos = { PANEL_CENTER.x + PANEL_SIZE.x * 0.5f - 28.f, PANEL_CENTER.y - 45.f };
    _printText("MENU", Alignment::RIGHT);

    // ##### CENTER #####
    // best score
    _data.size = CELL_SIZE * 1.2;
    _data.pos = { PANEL_CENTER.x - 40.f, PANEL_CENTER.y + 30.f };
    _data.texType = TexType::STAR;
    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.color = { 1.f, 0.8f, 0.f, 1.f };  // yellow
    _add(_data);
}

void Drawer::_addPanelIconsValues(const int fps, const vec2i fieldSize, const GameMode mode,
                                  const float stepTime, const int bestScore, const int score)
{
    _origin = _lbPanelPos;

    _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    _data.color = { 1.f, 1.f, 1.f, 1.f };
    _data.depth = Depth::PANEL_ICONS;
    _data.rotateAngle = 0.f;

    const bool isThemeLight = _themeColor.x > 0.7 && _themeColor.y > 0.7 && _themeColor.z > 0.7;
    _data.color = isThemeLight ? vec4f{0.f, 0.f, 0.f, 1.f} : vec4f{ 1.f, 1.f, 1.f, 1.f };

    std::string toPrint;

    // ##### LEFT #####
    // field size
    _data.pos = { PANEL_CENTER.x - PANEL_SIZE.x * 0.5f + 28.f + 6 * 16.f, PANEL_CENTER.y + 45.f };
    toPrint = std::to_string(fieldSize.x);
    toPrint += ":";
    toPrint += std::to_string(fieldSize.y);
    _printText(toPrint);

    // game mode
    _data.pos = { PANEL_CENTER.x - PANEL_SIZE.x * 0.5f + 28.f + 7 * 16.f, PANEL_CENTER.y };
    toPrint = stepTime == 0.6f ? "SLOW"
            : (stepTime == 0.3f ? "MEDIUM"
            : (stepTime == 0.15f ? "FAST"
            : (stepTime == 0.1f ? "VERY FAST"
            : "CUSTOM")));
    _printText(toPrint);

    // snake speed
    _data.pos = { PANEL_CENTER.x - PANEL_SIZE.x * 0.5f + 28.f + 6 * 16.f, PANEL_CENTER.y - 45.f };
    toPrint = mode == GameMode::DEFAULT ? "DEFAULT"
            : (mode == GameMode::THROUGH_WALLS ? "THROUGH"
            : "BOUNDLESS");
    _printText(toPrint);

    // ##### RIGHT #####
    // fps
    _data.pos = { PANEL_CENTER.x + PANEL_SIZE.x * 0.5f - 28.f - 4 * 16.f, PANEL_CENTER.y + 45.f };
    _printText(std::to_string(fps), Alignment::RIGHT);

    // ##### CENTER #####
    // best score
    _data.pos = { PANEL_CENTER.x + 40.f, PANEL_CENTER.y + 30.f };
    _printText(std::to_string(bestScore));

    // score
    if (score != -1) {
        _data.size = CELL_SIZE;
        _data.pos = { PANEL_CENTER.x - 40.f, PANEL_CENTER.y - 30.f };
        _data.texType = TexType::APPLE;
        _data.texCoord = { 0.f, 0.f, 1.f, 1.f };
        _add(_data);

        _data.pos = { PANEL_CENTER.x + 40.f, PANEL_CENTER.y - 30.f };
        _printText(std::to_string(score));
    }
}

void Drawer::_addMenuButtons() {
    const bool isThemeLight = _themeColor.x > 0.7 && _themeColor.y > 0.7 && _themeColor.z > 0.7;
    const float symbolScale = 2.f;
    const vec2f tempPos = {
        _viewSize.x * 0.5f,
        _zenMode ? _viewSize.y * 0.5f : _viewSize.y * 0.5f - PANEL_SIZE.y * 0.5f - 10.f
    };

    _origin = 0.f;
    _data.pos = tempPos;
    _data.depth = Depth::PANEL;
    _data.color = isThemeLight ? vec4f{0.f, 0.f, 0.f, 1.f} : vec4f{ 1.f, 1.f, 1.f, 1.f };
    _data.rotateAngle = 0.f;

    _printText("SETTINGS", Alignment::CENTER, symbolScale);

    _data.pos = { tempPos.x, tempPos.y + SYMBOL_SIZE.y * symbolScale * 1.5f };
    _printText("PLAY", Alignment::CENTER, symbolScale);

    _data.pos = { tempPos.x, tempPos.y - SYMBOL_SIZE.y * symbolScale * 1.5f };
    _printText("EXIT", Alignment::CENTER, symbolScale);
}

void Drawer::_addAuxPanel(const GameStatus status) {
    _origin = 0.f;
    const vec2f auxPanelSize = _data.size = { 300.f, 200.f };
    _data.pos = { (_viewSize.x) * 0.5f, (_viewSize.y - (_zenMode ? 0.f : PANEL_SIZE.y + 20.f)) * 0.5f };
    _data.depth = Depth::PANEL;
    _data.texType = TexType::SNAKE_BODY;
    _data.texCoord = { 0.45f, 0.45f, 0.5f, 0.5f };
    _data.color = { 0.0f, 0.0f, 0.0f, 0.6f };
    _data.rotateAngle = 0.f;
    _add(_data);

    _origin = _data.pos - auxPanelSize * 0.5f;
    _data.pos = { auxPanelSize.x * 0.5f, auxPanelSize.y };
    _data.depth -= 0.1f;
    _data.color = 1.f;

    float scaleCoeff = 2.f;
    if (status == GameStatus::GAME_SURE_EXIT) {
        _printText("QUIT TO MENU?", Alignment::CENTER, scaleCoeff);
        _data.pos = { auxPanelSize.x * 0.25f, auxPanelSize.y * 0.5f };
        scaleCoeff = 1.5f;
        _printText("YES", Alignment::CENTER, scaleCoeff);
        _data.pos = { auxPanelSize.x * 0.75f, auxPanelSize.y * 0.5f };
        _printText("NO", Alignment::CENTER, scaleCoeff);
    }
    else {
        _printText(status == GameStatus::GAME_WIN ? "YOU WIN" : "YOU LOOSE", Alignment::CENTER, scaleCoeff);
        _data.pos = { auxPanelSize.x * 0.5f, auxPanelSize.y * 0.55f };
        scaleCoeff = 1.5f;
        _printText("RESTART", Alignment::CENTER, scaleCoeff);
        _data.pos = { auxPanelSize.x * 0.5f, auxPanelSize.y * 0.25f };
        _printText("QUIT TO MENU", Alignment::CENTER, scaleCoeff);
    }
}

void Drawer::_addField(const vec2i cellsNum) {
    _origin = 0.f;

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
                            const vec2i applePos, bool& straightLook, const bool isBlinkTime)
{
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

void Drawer::_printText(const std::string& str, const Alignment alignment, const float scale) {
    _data.texType = TexType::ALPHABET;
    _data.size = SYMBOL_SIZE * scale;
    _data.pos.y -= 2.5f * scale;

    switch (alignment) {
        case Alignment::CENTER:
            _data.pos.x -= static_cast<float>(str.size() - 1) * 0.5f * _data.size.x;
            break;
        case Alignment::RIGHT:
            _data.pos.x -= static_cast<float>(str.size() - 1) * _data.size.x;
            break;
        default: break;
    }

    for (const char symbol : str) {
        if (symbol == ' ') {
            _data.pos.x += _data.size.x;
            continue;
        }

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
        int y = shiftNum / 8;
        int x = shiftNum - y * 8;

        constexpr vec2f texSymbolSize = { 0.125f, 0.1953f };
        const vec2f texSymbolPos = texSymbolSize * static_cast<vec2f>(vec2i{ x + 1, y + 1 });
        _data.texCoord = { texSymbolPos - texSymbolSize, texSymbolPos };
        _add(_data);
        _data.pos.x += _data.size.x;
    }
}

void Drawer::_add(const RectangleData& data) {
    float contentScale = static_cast<float>(_contentScale) / 100.f;
    vec2f size = data.size * 0.5f * contentScale;
    vec2f pos = data.pos * contentScale;

    vec2f normViewSize = _viewSize * 0.5f * contentScale;
    vec2f normOrigin = _origin * contentScale - normViewSize;

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
