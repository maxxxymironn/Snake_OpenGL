#include "config/config_manager.hpp"
#include "core/clock.hpp"
#include "core/input_manager.hpp"
#include "game/enums.hpp"
#include "game/game.hpp"
#include "game/snake.hpp"
#include "render/texture_enum.hpp"
#include "render/renderer.hpp"
#include "window/window.hpp"

void checkPressedKeys(Window& window, Renderer& renderer, Clock& clock, Game& game, InputManager& inputManager);
void updateDir(Snake& snake, InputManager& inputManager);
float getRotateAngle(const vec2i v1, const vec2i v2);

void draw(Renderer& renderer, const Game& game, const Clock& clock);

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
	Clock clock; // TODO: maybe class Clock should be in composition with class Game
	
	window.setInputManagerSetKey(
		[&inputManager](Action actionKey, bool isPressed, Mod mods) {
			inputManager.setKey(actionKey, isPressed, mods);
		}
	);
	window.setRefreshCallback([&renderer, &game, &clock]() { draw(renderer, game, clock); });

	window.updateFieldSize(static_cast<vec2i>(game.field().getFieldSize()));
	renderer.setContentScale(window.getContentScale());
	renderer.setViewSize(window.getViewSize());

	clock.start();
	while(!window.shouldClose()) {
		clock.calculate();

		if (clock.updateFPS())
			window.updateFPS(clock.getPrevFrames());

		draw(renderer, game, clock);

		checkPressedKeys(window, renderer, clock, game, inputManager);
		
		/* Logic part */
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
		if (game.status() == GameStatus::LOOSE) {
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

void checkPressedKeys(Window& window, Renderer& renderer, Clock& clock, Game &game, InputManager& inputManager) {
	if (inputManager.isKeyDown(Action::Exit))
		window.close();

	else if (inputManager.isKeyPressed(Action::Pause)) {
		clock.updatePauseStatus();
		game.updateStatus(GameStatus::PAUSE);
		inputManager.changeWorkStatus();
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

	vec2f size{};
	vec2f pos{};
	vec2f texCoord{};
	vec4f color{};
	float rotateAngle = 0.f;

	bool updatedStaticData = false;

	/* ###### static objects ###### */
	if (renderer.getNeedUpdateStatic()) {
		renderer.setStaticMode(true);
		renderer.setOrigin(vec2f{ 0.f, 0.f });

		// =================== field
		constexpr vec2f minViewSize{1200.f, 800.f};

		size = static_cast<vec2f>(game.field().getFieldSize()) * cellSize;
		pos = (renderer.isZenMode() ? vec2f{ 0.f, 0.f } : ((minViewSize - size) * 0.5f)) + size * 0.5f;
		texCoord = size / (cellSize * 2.f);
		color = {0.2, 0.2, 0.2, 1.f};
		renderer.addObject(
			size, pos,
			TexType::FIELD,
			texCoord, color, rotateAngle
		);
		const vec2f fieldPos = pos - size * 0.5f;

		if (!renderer.isZenMode()) {
			// =================== right panel
			constexpr float minRightPanelX = 1000.f;
			float rightPanelX = fieldPos.x + size.x * 0.5f;

			size = { 200.f, 800.f };
			pos = vec2f{ rightPanelX < minRightPanelX ? minRightPanelX : rightPanelX, 0.f } + size * 0.5f;
			texCoord = 1.f;
			color = {0.15f, 0.15f, 0.15f, 1.f};
			renderer.addObject(
				size, pos,
				TexType::SNAKE_BODY,
				texCoord, color, rotateAngle
			);

			// ~~~~~~~~~~~~~~~~~~~ icons on the right panel

			// =================== left panel
			size = {200.f, 800.f};
			pos = vec2f{0.f, 0.f } + size * 0.5f;
			texCoord = 1.f;
			color = {0.15f, 0.15f, 0.15f, 1.f};
			renderer.addObject(
				size, pos,
				TexType::SNAKE_BODY,
				texCoord, color, rotateAngle
			);

			// ~~~~~~~~~~~~~~~~~~~ icons on the left panel
		}
		
		updatedStaticData = true;
		renderer.setOrigin(fieldPos);
		renderer.setStaticMode(false);
	}

	/* ###### dynamic objects ###### */
	size = vec2f{cellSize, cellSize};

	// =================== Snake body
	color = {0.15f, 0.4f, 0.2f, 1.f};
	texCoord = 1.f;

	std::vector<vec2i> snakeBody = game.snake().getBody();
	vec2i head = *snakeBody.begin();

	static vec2i lastHead(*(snakeBody.begin() + 1));
	if (head != lastHead || updatedStaticData) {
		lastHead = head;
		for (auto it = snakeBody.rbegin() + 1; it + 1 != snakeBody.rend(); ++it) {
			pos = static_cast<vec2f>(*it) * cellSize + size * 0.5f;
			rotateAngle = getRotateAngle(*(it + 1), *it);
			TexType texType = TexType::SNAKE_BODY;

			vec2i prev = *(it - 1);
			vec2i next = *(it + 1);
			vec2i diff = next - prev;
			if (diff.x != 0 && diff.y != 0) {
				vec2i from = prev - *it;
				vec2i to = next - *it;

				if (from.x < -1 || from.x > 1) 
					from.x = from.x < -1 ? 1 : -1;
				else if (from.y < -1 || from.y > 1)
					from.y = from.y < -1 ? 1 : -1;

				if (to.x < -1 || to.x > 1)
					to.x = to.x < -1 ? 1 : -1;
				else if (to.y < -1 || to.y > 1)
					to.y = to.y < -1 ? 1 : -1;

				bool counterclockwise = (from == vec2i{ 1, 0 } && to == vec2i{ 0,-1 }) ||
										(from == vec2i{ 0, 1 } && to == vec2i{ 1, 0 }) ||
										(from == vec2i{-1, 0 } && to == vec2i{ 0, 1 }) ||
										(from == vec2i{ 0,-1 } && to == vec2i{-1, 0 });
				if (counterclockwise)
					rotateAngle = rotateAngle + 3.14159265359f / 2.f;

				texType = TexType::SNAKE_CORNER;
			}

			renderer.addObject(
				size, pos, texType,
				texCoord, color, rotateAngle
			);
		}

		renderer.saveSemistaticIndices();
	}

	// =================== apple
	pos = static_cast<vec2f>(game.apple().getPosition()) * cellSize + size * 0.5f;
	texCoord = { 1.f, 1.f };
	color = {1.f, 1.f, 1.f, 1.f};
	rotateAngle = 0.f;
	renderer.addObject(
		size * clock.getAppleBreathingCoeff(), pos,
		TexType::APPLE,
		texCoord, color, rotateAngle
	);

	// =================== Snake head
	color = {0.15f, 0.4f, 0.2f, 1.f};
	texCoord = 1.f;

	vec2i direction = game.snake().getDirection();
	vec2i prevHead = *(snakeBody.begin() + 1);
	vec2f alpha = static_cast<vec2f>(direction) * clock.getSnakeMovingCoeff();

	rotateAngle = getRotateAngle(head + direction, head);
	pos = (static_cast<vec2f>(prevHead) + alpha) * cellSize + size * 0.5f;
	renderer.addObject(
		size, pos, TexType::SNAKE_TAIL,
		texCoord, color, rotateAngle
	);
	
	// =================== Snake tail
	vec2i tail = *snakeBody.rbegin();
	vec2i preTail = *(snakeBody.rbegin() + 1);
	vec2i prevTail = game.snake().getPrevTail();

	alpha = static_cast<vec2f>(tail - prevTail) * clock.getSnakeMovingCoeff();
	rotateAngle = getRotateAngle(prevTail, tail);
	pos = (static_cast<vec2f>(prevTail) + alpha) * cellSize + size * 0.5f;
	renderer.addObject(
		size, pos, TexType::SNAKE_TAIL,
		texCoord, color, rotateAngle
	);

	// // drawSnake(renderer, game, clock.getSnakeMovingCoeff());
	// // if  (game.status() == GameStatus::PAUSE)
	// // 	renderer.drawPause();

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

// void drawSnake(Renderer& renderer, const Game& game, float alpha) {
// 	const GameStatus gameStatus = game.status();
// 	const auto snakeBody = game.snake().getBody();
// 	float cellX, cellY;
// 	float rotateAngle;

// 	if (gameStatus == GameStatus::LOOSE) {

// 	}
// 	if (gameStatus != GameStatus::MENU) {
// 		const Cell fieldSize = game.field().getFieldSize();

// 		bool throughBorder;
// 		bool specificTailDraw = false;
// 		bool specificHeadDraw = false;
// 		bool tailThroughBorder = false;
// 		bool headThroughBorder = false;

// 		Cell prevCell = game.snake().getPrevTail();Dire
// 		Cell nextCell = *(snakeBody.rbegin() + 1);

// 		if (prevCell.x != nextCell.x && prevCell.y != nextCell.y)
// 			specificTailDraw = true;

// 		/* ====================== dynamic tail ====================== */
// 		Cell tail = *snakeBody.rbegin();
// 		Cell prevTail = game.snake().getPrevTail();

// 		// prevTail = tail;	// <-- delete this to fix static tail
// 		Cell diff = tail - prevTail;
// 		Cell shift = getShift(diff, fieldSize);

// 		throughBorder = diff.x > 1 || diff.x < -1 || diff.y > 1 || diff.y < -1;
// 		if (throughBorder) {
// 			updateCellPosition(diff, fieldSize, tail);
// 			diff = tail - prevTail;
// 			tailThroughBorder = true;
// 		}
// 		cellX = static_cast<float>(prevTail.x) + static_cast<float>(diff.x) * alpha;
// 		cellY = static_cast<float>(prevTail.y) + static_cast<float>(diff.y) * alpha;

// 		const bool isAppleEaten = diff == Cell{0, 0};
// 		if (isAppleEaten) {
// 			prevTail = *(snakeBody.rbegin() + 1);
// 			std::swap(tail, prevTail);
// 		}

// 		rotateAngle = getRotateAngle(tail, prevTail) + 3.1416f;

// 		if (specificTailDraw) { 	// shift needed for 
// 			if (alpha < 0.5f) {
// 				renderer.drawSnake(cellX + shift.x, cellY + shift.y, SnakeType::TAIL, rotateAngle);
// 			} else if (alpha < 0.875f) {
// 				renderer.drawSnake(cellX + shift.x, cellY + shift.y, SnakeType::TAIL_TAIL, rotateAngle);
// 			} else {
// 				rotateAngle += 3.14159f;
// 				renderer.drawSnake(cellX + shift.x, cellY + shift.y, SnakeType::CAP, rotateAngle);
// 			}
// 		} else
// 			renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);
// 		if (throughBorder && !specificTailDraw)
// 			renderer.drawSnake(cellX + shift.x, cellY + shift.y, SnakeType::TAIL, rotateAngle);
// 		else if (throughBorder && specificTailDraw)
// 			renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);

// 		/* ====================== dynamic head ====================== */
// 		Cell head = *snakeBody.begin();
// 		Cell prevHead = *(snakeBody.begin() + 1);

// 		nextCell = head;
// 		auto checkCell = snakeBody.begin() + 1;
// 		if (checkCell + 1 == snakeBody.end())
// 			prevCell = game.snake().getPrevTail();
// 		else
// 			prevCell = *(checkCell + 1);

// 		if (prevCell.x != nextCell.x && prevCell.y != nextCell.y)
// 			specificHeadDraw = true;

// 		diff = head - prevHead;
// 		shift = getShift(diff, fieldSize);

// 		throughBorder = diff.x > 1 || diff.x < -1 || diff.y > 1 || diff.y < -1;
// 		if (throughBorder) {
// 			updateCellPosition(diff, fieldSize, head);
// 			diff = head - prevHead;
// 			headThroughBorder = true;

// 		}

// 		cellX = static_cast<float>(prevHead.x) + static_cast<float>(diff.x) * alpha;
// 		cellY = static_cast<float>(prevHead.y) + static_cast<float>(diff.y) * alpha;
// 		float eyeX = cellX;
// 		float eyeY = cellY;

// 		rotateAngle = getRotateAngle(head, prevHead);
// 		float eyeRotateAngle = rotateAngle;
		
// 		if (specificHeadDraw) {
// 			if (alpha < 0.125f) {
// 				rotateAngle += 3.14159f;
// 				renderer.drawSnake(cellX, cellY, SnakeType::CAP, rotateAngle);
// 			} else if (alpha < 0.5f) {
// 				renderer.drawSnake(cellX, cellY, SnakeType::TAIL_TAIL, rotateAngle);
// 			} else {
// 				renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);
// 			}
// 		} else
// 			renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);
// 		if (throughBorder)
// 			renderer.drawSnake(cellX + shift.x, cellY + shift.y, SnakeType::TAIL, rotateAngle);
// 		/* ================================== */

// 		/* static body */
// 		for (auto it = snakeBody.rbegin(); it + 1 != snakeBody.rend(); ++it) {
// 			Cell prevCell;
// 			if (it == snakeBody.rbegin())
// 				prevCell = game.snake().getPrevTail();
// 			else
// 				prevCell = *(it - 1);

// 			Cell nextCell = *(it + 1);
// 			Cell curCell = *it;
			
// 			cellX = static_cast<float>(curCell.x);
// 			cellY = static_cast<float>(curCell.y);

// 			// без изгиба
// 			if (prevCell.x == nextCell.x || prevCell.y == nextCell.y) {
// 				rotateAngle = getRotateAngle(curCell, nextCell);
// 				if (it + 2 == snakeBody.rend() && alpha < 0.5f){
// 					rotateAngle = getRotateAngle(nextCell, curCell);
// 					if (headThroughBorder)
// 						rotateAngle += 3.14159f;
// 					renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);
// 				}
// 				else if (it == snakeBody.rbegin() && alpha > 0.5f) {
// 					Cell diff = *(it+1) - *it;
// 					if (abs(diff.x) > 1 || abs(diff.y) > 1)
// 						rotateAngle += 3.14159f;
// 					renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);
// 				}  
// 				else if (*it == game.snake().getPrevTail()) {
// 					renderer.drawSnake(cellX, cellY, SnakeType::TAIL, rotateAngle);
// 				} 
// 				else {
// 					renderer.drawSnake(cellX, cellY, SnakeType::BODY, rotateAngle);
// 				}
// 			} 
// 			// с изгибом хвоста
// 			else if (prevCell == game.snake().getPrevTail() && snakeBody.back() != game.snake().getPrevTail()) {
// 				specificTailDraw = true;
// 				// Определяем направление по следующему элементу тела
// 				Cell to = nextCell - curCell;
// 				throughBorder = to.x > 1 || to.x < -1 || to.y > 1 || to.y < -1;
// 				if (throughBorder) {
// 					updateCellPosition(to, fieldSize, nextCell);
// 					to = nextCell - curCell;
// 				}
// 				rotateAngle = getRotateAngle(nextCell, curCell);

// 				// Определяем направление по предыдущему элементу тела
// 				Cell from = prevCell - curCell;
// 				throughBorder = from.x > 1 || from.x < -1 || from.y > 1 || from.y < -1;
// 				if (throughBorder) {
// 					updateCellPosition(from, fieldSize, prevCell);
// 					from = prevCell - curCell;
// 				}
// 				bool isClockWiseDir = (to == Direction::RIGHT && from == Direction::DOWN)
// 					   			   || (to == Direction::DOWN && from == Direction::LEFT)
// 								   || (to == Direction::LEFT && from == Direction::UP) 
// 								   || (to == Direction::UP && from == Direction::RIGHT);
// 				if (alpha < 0.7) {
// 					if (nextCell == *snakeBody.begin()) {
// 						if (alpha < 0.34) {
// 							if (isClockWiseDir)
// 								rotateAngle -= 1.5708f;
// 							else
// 								rotateAngle += 1.5708f;
// 							renderer.drawSnake(cellX, cellY, SnakeType::TAIL_CORNER, rotateAngle);
// 						} else {
// 							if (isClockWiseDir)
// 								rotateAngle -= 1.5708f;
// 							renderer.drawSnake(cellX, cellY, SnakeType::CORNER, rotateAngle);
// 						}
// 					} else {
// 						if (isClockWiseDir)
// 							rotateAngle -= 1.5708f;
// 						renderer.drawSnake(cellX, cellY, SnakeType::CORNER, rotateAngle);
// 					}
// 				} else {
// 					renderer.drawSnake(cellX, cellY, SnakeType::TAIL_CORNER, rotateAngle);
// 				}
// 			}
// 			// с изгибом
// 			else if (nextCell != *snakeBody.begin()) {
// 				// Определяем направление по следующему элементу тела
// 				Cell to = nextCell - curCell;
// 				throughBorder = to.x > 1 || to.x < -1 || to.y > 1 || to.y < -1;
// 				if (throughBorder) {
// 					updateCellPosition(to, fieldSize, nextCell);
// 					to = nextCell - curCell;
// 				}
// 				rotateAngle = getRotateAngle(nextCell, curCell);

// 				// Определяем направление по предыдущему элементу тела
// 				Cell from = prevCell - curCell;
// 				throughBorder = from.x > 1 || from.x < -1 || from.y > 1 || from.y < -1;
// 				if (throughBorder) {
// 					updateCellPosition(from, fieldSize, prevCell);
// 					from = prevCell - curCell;
// 				}

// 				bool isClockWiseDir = (to == Direction::RIGHT && from == Direction::DOWN)
// 					   			   || (to == Direction::DOWN && from == Direction::LEFT)
// 								   || (to == Direction::LEFT && from == Direction::UP) 
// 								   || (to == Direction::UP && from == Direction::RIGHT);
// 				if (isClockWiseDir)
// 					rotateAngle -= 1.5708f;
// 				renderer.drawSnake(cellX, cellY, SnakeType::CORNER, rotateAngle);
// 			}
// 			// с изгибом головы?
// 			else {
// 				// Определяем направление по следующему элементу тела
// 				Cell to = nextCell - curCell;
// 				throughBorder = to.x > 1 || to.x < -1 || to.y > 1 || to.y < -1;
// 				if (throughBorder) {
// 					updateCellPosition(to, fieldSize, nextCell);
// 					to = nextCell - curCell;
// 				}
// 				rotateAngle = getRotateAngle(nextCell, curCell);

// 				// Определяем направление по предыдущему элементу тела
// 				Cell from = prevCell - curCell;
// 				throughBorder = from.x > 1 || from.x < -1 || from.y > 1 || from.y < -1;
// 				if (throughBorder) {
// 					updateCellPosition(from, fieldSize, prevCell);
// 					from = prevCell - curCell;
// 				}

// 				bool isClockWiseDir = (to == Direction::RIGHT && from == Direction::DOWN)
// 					   			   || (to == Direction::DOWN && from == Direction::LEFT)
// 								   || (to == Direction::LEFT && from == Direction::UP) 
// 								   || (to == Direction::UP && from == Direction::RIGHT);
				
// 				if (alpha < 0.34) {
// 					if (isClockWiseDir)
// 						rotateAngle -= 1.5708f;
// 					else
// 						rotateAngle += 1.5708f;
// 					renderer.drawSnake(cellX, cellY, SnakeType::TAIL_CORNER, rotateAngle);
// 				} else {
// 					if (isClockWiseDir)
// 						rotateAngle -= 1.5708f;
// 					renderer.drawSnake(cellX, cellY, SnakeType::CORNER, rotateAngle);
// 				}
// 			}
// 		}

// 		Cell applePos = game.apple().getPosition();
// 		float yPart = eyeY - static_cast<float>(applePos.y);
// 		float xPart = eyeX - static_cast<float>(applePos.x);
// 		float eyePointAngle = std::atan2(yPart, xPart);
		
// 		Cell dir = game.snake().getDirection();
// 		if (dir.x > 0) {
// 			eyePointAngle += M_PIf;
// 		} else if (dir.y > 0) {
// 			eyePointAngle += M_PI_2f;
// 		} else if (dir.y < 0) {
// 			eyePointAngle -= M_PI_2f;
// 		}

// 		static bool isSnakeDead = false;
// 		if (!isSnakeDead && gameStatus == GameStatus::LOOSE)
// 			isSnakeDead = true;
// 		else if (gameStatus == GameStatus::GAME_START && isSnakeDead) {
// 			isSnakeDead = false;
// 		}
// 		renderer.drawEyes(eyeX, eyeY, eyeRotateAngle, eyePointAngle, isSnakeDead);
// 	}
// }

// float getRotateAngle(const Cell first, const Cell second) {
// 	float piDiv2 = 1.5708;
// 	float pi = 3.1416;

// 	Cell diff = first - second;
// 	if (diff.y > 0) {
// 		return piDiv2;
// 	} else if (diff.x < 0) {
// 		return pi;
// 	} else if (diff.y < 0) {
// 		return -piDiv2;
// 	}
// 	return 0;
// }

// void updateCellPosition(const Cell diff, const Cell fieldSize, Cell& toUpdate) {
// 	if (diff.x > 1 || diff.x < -1)
// 		toUpdate.x = (diff.x > 1) ? -1 : fieldSize.x;
// 	else
// 		toUpdate.y = (diff.y > 1) ? -1 : fieldSize.y;
// }

// Cell getShift(const Cell diff, const Cell fieldSize) {
// 	Cell shift{0, 0};

// 	if (diff.x > 1 || diff.x < -1)
// 		shift.x = (diff.x > 1) ? fieldSize.x : -fieldSize.x;
// 	else if (diff.y > 1 || diff.y < -1)
// 		shift.y = (diff.y > 1) ? fieldSize.y : -fieldSize.y;

// 	return shift;
// }