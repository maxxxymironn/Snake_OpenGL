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
void updateCellPosition(const Cell diff, const Cell fieldSize, Cell& toUpdate);
Cell getShift(const Cell diff, const Cell fieldSize);

void draw(Renderer& renderer, const Game& game, const Clock& clock);

int main() {
	ConfigManager configManager;
	InputManager inputManager;
	Window window;
	Renderer renderer;
	if(!renderer.getInitializeInfo()) {
		window.terminate();
		return -1;
	}
	Game game;
	Clock clock; // TODO: maybe class Clock should be in composition with class Game

	// Cell applePos = game.apple().getPosition();
	
	renderer.setFieldSize(game.field().getFieldSize().x, game.field().getFieldSize().y);

	window.setInputManagerSetKey([&inputManager](Action a, bool b) { inputManager.setKey(a, b); });
	window.setRefreshCallback([&renderer, &game, &clock]() {	draw(renderer, game, clock); });

	clock.start();
	while(!window.shouldClose()) {
		clock.calculate();

		if (clock.updateFPS())
			window.updateFPS(clock.getPrevFrames());

		draw(renderer, game, clock);

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
					// applePos = game.apple().getPosition();
				}
			}
		}
		if (game.status() == GameStatus::LOOSE) {
			if (configManager.isReadyToSaveFile()) {
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
		else if (game.status() == GameStatus::WIN) {
			window.close();
		}
		else if (game.status() == GameStatus::GAME_START) {
			if (game.apple().isNew()) {
				window.updateScore();
				game.apple().setOld();
				// applePos = game.apple().getPosition();
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
			default: break;
		}

		if (snake.getDirection() + newDir != Cell{0, 0})
			snake.setDirection(newDir);
	}
}

void drawSnake(Renderer& renderer, const Game& game, float alpha) {
	const GameStatus gameStatus = game.status();
	const auto snakeBody = game.snake().getBody();
	float cellX, cellY;
	float rotateAngle;

	if (gameStatus == GameStatus::LOOSE) {

	}
	if (gameStatus != GameStatus::MENU) {
		const Cell fieldSize = game.field().getFieldSize();

		bool throughBorder;

		/* static body */
		for (auto it = snakeBody.rbegin(); it + 1 != snakeBody.rend(); ++it) {
			Cell prevCell;
			if (it == snakeBody.rbegin())
				prevCell = game.snake().getPrevTail();
			else
				prevCell = *(it - 1);

			Cell nextCell = *(it + 1);
			Cell curCell = *it;
			
			cellX = static_cast<float>(curCell.x);
			cellY = static_cast<float>(curCell.y);

			// без изгиба
			if (prevCell.x == nextCell.x || prevCell.y == nextCell.y) {
				rotateAngle = getRotateAngle(curCell, nextCell);
				if (it == snakeBody.rbegin() && alpha > 0.5f) {
					renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);
				} else if (it + 2 == snakeBody.rend() && alpha < 0.5f){
					rotateAngle = getRotateAngle(nextCell, curCell);
					renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);
				} else {
					renderer.drawSnake(cellX, cellY, SnakeType::BODY, rotateAngle);
				}
			} 
			// с изгибом
			else {
				// Определяем направление по следующему элементу тела
				Cell to = nextCell - curCell;
				throughBorder = to.x > 1 || to.x < -1 || to.y > 1 || to.y < -1;
				if (throughBorder) {
					updateCellPosition(to, fieldSize, nextCell);
					to = nextCell - curCell;
				}
				rotateAngle = getRotateAngle(nextCell, curCell);

				// Определяем направление по предыдущему элементу тела
				Cell from = prevCell - curCell;
				throughBorder = from.x > 1 || from.x < -1 || from.y > 1 || from.y < -1;
				if (throughBorder) {
					updateCellPosition(from, fieldSize, prevCell);
					from = prevCell - curCell;
				}

				bool isClockWiseDir = (to == Direction::RIGHT && from == Direction::DOWN)
					   			   || (to == Direction::DOWN && from == Direction::LEFT)
								   || (to == Direction::LEFT && from == Direction::UP) 
								   || (to == Direction::UP && from == Direction::RIGHT);
				if (isClockWiseDir)
					rotateAngle -= 1.5708f;
				renderer.drawSnake(cellX, cellY, SnakeType::CORNER, rotateAngle);
			}
		}

		/* ====================== dynamic tail ====================== */
		Cell tail = *snakeBody.rbegin();
		Cell prevTail = game.snake().getPrevTail();

		// prevTail = tail;	// <-- delete this to fix static tail
		Cell diff = tail - prevTail;
		Cell shift = getShift(diff, fieldSize);

		throughBorder = diff.x > 1 || diff.x < -1 || diff.y > 1 || diff.y < -1;
		if (throughBorder) {
			updateCellPosition(diff, fieldSize, tail);
			diff = tail - prevTail;
		}
		cellX = static_cast<float>(prevTail.x) + static_cast<float>(diff.x) * alpha;
		cellY = static_cast<float>(prevTail.y) + static_cast<float>(diff.y) * alpha;

		const bool isAppleEaten = diff == Cell{0, 0};
		if (isAppleEaten) {
			prevTail = *(snakeBody.rbegin() + 1);
			std::swap(tail, prevTail);
		}

		rotateAngle = getRotateAngle(tail, prevTail) + 3.1416f;

		renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);
		if (throughBorder)
			renderer.drawSnake(cellX + shift.x, cellY + shift.y, SnakeType::TAIL, rotateAngle);

		/* ====================== dynamic head ====================== */
		Cell head = *snakeBody.begin();
		Cell prevHead = *(snakeBody.begin() + 1);

		diff = head - prevHead;
		shift = getShift(diff, fieldSize);

		throughBorder = diff.x > 1 || diff.x < -1 || diff.y > 1 || diff.y < -1;
		if (throughBorder) {
			updateCellPosition(diff, fieldSize, head);
			diff = head - prevHead;
		}

		/* ====================== corner beetwen head and tail ====================== */
		// if (snakeBody.size() == 2) {
		// 	if (prevTail.x != head.x && prevTail.y != head.y) {
		// 		rotateAngle = getRotateAngle(head, tail);
		// 		cellX = static_cast<float>(tail.x);
		// 		cellY = static_cast<float>(tail.y);

		// 		Cell to = head - tail;
		// 		Cell from = prevTail - tail;
		// 		bool isClockWiseDir = (to == Direction::RIGHT && from == Direction::DOWN)
		// 			   			   || (to == Direction::DOWN && from == Direction::LEFT)
		// 						   || (to == Direction::LEFT && from == Direction::UP) 
		// 						   || (to == Direction::UP && from == Direction::RIGHT);
		// 		if (isClockWiseDir)
		// 			rotateAngle -= 1.5708f;
		// 		renderer.drawSnake(cellX, cellY, SnakeType::CORNER, rotateAngle);
		// 	}
		// }
		/* ============================================ */

		cellX = static_cast<float>(prevHead.x) + static_cast<float>(diff.x) * alpha;
		cellY = static_cast<float>(prevHead.y) + static_cast<float>(diff.y) * alpha;

		rotateAngle = getRotateAngle(head, prevHead);
		
		renderer.drawSnake(cellX, cellY, SnakeType::HEAD, rotateAngle);
		if (throughBorder)
			renderer.drawSnake(cellX + shift.x, cellY + shift.y, SnakeType::HEAD, rotateAngle);
	}
}

