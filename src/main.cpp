#include "config/config_manager.hpp"
#include "core/clock.hpp"
#include "core/input_manager.hpp"
#include "core/vector.hpp"
#include "game/enums.hpp"
#include "game/game.hpp"
#include "game/snake.hpp"
#include "drawer/drawer.hpp"
#include "src/core/action.hpp"
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

    drawer.updateFieldSize(game.field().size());
    window.updateView(drawer.getViewSize(), drawer.getContentScale(), drawer.getZenMode());

    window.setInputManagerSetKey([&inputManager](Action actionKey, bool isPressed, Mod mods) {
        inputManager.setKey(actionKey, isPressed, mods);
    });
    window.setRefreshCallback([&drawer, &game, &clock]() { drawer.draw(game, clock); });

    clock.start();
    while(!window.shouldClose()) {
        clock.calculate();
        clock.updateFPS();

        checkPressedKeys(window, drawer, game, clock, inputManager);
        drawer.draw(game, clock);

        if (!clock.isPauseTime()) {
            switch (game.status()) {
                case GameStatus::GAME:
                    while(clock.isMoreThan()) {
                        updateDir(game.snake(), inputManager);
                        game.update();
                        clock.updateGameStepAccumulator();
                    }
                    break;
                case GameStatus::GAME_START:
                    if (clock.isMoreThan(0.5f))  {
                        configManager.setReadyToSaveFile();
                        game.updateStatus(GameStatus::GAME);
                        game.update();
                        clock.resetGameStepAccumulator();
                        inputManager.turnOnBuffer();
                        clock.unfreezeSnake();
                    }
                    break;
                case GameStatus::GAME_WIN:
                    if (configManager.isReadyToSaveFile()) {
                        game.saveStats();
                        configManager.saveFile();
                    }
                    clock.freezeSnake();
                    break;
                case GameStatus::GAME_LOOSE:
                    if (configManager.isReadyToSaveFile()) {
                        game.saveStats();
                        configManager.saveFile();
                    }
                    if (clock.getSnakeMovingCoeff() > 0.1f)
                        clock.freezeSnake();
                    break;
                default: break;
            }
        }

        inputManager.update();
        window.endFrame();
    }

    return 0;
}

void checkPressedKeys(Window& window, Drawer& drawer, Game& game, Clock& clock, InputManager& inputManager) {
    switch (game.status()) {
        case GameStatus::MENU:
            if (inputManager.isKeyPressed(Action::EXIT)) {
                window.close();
            }
            else if (inputManager.isKeyPressed(Action::SPACE)) {
                game.updateStatus(GameStatus::GAME_START);
                inputManager.turnOffBuffer();
                clock.resetGameStepAccumulator();
                clock.freezeSnake();
            }
            break;
        case GameStatus::GAME:
            if (inputManager.isKeyPressed(Action::EXIT)) {
                game.updateStatus(GameStatus::GAME_SURE_EXIT);
                if (!clock.isPauseTime()) {
                    clock.updatePauseStatus();
                    inputManager.changeWorkStatus();
                }
            }
            else if (inputManager.isKeyPressed(Action::PAUSE)) {
                clock.updatePauseStatus();
                inputManager.changeWorkStatus();
            }
            else if (inputManager.isKeyPressed(Action::PRAVEDNOV)) {
                drawer.changePravednovStatus();
            }
            break;
        case GameStatus::GAME_SURE_EXIT:
            if (inputManager.isKeyPressed(Action::YES)) {
                game.updateStatus(GameStatus::MENU);
                game.reset();
                clock.updatePauseStatus();
            }
            else if (inputManager.isKeyPressed(Action::NO)) {
                game.updateStatus(GameStatus::GAME);
                clock.updatePauseStatus();
                inputManager.changeWorkStatus();
            }
            break;
        case GameStatus::GAME_LOOSE:
        case GameStatus::GAME_WIN:
            if (inputManager.isKeyPressed(Action::YES)) {
                game.reset();
                game.updateStatus(GameStatus::GAME_START);
                inputManager.turnOffBuffer();
                clock.resetGameStepAccumulator();
            }
            else if (inputManager.isKeyPressed(Action::NO)) {
                game.reset();
                game.updateStatus(GameStatus::MENU);
                if (clock.isPauseTime())
                    clock.updatePauseStatus();
            }
            break;
        default: break;
    }

    if (inputManager.isKeyPressed(Action::SCALE_UP)) {
        drawer.updateContentScale(true);
        window.updateView(drawer.getViewSize(), drawer.getContentScale(), drawer.getZenMode());
    }
    else if (inputManager.isKeyPressed(Action::SCALE_DOWN)) {
        drawer.updateContentScale(false);
        window.updateView(drawer.getViewSize(), drawer.getContentScale(), drawer.getZenMode());
    }
    else if (inputManager.isKeyPressed(Action::ZEN_MODE)) {
        drawer.updateZenMode();
        window.updateView(drawer.getViewSize(), drawer.getContentScale(), drawer.getZenMode());
    }
}

void updateDir(Snake& snake, InputManager& inputManager) {
    if (!inputManager.isBufferEmpty()) {
        vec2i newDir;
        switch (inputManager.getBufferKey()) {
            case Action::MOVE_UP: newDir = Direction::UP; break;
            case Action::MOVE_DOWN: newDir = Direction::DOWN; break;
            case Action::MOVE_LEFT: newDir = Direction::LEFT; break;
            case Action::MOVE_RIGHT: newDir = Direction::RIGHT; break;
            default: break;
        }

        if (snake.getDirection() + newDir != vec2i{0, 0})
            snake.setDirection(newDir);
    }
}
