#include "config/config_manager.hpp"
#include "core/clock.hpp"
#include "core/input_manager.hpp"
#include "core/rectangle.hpp"
#include "core/vector.hpp"
#include "game/enums.hpp"
#include "game/game.hpp"
#include "game/snake.hpp"
#include "render/texture_enum.hpp"
#include "render/renderer.hpp"
#include "window/window.hpp"

#include <cstdlib>

void checkPressedKeys(Window& window, Renderer& renderer, Game& game, Clock& clock, InputManager& inputManager);

void updateDir(Snake& snake, InputManager& inputManager);

void draw(Renderer& renderer, const Game& game, const Clock& clock);

float getRotateAngle(const vec2i v1, const vec2i v2);

vec2f getThroughPos(vec2f position, const vec2i direction, const vec2f fieldSize);

int main() {
    ConfigManager configManager;
    InputManager inputManager;
    Window window;
    Renderer renderer;
    if(!renderer.getInitializeInfo()) {
        window.close();
        return -1;
    }
    Game game;
    Clock clock;
    window.setInputManagerSetKey(
        [&inputManager](Action actionKey, bool isPressed, Mod mods) {
            inputManager.setKey(actionKey, isPressed, mods);
        }
    );
    window.setRefreshCallback([&renderer, &game, &clock]() { draw(renderer, game, clock); });

    window.updateFieldSize(static_cast<vec2i>(game.field().getFieldSize()));
    renderer.setContentScale(window.getContentScale());
    renderer.setViewSize(window.getViewSize());
    renderer.setClearColor(game.getThemeColor() - vec4f{0.1f, 0.1f, 0.1f, 0.f});
    clock.start();
    while(!window.shouldClose()) {
        clock.calculate();

        if (clock.updateFPS())
            window.updateFPS(clock.getPrevFrames());

        draw(renderer, game, clock);

        checkPressedKeys(window, renderer, game, clock, inputManager);

        /* Logic part */
        if (!clock.isPauseTime()) {
            if (game.status() == GameStatus::GAME) {
                while(clock.isUpdateTime()) {
                    updateDir(game.snake(), inputManager);
                    game.update();

                    clock.updateGameStepAccumulator();

                    if (game.apple().isNew()) {
                        window.updateScore();
                        game.apple().setOld();
                    }
                }
            }
            else if (game.status() == GameStatus::LOOSE) {
                if (configManager.isReadyToSaveFile()) {
                    game.saveStats();
                    configManager.saveFile();
                }
                if (clock.getSnakeMovingCoeff() > 0.1f) {
                    clock.freezeSnake();
                }
                if (clock.isUpdateTime(3.f)) {
                    game.reset();
                    window.updateScore(false);
                    inputManager.turnOffBuffer();
                    clock.resetGameStepAccumulator();
                }
            }
            else if (game.status() == GameStatus::WIN) {
                if (configManager.isReadyToSaveFile()) {
                    game.saveStats();
                    configManager.saveFile();
                    clock.freezeSnake();
                }
                if (clock.isUpdateTime(3.f)) {
                    game.reset();
                    window.updateScore(false);
                    inputManager.turnOffBuffer();
                    clock.resetGameStepAccumulator();
                }
            }
            else if (game.status() == GameStatus::GAME_START) {
                if (game.apple().isNew()) {
                    window.updateScore();
                    game.apple().setOld();
                }

                if (clock.isUpdateTime(0.5f))  {
                    configManager.setReadyToSaveFile();
                    game.updateStatus(GameStatus::GAME);
                    game.update();
                    clock.resetGameStepAccumulator();
                    inputManager.turnOnBuffer();
                    clock.unfreezeSnake();
                }
            }
        }
        /* End logic */

        inputManager.update();

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}

void checkPressedKeys(Window& window, Renderer& renderer, Game& game, Clock& clock, InputManager& inputManager) {
    if (inputManager.isKeyPressed(Action::Exit)) {
        if (game.status() == GameStatus::MENU)
            window.close();
        else {
            window.showDetailedTitle(false);
            game.updateStatus(GameStatus::MENU);
            game.setDefaultHead();
            if (clock.isPauseTime())
                clock.updatePauseStatus();
        }
    }

    else if (inputManager.isKeyPressed(Action::ScaleUp)) {
        window.increaseContentScaling();
        renderer.setContentScale(window.getContentScale());
        renderer.setViewSize(window.getViewSize());
    }

    else if (inputManager.isKeyPressed(Action::ScaleDown)) {
        window.decreaseContentScaling();
        renderer.setContentScale(window.getContentScale());
        renderer.setViewSize(window.getViewSize());
    }

    else if (inputManager.isKeyPressed(Action::ZenMode)) {
        window.changeZenStatus();
        renderer.changeZenMode();
        renderer.setViewSize(window.getViewSize());
    }

    else if (game.status() == GameStatus::MENU) {
        if (inputManager.isKeyPressed(Action::Space) || window.isButtonClicked()) {
            window.setMenuMode(false);
            window.updateScore(false);
            window.showDetailedTitle(true);

            game.updateStatus(GameStatus::GAME_START);
            game.reset();

            inputManager.turnOffBuffer();
            clock.resetGameStepAccumulator();
            clock.freezeSnake();
        }
    }

    else if (game.status() != GameStatus::MENU) {
        if (inputManager.isKeyPressed(Action::Pause)) {
            window.setMenuMode(true);
            clock.updatePauseStatus();
            inputManager.changeWorkStatus();
        }
        else if (inputManager.isKeyPressed(Action::EasterEgg)) {
            if (game.getPravednovBool())
                game.setDefaultHead();
            else
                game.setPravednovHead();
        }
    }
}

void updateDir(Snake& snake, InputManager& inputManager) {
    if (!inputManager.isBufferEmpty()) {
        vec2i newDir;
        switch (inputManager.getBufferKey()) {
            case Action::MoveUp:   newDir = Direction::UP; break;
            case Action::MoveDown: newDir = Direction::DOWN; break;
            case Action::MoveLeft: newDir = Direction::LEFT; break;
            case Action::MoveRight:newDir = Direction::RIGHT; break;
            default: break;
        }

        if (snake.getDirection() + newDir != vec2i{0, 0})
            snake.setDirection(newDir);
    }
}

void draw(Renderer& renderer, const Game& game, const Clock& clock) {
    constexpr float cellSize = 40.f;
    const vec2f fieldSize = static_cast<vec2f>(game.field().getFieldSize()) * cellSize;

    const vec4f snakeColor = game.getSnakeColor();
    const vec4f themeColor = game.getThemeColor();

    rectangleData data(
        cellSize, {0.f, 0.f}, 0.9f, {1.f, 1.f, 1.f, 1.f},
        {0.f, 0.f, 1.f, 1.f}, TexType::SNAKE_BODY, 0.f
    );

    static bool isMenuStatus = true;
    GameStatus gameStatus = game.status();

    bool updatedStaticData = false;
    /* ###### static objects ###### */
    if (renderer.needRefreshStaticBuffer()
        || (!isMenuStatus && gameStatus == GameStatus::MENU)
        || (isMenuStatus && gameStatus != GameStatus::MENU)
    ) {
        renderer.setOrigin(vec2f{ 0.f, 0.f });

        constexpr vec2f minViewSize{800.f, 1000.f};
        constexpr vec2f fieldTexSize{80.f, 80.f};
        vec2i rawFieldSize(game.field().getFieldSize());

        vec2f startPos(0.f, 0.f);
        if (!renderer.isZenMode()) {
            startPos.x = fieldSize.x <= minViewSize.x ? (minViewSize.x - fieldSize.x + cellSize) * 0.5f : 20.f;
            startPos.y = (fieldSize.y <= minViewSize.y - 200.f ? (minViewSize.y - 200.f - fieldSize.y) * 0.5f : 0.f) + 20.f;
        }
        startPos += fieldTexSize * 0.5f;
        const vec2f fieldStartPos = startPos - fieldTexSize * 0.5f;

        // ========== info panel ==========
        if (!renderer.isZenMode()) {
            data.depth = 0.1f;
            data.color = {themeColor.x - 0.05f, themeColor.y - 0.05f, themeColor.z - 0.05f, themeColor.w};
            data.size = { 200.f, 800.f };
            data.texType = TexType::SNAKE_BODY;
            data.texCoord = { 0.f, 0.f, 1.f, 1.f };
            data.rotateAngle = 3.141592f * 0.5f;
            data.pos = vec2f{
                fieldStartPos.x + fieldSize.x * 0.5f,
                (fieldSize.y <= minViewSize.y - 200.f ? minViewSize.y - data.size.x * 0.5f : fieldSize.y + 100.f) + 20.f
            };
            renderer.addObject(data);
            data.rotateAngle = 0.f;

            // ~~~~~ icons on the panel ~~~~~
            data.depth = 0.0f;
        }

        // ========== field ==========
        if (gameStatus != GameStatus::MENU) {
            isMenuStatus = false;

            data.depth = 0.9f;
            data.size = cellSize * 2.f;
            data.color = themeColor;
            data.texType = TexType::FIELD;

            bool evenXCellsCount = rawFieldSize.x % 2 == 0;
            bool evenYCellsCount = rawFieldSize.y % 2 == 0;
            vec2i fieldCellsCount(
                (rawFieldSize.x + (evenXCellsCount ? 0 : 1)) / 2,
                (rawFieldSize.y + (evenYCellsCount ? 0 : 1)) / 2
            );
            for (int x = 0; x < fieldCellsCount.x; ++x) {
                for (int y = 0; y < fieldCellsCount.y; ++y) {
                    data.pos = startPos + vec2f{ data.size.x * x, data.size.y * y };
                    renderer.addObject(data);
                }
            }

            data.depth = 0.5f;
            data.color = {themeColor.x - 0.1f, themeColor.y - 0.1f, themeColor.z - 0.1f, themeColor.w};
            data.texCoord = { 0.1f, 0.1f };

            const vec2f fieldCenter = fieldStartPos + fieldSize * 0.5f;
            data.pos = fieldCenter;

            data.size = { cellSize, fieldSize.y };
            // left
            data.pos.x = fieldCenter.x - (fieldSize.x + cellSize) * 0.5f;
            renderer.addObject(data);
            // right
            data.pos.x = fieldCenter.x + (fieldSize.x + cellSize) * 0.5f;
            renderer.addObject(data);

            data.pos.x = fieldCenter.x;
            data.size = { fieldSize.x + cellSize * 2.f, cellSize };
            // top
            data.pos.y = fieldCenter.y + (fieldSize.y + cellSize) * 0.5f;
            renderer.addObject(data);
            //bottom
            data.pos.y = fieldCenter.y - (fieldSize.y + cellSize) * 0.5f;
            renderer.addObject(data);
        }

        updatedStaticData = true;
        renderer.setOrigin(fieldStartPos);
        renderer.refreshStaticBuffer();
    }

    data.size = cellSize;
    data.color = snakeColor;
    data.texCoord = { 0.f, 0.f, 1.f, 1.f };
    data.depth = 0.8f;

    if (gameStatus != GameStatus::MENU) {
        const vec2i direction = game.snake().getDirection();
        std::vector<vec2i> snakeBody = game.snake().getBody();
        const size_t snakeSize = snakeBody.size();
        const vec2i head = snakeBody.front();
        const vec2i tail = snakeBody.back();
        const vec2i applePos = game.apple().getPosition();
        const float snakeMovingCoeff = clock.getSnakeMovingCoeff();
        const bool eatingApple = head == applePos;

        vec2i prevTail = game.snake().getPrevTail();
        static vec2i lastHead(*(snakeBody.begin() + 1));
        static bool straightView = false;

        const bool headThroughBorder = std::abs(head.x - snakeBody[1].x) > 1
                                    || std::abs(head.y - snakeBody[1].y) > 1;
        const bool tailThroughBorder = std::abs(tail.x - prevTail.x) > 1
                                    || std::abs(tail.y - prevTail.y) > 1;

        /* ###### dynamic objects ###### */
        // snake body
        if (updatedStaticData || (head != lastHead && (snakeMovingCoeff > 0.5f || game.status() == GameStatus::GAME_START))) {
            lastHead = head;
            straightView = false;

            for (size_t i = snakeSize - 2; i > 0; --i) {
                vec2i prev = snakeBody[i - 1];
                vec2i cur = snakeBody[i];
                vec2i next = snakeBody[i + 1];

                data.pos = static_cast<vec2f>(cur) * cellSize + data.size * 0.5f;
                data.rotateAngle = getRotateAngle(next, cur);
                data.texType = TexType::SNAKE_BODY;

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
                        data.rotateAngle += 3.14159265359f / 2.f;

                    data.texType = TexType::SNAKE_CORNER;
                }

                renderer.addObject(data);
            }

            renderer.refreshDynamicBuffer();
        }

        /* ###### stream objects ###### */
        // apple
        if (!(eatingApple && snakeMovingCoeff > 0.9f)) {
            data.depth = 0.4f;
            data.rotateAngle = 0.f;
            data.color = {1.f, 1.f, 1.f, 1.f};
            data.size = cellSize;
            data.pos = static_cast<vec2f>(applePos) * cellSize + data.size * 0.5f;
            data.size *= clock.getAppleBreathingCoeff();
            data.texType = TexType::APPLE;
            renderer.addObject(data);
        }

        static bool newDirectionTail = false;
        bool drawHead = true;
        bool drawTail = true;

        data.color = snakeColor;
        data.depth = 0.8f;
        if (snakeMovingCoeff <= 0.5f) {
            data.size = cellSize;

            vec2i prev = snakeSize > 2 ? snakeBody[2] : prevTail;
            vec2i cur = snakeBody[1];
            vec2i next = head;

            vec2i diff = next - prev;
            // turning head
            if (diff.x != 0 && diff.y != 0) {
                // waiting head
                data.pos = static_cast<vec2f>(cur) * cellSize + data.size * 0.5f;
                data.texCoord = {0.f, -0.1f, 1.f, 0.9f };
                data.rotateAngle = getRotateAngle(cur, prev);
                data.texType = TexType::SNAKE_TAIL;
                renderer.addObject(data);

                // dynamic head
                data.pos = (static_cast<vec2f>(cur) + static_cast<vec2f>(direction) * 0.5f) * cellSize + data.size * 0.5f;
                data.rotateAngle = getRotateAngle(next + direction, next);
                data.texCoord = { 0.f, -0.5f + snakeMovingCoeff, 1.f, 0.5f + snakeMovingCoeff};

                if (headThroughBorder) {
                    data.pos = static_cast<vec2f>(head) * cellSize + data.size * 0.5f;
                    data.texCoord = { 0.f, -1.f + snakeMovingCoeff, 1.f, snakeMovingCoeff };
                    renderer.addObject(data);

                    data.pos = static_cast<vec2f>(snakeBody[1]) * cellSize + data.size * 0.5f + static_cast<vec2f>(direction) * cellSize * 0.25f;
                    data.texCoord = { 0.f, 0.0f + snakeMovingCoeff, 1.f, 0.5f + snakeMovingCoeff};
                    data.size = { cellSize, cellSize * 0.5f };
                }
                renderer.addObject(data);

                drawHead = false;
            }
            // aux straight head
            else if (snakeSize > 2) {
                data.rotateAngle = getRotateAngle(next, cur);
                data.pos = static_cast<vec2f>(cur) * cellSize + data.size * 0.5f
                        - static_cast<vec2f>(direction) * cellSize * 0.25f;
                data.size = { cellSize, cellSize * 0.5f };
                data.texCoord = { 0.f, 0.f, 1.f, 0.5f };
                data.texType = TexType::SNAKE_BODY;
                renderer.addObject(data);
            }

            if (newDirectionTail) {
                float auxSnakeMovingCoeff = snakeMovingCoeff;

                if (!eatingApple) {
                    prev = game.snake().getPrevPrevTail();
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
                data.size = cellSize;
                data.pos = static_cast<vec2f>(cur) * cellSize + data.size * 0.5f;
                data.texCoord = {0.f, -0.1f, 1.f, 0.9f };
                data.rotateAngle = getRotateAngle(cur, next);
                data.texType = TexType::SNAKE_TAIL;
                renderer.addObject(data);

                // aux static body to fill space between head and tail in 2sized snake
                data.pos = static_cast<vec2f>(next) * cellSize + data.size * 0.5f;
                data.texCoord = { 0.f, 0.5f, 1.f, 1.5f };
                data.texType = TexType::SNAKE_BODY;
                renderer.addObject(data);

                // new step after turned tail -- little dynamic tail
                diff = cur - prev;
                if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
                else  			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;
                data.pos = (static_cast<vec2f>(cur) - static_cast<vec2f>(diff) * 0.5f) * cellSize + data.size * 0.5f;
                data.texCoord = {0.f, -0.5f - auxSnakeMovingCoeff, 1.f, 0.5f - auxSnakeMovingCoeff};
                data.rotateAngle = getRotateAngle(prev, cur);
                data.texType = TexType::SNAKE_TAIL;
                renderer.addObject(data);

                drawTail = false;
                if (auxSnakeMovingCoeff > 0.1f)
                    newDirectionTail = false;
            }
        }
        else {
            data.size = cellSize;

            vec2i prev = prevTail;
            vec2i cur = tail;
            vec2i next = *(snakeBody.rbegin() + 1);

            vec2i diff = next - prev;
            // turning tail
            if ((diff.x != 0 && diff.y != 0)) {
                // waiting tail
                data.size = cellSize;
                data.pos = static_cast<vec2f>(cur) * cellSize + data.size * 0.5f;
                data.texCoord = {0.f, -0.1f, 1.f, 0.9f };
                data.rotateAngle = getRotateAngle(cur, next);
                data.texType = TexType::SNAKE_TAIL;
                renderer.addObject(data);

                // dynamic tail
                diff = cur - prev;
                if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
                else  			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;
                data.pos = (static_cast<vec2f>(cur) - static_cast<vec2f>(diff) * 0.5f) * cellSize + data.size * 0.5f;
                data.rotateAngle = getRotateAngle(prev, cur);
                data.texCoord = eatingApple ? vec4f{0.f, -0.5f, 1.f, 0.5f }
                                            : vec4f{0.f, 0.5f - snakeMovingCoeff, 1.f, 1.5f - snakeMovingCoeff};

                if (tailThroughBorder && !eatingApple) {
                    data.texType = TexType::SNAKE_TAIL;
                    data.pos = static_cast<vec2f>(prevTail) * cellSize + data.size * 0.5f;
                    data.texCoord = { 0.f, -snakeMovingCoeff , 1.f, 1.f - snakeMovingCoeff};
                    renderer.addObject(data);

                    data.pos = static_cast<vec2f>(tail) * cellSize + data.size * 0.5f - static_cast<vec2f>(diff) * cellSize * 0.25f;
                    data.texCoord = { 0.f, 1.f - snakeMovingCoeff, 1.f, 1.5f - snakeMovingCoeff };
                    data.size = { cellSize, cellSize * 0.5f };
                }
                renderer.addObject(data);

                drawTail = false;
                newDirectionTail = true;
            }
            // aux straight tail
            else if (snakeSize > 2) {
                vec2i diff = next - cur;
                if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
                else 			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;

                data.pos = static_cast<vec2f>(cur) * cellSize + data.size * 0.5f
                        + static_cast<vec2f>(diff) * cellSize * 0.25f;
                data.size = { cellSize, cellSize * 0.5f };
                data.texCoord = { 0.f, 0.f, 1.f, 0.5f };
                data.rotateAngle = getRotateAngle(cur, next);
                data.texType = TexType::SNAKE_BODY;
                renderer.addObject(data);
            }
        }

        // snake head
        if (drawHead) {
            vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;

            data.size = cellSize;
            data.pos = (static_cast<vec2f>(snakeBody[1]) + alpha) * cellSize + data.size * 0.5f;
            data.texType = TexType::SNAKE_TAIL;
            data.texCoord = {0.f, 0.f, 1.f, 1.f};
            data.rotateAngle = getRotateAngle(head + direction, head);

            renderer.addObject(data);
            if (headThroughBorder) {
                data.pos = getThroughPos(data.pos, direction, fieldSize);
                renderer.addObject(data);
            }
        }

        // snake tail
        if (drawTail) {
            vec2i diff = tail - prevTail;
            if (diff.x != 0) diff.x = (diff.x == 1 || diff.x < -1) ? 1 : -1;
            else 			 diff.y = (diff.y == 1 || diff.y < -1) ? 1 : -1;

            vec2f alpha = static_cast<vec2f>(diff) * snakeMovingCoeff;
            
            data.size = cellSize;
            data.pos = (static_cast<vec2f>(prevTail) + alpha) * cellSize + data.size * 0.5f;
            data.texType = TexType::SNAKE_TAIL;
            data.texCoord = { 0.f, 0.f, 1.f, 1.f };
            data.rotateAngle = getRotateAngle(prevTail, tail);

            if (eatingApple)
                data.pos = static_cast<vec2f>(tail) * cellSize + data.size * 0.5f;

            renderer.addObject(data);
            if (tailThroughBorder) {
                data.pos = getThroughPos(data.pos, diff, fieldSize);
                renderer.addObject(data);
            }
        }

        const vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;
        const vec2f eyePos = (static_cast<vec2f>(snakeBody[1]) + alpha) * cellSize + data.size * 0.5f;

        // ---------- eyes ----------
        if (!game.getPravednovBool()) {
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

            data.size = eyePlaceSize;
            data.pos = eye1Pos;
            data.color = { snakeColor.x - 0.03f, snakeColor.y - 0.03f, snakeColor.z - 0.03f, snakeColor.w };
            data.texCoord = {0.f, 0.f, 1.f, 1.f};
            data.texType = TexType::EYE_ORBIT;
            data.rotateAngle = getRotateAngle(head + direction, head);

            // eye places
            if (!headThroughBorder) {
                data.depth = 0.2f;
                renderer.addObject(data);
                data.pos = eye2Pos;
                renderer.addObject(data);
            }
            else {
                data.depth = 0.6f;
                
                renderer.addObject(data);
                data.pos = getThroughPos(data.pos, direction, fieldSize);
                renderer.addObject(data);
                
                data.pos = eye2Pos;
                renderer.addObject(data);
                data.pos = getThroughPos(data.pos, direction, fieldSize);
                renderer.addObject(data);
            }

            if (!(headThroughBorder && snakeMovingCoeff > 0.6f) && !clock.isBlinkTime()) {
                data.depth -= 0.05f;
                data.color = { 1.f, 1.f, 1.f, 1.f };
                data.texType = TexType::EYE;

                if (!eatingApple && !straightView) {
                    vec2f diff = eyePos - (static_cast<vec2f>(applePos) * cellSize + data.size * 0.5f);
                    data.rotateAngle = std::atan2(diff.x, -diff.y);
                }
                else {
                    straightView = true;
                }

                constexpr vec2f eyeSize = cellSize * 0.3f;
                data.size = eyeSize;

                data.pos = eye1Pos;
                renderer.addObject(data);
                data.pos = eye2Pos;
                renderer.addObject(data);
            }
        }
        // ---------- pravednov head ----------
        else {
            data.size = 80.f;
            data.depth = 0.35f;
            data.color = {1.f, 1.f, 1.f, 1.f};
            data.texType = TexType::PRAVEDNOV;
            data.texCoord = { 0.f, 0.f, 1.f, 1.f };
            data.rotateAngle = getRotateAngle(head + direction, head) - 1.570796f;
            if (direction == Direction::RIGHT)
                data.texCoord = { 0.f, 1.f, 1.f, 0.f };

            if (headThroughBorder) {
                data.depth = 0.55f;
                data.pos = getThroughPos(data.pos, direction, fieldSize);
                renderer.addObject(data);
            }
            data.pos = eyePos;
            renderer.addObject(data);
        }

        renderer.refreshStreamBuffer();
    }
    else {
        data.depth = 0.9f;
        if (!isMenuStatus) {
            isMenuStatus = true;
            renderer.refreshDynamicBuffer();
        }
        data.color = (themeColor.x < 0.7f && themeColor.y < 0.7f && themeColor.z < 0.7f)
                    ? vec4f{ 1.f, 1.f, 1.f, 1.f}
                    : vec4f{0.f, 0.f, 0.f, 1.f };
        data.size = 256.f * clock.getAppleBreathingCoeff();
        data.pos = fieldSize * 0.5f;
        data.texType = TexType::PLAY;
        renderer.addObject(data);
        renderer.refreshStreamBuffer();
    }
    renderer.draw();
}

float getRotateAngle(const vec2i v1, const vec2i v2) {
    constexpr float PI = 3.14159265359f;
    constexpr float PI2 = PI / 2.f;

    vec2i diff(v1 - v2);
    if (diff.x != 0)
        return diff.x == -1 || diff.x > 1 ? PI2 : -PI2;
    else
        return diff.y == -1 || diff.y > 1 ? PI : 0.f;
}

vec2f getThroughPos(vec2f position, const vec2i direction, const vec2f fieldSize) {
    if (direction.x != 0)
        position.x += fieldSize.x * (direction.x > 0 ? -1.f : 1.f);
    else
        position.y += fieldSize.y * (direction.y > 0 ? -1.f : 1.f);

    return position;
}
