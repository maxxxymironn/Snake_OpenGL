#include "core/config_manager.hpp"
#include "core/clock.hpp"
#include "core/input.hpp"
#include "game/enums.hpp"
#include "game/snake_parts_enum.hpp"
#include "game/game.hpp"
#include "render/renderer.hpp"
#include "window/window.hpp"

void drawSnake(Renderer& renderer, const Game& game, const float alpha);

void checkPressedKeys(const Window& window, Clock& clock, Game& game, Cell& newSnakeDir);
void checkNewDirection(Game& game, Cell& newSnakeDir);

float getRotateAngle(const Cell first, const Cell second);

int main() {
	ConfigManager& configManager = ConfigManager::getInstance();
	Window window;
	Renderer renderer;
	if(!renderer.getInitializeInfo()) {
		window.terminate();
		return -1;
	}
	Game game;

	// TODO: maybe class Clock should be in composition with class Game
	Clock& clock = Clock::getInstance();

	// often used game info
	Cell applePos = game.apple().getPosition();
	Cell newSnakeDir = Direction::RIGHT;

	const Cell fieldSize = game.field().getFieldSize();
	renderer.setFieldSize(fieldSize.x, fieldSize.y);

	// TODO: replace blink to trigonomrty
	bool blink = false;

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

		checkPressedKeys(window, clock, game, newSnakeDir);
		
		/* Logic part */
		if (game.status() == GameStatus::GAME) {
			while(clock.isUpdateTime()) {
				checkNewDirection(game, newSnakeDir);
				if (game.snake().haveNewDir())
					game.snake().setDirection(newSnakeDir);
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
			// TODO: save stats to file

			if(clock.isUpdateTime(0.45f)) {
				if (++clock.counter < 8) 
					blink = !blink;
				else if (clock.counter > 11) {
					blink = false;
					clock.counter = 0;
					Input::clearBuffer();
					game.reset();
					window.updateScore(false);
				}

				clock.resetGameStepAccumulator();
			}
		}
		else if (game.status() == GameStatus::WIN) {
			window.close();
		}
		else if (game.status() == GameStatus::GAME_START && clock.isUpdateTime(0.5f))  {
			game.changeStatus(GameStatus::GAME);
			game.update();
			clock.resetGameStepAccumulator();
			Input::changeWorkingMode();
		}
		/* End logic */

		Input::update();

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}

void drawSnake(Renderer& renderer, const Game& game, const float alpha) {
	const GameStatus gameStatus = game.status();
	const auto snakeBody = game.snake().getBody();
	float cellX, cellY;
	float rotateAngle;

	const Cell head = snakeBody[0];
	Cell prevHead = snakeBody[1];
	float headRotateAngle = getRotateAngle(head, prevHead);

	if (gameStatus == GameStatus::GAME || gameStatus == GameStatus::PAUSE) {
		const Cell prevTail = game.snake().getPrevTail();
		const Cell fieldSize = game.field().getFieldSize();
		
		/* static body */
		bool flag = true;
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

void checkNewDirection(Game& game, Cell& newSnakeDir) {
	if (Input::bufferIsNotEmpty() && game.snake().canGetNewDir()) {
		switch (Input::getKeyFromBuffer()) {
			case Action::MoveUp:   newSnakeDir = Direction::UP; break;
			case Action::MoveDown: newSnakeDir = Direction::DOWN; break;
			case Action::MoveLeft: newSnakeDir = Direction::LEFT; break;
			case Action::MoveRight:newSnakeDir = Direction::RIGHT; break;
		}

		if (newSnakeDir + game.snake().getDirection() != Cell{0, 0})
			game.snake().setHaveNewDir();
	}
}

void checkPressedKeys(const Window& window, Clock& clock, Game &game, Cell &newSnakeDir) {
	if (Input::isKeyDown(Action::Exit))
		window.close();
	else if (Input::isKeyPressed(Action::Pause)) {
		clock.changePauseMode();
		game.changeStatus();
		Input::changeWorkingMode();
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