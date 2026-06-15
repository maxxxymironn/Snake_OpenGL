#include "config/config_manager.hpp"
#include "core/clock.hpp"
#include "core/input_manager.hpp"
#include "game/enums.hpp"
#include "game/game.hpp"
#include "game/snake_parts_enum.hpp"
#include "render/renderer.hpp"
#include "window/window.hpp"

void checkPressedKeys(const Window& window, Clock& clock, Game& game, InputManager& inputManager);
void updateDir(Snake& snake, InputManager& inputManager);
void drawSnake(Renderer& renderer, const Game& game, const float alpha);
float getRotateAngle(const Cell first, const Cell second);

int main() {
	ConfigManager configManager;
	InputManager inputManager;
	Window window(&inputManager);
	Renderer renderer;
	if(!renderer.getInitializeInfo()) {
		window.terminate();
		return -1;
	}
	Game game;
	Clock clock; // TODO: maybe class Clock should be in composition with class Game

	Cell applePos = game.apple().getPosition();
	bool readyToSaveFile = true;
	
	renderer.setFieldSize(game.field().getFieldSize().x, game.field().getFieldSize().y);

	clock.start();
	while(!window.shouldClose()) {
		clock.calculate();

		if (clock.updateFPS())
			window.updateFPS(clock.getPrevFrames());

		/* Render part */
		renderer.beginFrame();

		renderer.useTextureProgram();
		renderer.drawField();
		renderer.drawApple(applePos.x, applePos.y, clock.getAppleBreathingCoeff());
		drawSnake(renderer, game, clock.getSnakeMovingCoeff());
		if (game.status() == GameStatus::PAUSE)
			renderer.drawPause();

		renderer.endFrame();
		/* End render */

		checkPressedKeys(window, clock, game, inputManager);
		
		/* Logic part */
		if (game.status() == GameStatus::GAME) {
			while(clock.isUpdateTime()) {
				updateDir(game.snake(), inputManager);
				game.update();
			
				clock.updateGameStepAccumulator();

				if (game.apple().isNew()) {
					window.updateScore();
					game.apple().setOld();
					applePos = game.apple().getPosition();
				}
			}
		}
		if (game.status() == GameStatus::LOOSE) {
			if (readyToSaveFile) {
				configManager.saveFile();
				readyToSaveFile = false;
			}
			if (clock.isUpdateTime(3.f)) {
				game.reset();
				window.updateScore(false);
				inputManager.turnOffBuffer();
				clock.resetGameStepAccumulator();
			}
		}
		else if (game.status() == GameStatus::WIN) {
			window.close();
		}
		else if (game.status() == GameStatus::GAME_START && clock.isUpdateTime(0.5f))  {
			readyToSaveFile = true;
			game.updateStatus(GameStatus::GAME);
			game.update();
			clock.resetGameStepAccumulator();
			inputManager.turnOnBuffer();
		}
		/* End logic */

		inputManager.update();

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}

void checkPressedKeys(const Window& window, Clock& clock, Game &game, InputManager& inputManager) {
	if (inputManager.isKeyDown(Action::Exit))
		window.close();
	else if (inputManager.isKeyPressed(Action::Pause)) {
		clock.updatePauseStatus();
		game.updateStatus(GameStatus::PAUSE);
		inputManager.changeWorkStatus();
	}
}

void updateDir(Snake& snake, InputManager& inputManager) {
	if (!inputManager.isBufferEmpty()) {
		Cell newDir;
		switch (inputManager.getBufferKey()) {
			case Action::MoveUp:   newDir = Direction::UP; break;
			case Action::MoveDown: newDir = Direction::DOWN; break;
			case Action::MoveLeft: newDir = Direction::LEFT; break;
			case Action::MoveRight:newDir = Direction::RIGHT; break;
		}

		if (snake.getDirection() + newDir != Cell{0, 0})
			snake.setDirection(newDir);
	}
}

void drawSnake(Renderer& renderer, const Game& game, const float alpha) {
	const GameStatus gameStatus = game.status();
	const auto snakeBody = game.snake().getBody();
	float cellX, cellY;
	float rotateAngle;

	const Cell head = snakeBody[0];
	Cell prevHead = snakeBody[1];

	if (gameStatus == GameStatus::GAME || gameStatus == GameStatus::PAUSE) {
		const Cell prevTail = game.snake().getPrevTail();
		const Cell fieldSize = game.field().getFieldSize();
		
		/* static body */
		for (auto it = snakeBody.rbegin() + 1; it + 1 != snakeBody.rend(); ++it) {
			cellX = static_cast<float>((*it).x);
			cellY = static_cast<float>((*it).y);
			Cell to = *(it + 1) - *it;
			rotateAngle = getRotateAngle(*(it + 1), *it);

			if ((*(it - 1)).y == (*(it + 1)).y || (*(it - 1)).x == (*(it + 1)).x) {
				renderer.drawSnake(cellX, cellY, SnakeType::BODY, rotateAngle);
			} else {
				Cell from = *(it - 1) - *(it);
				Cell dir1 = {1, 0};
				Cell dir2 = {0, -1};
				Cell dir3 = {-1, 0};
				Cell dir4 = {0, 1};
				bool fl = (to == dir1 && from == dir2)
						|| (to == dir2 && from == dir3)
						|| (to == dir3 && from == dir4) 
						|| (to == dir4 && from == dir1);
				if (!fl) {
					renderer.drawSnake(cellX, cellY, SnakeType::CORNER, rotateAngle);
				} else {
					renderer.drawSnake(cellX, cellY, SnakeType::CORNER, rotateAngle - 1.5708f);
				}
			}
		}

		/* dynamic tail */
		Cell tail = *snakeBody.rbegin();

		Cell diff = tail - prevTail;
		// check to move through borders for tail
		if (diff.x > 1 || diff.x < -1) {
			tail.x = (diff.x > 1) ? -1 : fieldSize.x;
		} else if (diff.y > 1 || diff.y < -1) {
			tail.y = (diff.y > 1) ? -1 : fieldSize.y;
		}

		diff = tail - prevTail;
		cellX = static_cast<float>(prevTail.x) + static_cast<float>(diff.x) * alpha;
		cellY = static_cast<float>(prevTail.y) + static_cast<float>(diff.y) * alpha;
		rotateAngle = getRotateAngle(prevTail, tail);
		renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);

		/* dynamic head */
		diff = head - prevHead;
		// check to move through borders for head
		if (diff.x > 1 || diff.x < -1) {
			prevHead.x = (diff.x > 1) ? fieldSize.x : -1;
		} else if (diff.y > 1 || diff.y < -1) {
			prevHead.y = (diff.y > 1) ? fieldSize.y : -1;
		}

		diff = head - prevHead;
		cellX = static_cast<float>(prevHead.x) + static_cast<float>(diff.x) * alpha;
		cellY = static_cast<float>(prevHead.y) + static_cast<float>(diff.y) * alpha;
		rotateAngle = getRotateAngle(head, prevHead);
		renderer.drawSnake(cellX, cellY, SnakeType::HEAD, rotateAngle);
	} 
	else if (gameStatus == GameStatus::LOOSE) {

	}
	else {
		cellX = static_cast<float>(head.x);
		cellY = static_cast<float>(head.y);
		// TODO: use snake direction to set head rotate angle
		rotateAngle = getRotateAngle(head, prevHead);
		renderer.drawSnake(cellX, cellY, SnakeType::HEAD, rotateAngle);

		for (const Cell& bodyEl : snakeBody) {
			cellX = static_cast<float>(bodyEl.x);
			cellY = static_cast<float>(bodyEl.y);
			renderer.drawSnake(cellX, cellY, SnakeType::BODY, 0);
		}
	}
}

float getRotateAngle(const Cell first, const Cell second) {
	float piDiv2 = 1.5708;
	float pi = 3.1416;
	Cell left = {-1, 0};
	Cell up = {0, 1};
	Cell down = {0, -1};

	Cell diff = first - second;
	if (diff == up) {
		return piDiv2;
	} else if (diff == left) {
		return pi;
	} else if (diff == down) {
		return -piDiv2;
	}
	return 0;
}