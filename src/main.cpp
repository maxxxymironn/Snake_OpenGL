#include "config/config_manager.hpp"
#include "core/clock.hpp"
#include "core/input_manager.hpp"
#include "core/vector.hpp"
#include "game/enums.hpp"
#include "game/game.hpp"
#include "game/snake.hpp"
#include "drawer/drawer.hpp"
#include "window/window.hpp"

void checkPressedKeys(Window& window, Drawer& drawer, Game& game, Clock& clock, InputManager& inputManager);

void updateDir(Snake& snake, InputManager& inputManager);

int main() {
    ConfigManager configManager;
    InputManager inputManager;
    Window window;
    Drawer drawer;
    if(!drawer.getRendererStatus()) {
        window.close();
        return -1;
    }
    Game game;
    Clock clock;

    window.updateFieldSize(static_cast<vec2i>(game.field().getFieldSize()));
    drawer.updateSnakeColor(game.getSnakeColor());
    drawer.updateThemeColor(game.getThemeColor());
    drawer.updateViewSize(window.getRawViewSize());

    window.setInputManagerSetKey(
        [&inputManager](Action actionKey, bool isPressed, Mod mods) {
            inputManager.setKey(actionKey, isPressed, mods);
        }
    );
    window.setRefreshCallback(
        [&drawer, &game, &clock]() {
            drawer.draw(game, clock);
        }
    );

    clock.start();
    while(!window.shouldClose()) {
        clock.calculate();

        if (clock.updateFPS())
            window.updateFPS(clock.getPrevFrames());

        drawer.draw(game, clock);

        checkPressedKeys(window, drawer, game, clock, inputManager);

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

void checkPressedKeys(Window& window, Drawer& drawer, Game& game, Clock& clock, InputManager& inputManager) {
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
        drawer.updateContentScale(window.getContentScale());
    }

    else if (inputManager.isKeyPressed(Action::ScaleDown)) {
        window.decreaseContentScaling();
        drawer.updateContentScale(window.getContentScale());
    }

    else if (inputManager.isKeyPressed(Action::ZenMode)) {
        window.changeZenStatus();
        drawer.updateZenMode(window.getZenMode());
        drawer.updateViewSize(window.getRawViewSize());
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
            case Action::MoveUp: newDir = Direction::UP; break;
            case Action::MoveDown: newDir = Direction::DOWN; break;
            case Action::MoveLeft: newDir = Direction::LEFT; break;
            case Action::MoveRight: newDir = Direction::RIGHT; break;
            default: break;
        }

        if (snake.getDirection() + newDir != vec2i{0, 0})
            snake.setDirection(newDir);
    }
}