float getRotateAngle(const Cell first, const Cell second) {
	float piDiv2 = 1.5708;
	float pi = 3.1416;

	Cell diff = first - second;
	if (diff.y > 0) {
		return piDiv2;
	} else if (diff.x < 0) {
		return pi;
	} else if (diff.y < 0) {
		return -piDiv2;
	}
	return 0;
}

void updateCellPosition(const Cell diff, const Cell fieldSize, Cell& toUpdate) {
	if (diff.x > 1 || diff.x < -1)
		toUpdate.x = (diff.x > 1) ? -1 : fieldSize.x;
	else
		toUpdate.y = (diff.y > 1) ? -1 : fieldSize.y;
}

Cell getShift(const Cell diff, const Cell fieldSize) {
	Cell shift{0, 0};

	if (diff.x > 1 || diff.x < -1)
		shift.x = (diff.x > 1) ? fieldSize.x : -fieldSize.x;
	else
		shift.y = (diff.y > 1) ? fieldSize.y : -fieldSize.y;

	return shift;
}

void draw(Renderer& renderer, const Game& game, const Clock& clock) {
	renderer.beginFrame();
	renderer.useTextureProgram();

	renderer.drawField();
	Cell applePos = game.apple().getPosition();
	renderer.drawApple(applePos.x, applePos.y, clock.getAppleBreathingCoeff());
	drawSnake(renderer, game, clock.getSnakeMovingCoeff());
	if (game.status() == GameStatus::PAUSE)
		renderer.drawPause();

	renderer.endFrame();
}