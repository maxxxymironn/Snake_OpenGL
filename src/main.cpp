#include "core/clock.hpp"
#include "game/game.hpp"
#include "render/renderer.hpp"
#include "window/window.hpp"

#include <iostream>

void drawField(Renderer& renderer, const Cell& fieldSize,
		const std::array<int, 3>& evenCellColor, const std::array<int, 3>& oddCellColor);
void drawDynamicSnake(Renderer& renderer, const std::vector<Cell>& bodySnake, const Cell& lastTail, const float& alpha);
void drawStaticSnake(Renderer& renderer, const std::vector<Cell>& bodySnake);

void CheckPressedKeys(const Window& window, Game& game, Cell& newSnakeDir);

int main() {
	/* Create main objects */
	Window window(800, 800, false);
	Renderer renderer;
	if(!renderer.getSuccessInfo()) {
		window.terminate();
		return -1;
	}
	Game game;
	/* End Create */

	const Cell fieldSize = game.field().getFieldSize();
	const std::array<int, 3> evenCellColor = game.field().getEvenCellColor();
	const std::array<int, 3> oddCellColor = game.field().getOddCellColor();

	Cell appleLoc;
	Cell newSnakeDir = Direction::RIGHT;	

	bool blink = false;

	renderer.setFieldSize(fieldSize.x, fieldSize.y);

	Clock::startTime();

	while(!window.shouldClose()) {
		Clock::calculateTime();

		/* fps */
		if (Clock::fpsAccumulator >= 1.f) {
			window.setTitle(Clock::frames, game.snake().getLength());
			Clock::fpsAccumulator -= 1.f;
			Clock::frames = 0;
		}

		/* ============== Start render ============== */
		renderer.beginFrame();

		if (game.status() != GameStatus::MENU) {
			drawField(renderer, fieldSize, oddCellColor, evenCellColor);

			// Draw apple
			if (game.apple().isNew()) {
				appleLoc = game.apple().getPosition();
				game.apple().setOld();
			}
			renderer.drawApple(appleLoc.x, appleLoc.y, {150, 0, 0});

			// Draw snake
			if (Clock::secondHolding || game.status() == GameStatus::LOOSE && !blink)
				drawStaticSnake(renderer, game.snake().getBody());
			else if (game.status() == GameStatus::GAME)
				drawDynamicSnake(renderer, game.snake().getBody(), game.snake().getLastTail(), Clock::alpha);
		}

		renderer.endFrame();
		/* =============== End render =============== */

		CheckPressedKeys(window, game, newSnakeDir);
		
		if (game.status() == GameStatus::GAME) {
			if (Clock::secondHolding) {
				if (Clock::gameUpdateAccumulator >= 0.5f) {
					Clock::secondHolding = false;
					Clock::gameUpdateAccumulator = 0.f;
					game.update();
				}
			}
			else {
				while(Clock::gameUpdateAccumulator >= Clock::stepTime) {
					if (game.snake().haveNewDir())
						game.snake().setDirection(newSnakeDir);	
					game.update();
				
					Clock::gameUpdateAccumulator -= Clock::stepTime;
				}
			}
		}
		if (game.status() == GameStatus::LOOSE) { 
			if(Clock::gameUpdateAccumulator >= 0.45f) {
				if (++Clock::counter < 8) 
					blink = !blink;
				else if (Clock::counter > 11) {
					blink = false;
					Clock::counter = 0;
					Clock::secondHolding = true;
					game.reset();
				}

				Clock::gameUpdateAccumulator = 0.f;
			}
		}
		else if (game.status() == GameStatus::WIN) { window.close(); }

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}

void drawField(Renderer& renderer, const Cell& fieldSize,
			const std::array<int, 3>& evenCellColor, const std::array<int, 3>& oddCellColor) {
	float fX;
	float fY;
	for (int y = 0; y < fieldSize.x; ++y) {
		for (int x = 0; x < fieldSize.y; ++x) {
			fX = static_cast<float>(x);
			fY = static_cast<float>(y);
			renderer.drawCell({ fX, fY },
				(x + y) % 2 == 0 ? oddCellColor : evenCellColor
			);
		}
	}
}

void drawDynamicSnake(Renderer& renderer, const std::vector<Cell>& bodySnake, const Cell& lastTail, const float& alpha) {
	float cellX, cellY;
	int colMix = 0;

	auto it = bodySnake.begin() + 1;
	/* static body */
	for (it; it + 1 != bodySnake.end(); ++it) {
		cellX = static_cast<float>((*it).x);
		cellY = static_cast<float>((*it).y);
		renderer.drawCell({cellX, cellY}, {0, 100 + colMix, 0});
		++colMix;
	}

	/* static pre tail */
	cellX = static_cast<float>((*it).x);
	cellY = static_cast<float>((*it).y);
	renderer.drawCell({cellX, cellY}, {0, 100 + colMix, 0});

	/* dynamic tail */
	Cell phantomTail = *it;
	Cell diff = phantomTail - lastTail;
	Cell lastTailNew = lastTail;

	if (diff.x > 1 || diff.x < -1) {
		if (diff.x > 1)
			phantomTail.x = -1;
		else
			phantomTail.x = 20;
	}
	else if (diff.y > 1 || diff.y < -1) {
		if (diff.y > 1)
			phantomTail.y = -1;
		else
			phantomTail.y = 20;
	}

	cellX = static_cast<float>(lastTail.x) + static_cast<float>((phantomTail - lastTail).x) * alpha;
	cellY = static_cast<float>(lastTail.y) + static_cast<float>((phantomTail - lastTail).y) * alpha;
	renderer.drawCell({cellX, cellY}, {0, 100 + colMix, 0});

	/* dynamic head */
	it = bodySnake.begin();
	Cell prev = *(it + 1);

	diff = *it - prev;
	if (diff.x > 1 || diff.x < -1) {
		if (diff.x > 1)
			prev.x = 20;
		else
			prev.x = -1;
	}
	else if (diff.y > 1 || diff.y < -1) {
		if (diff.y > 1)
			prev.y = 20;
		else
			prev.y = -1;
	}
	
	cellX = static_cast<float>(prev.x) + static_cast<float>((*it - prev).x) * alpha;
	cellY = static_cast<float>(prev.y) + static_cast<float>((*it - prev).y) * alpha;
	renderer.drawCell({cellX, cellY}, {0, 100, 0});
}

void drawStaticSnake(Renderer& renderer, const std::vector<Cell>& bodySnake) {
	int colMix = 0;
	for (const Cell& bodyEl : bodySnake) {
		renderer.drawCell({static_cast<float>(bodyEl.x), static_cast<float>(bodyEl.y)}, {0, 100 + colMix, 0});
		++colMix;
	}
}

void CheckPressedKeys(const Window &window, Game &game, Cell &newSnakeDir) {
    if (window.isKeyPressed(GLFW_KEY_ESCAPE)) {
		window.close();
		return;
	}
	else if (game.status() != GameStatus::MENU && window.isKeyPressed(GLFW_KEY_M)) {
		Clock::memorizeAccum();
		game.changeStatus();
	}
	else if (game.status() == GameStatus::MENU && window.isKeyPressed(GLFW_KEY_SPACE)) {
		Clock::restoreAccum();
		game.changeStatus();
	}
	else {
		bool keys[4]{false}; 
		if (window.isKeyPressed(GLFW_KEY_W) || window.isKeyPressed(GLFW_KEY_UP)) keys[0] = true;
		if (window.isKeyPressed(GLFW_KEY_A) || window.isKeyPressed(GLFW_KEY_LEFT)) keys[1] = true;
		if (window.isKeyPressed(GLFW_KEY_S) || window.isKeyPressed(GLFW_KEY_DOWN)) keys[2] = true;
		if (window.isKeyPressed(GLFW_KEY_D) || window.isKeyPressed(GLFW_KEY_RIGHT)) keys[3] = true;

		if (keys[0] || keys[1] || keys[2] || keys[3]) {
			Cell snakeDir = game.snake().getDirection();

			if ((keys[0] || keys[2]) && snakeDir != Direction::UP && snakeDir != Direction::DOWN) {
				if (keys[0]) 
					newSnakeDir = Direction::UP;
				else 
					newSnakeDir = Direction::DOWN;
				game.snake().setHaveNewDir();
			}

			if ((keys[1] || keys[3]) && snakeDir != Direction::LEFT && snakeDir != Direction::RIGHT) {
				if (keys[1]) 
					newSnakeDir = Direction::LEFT;
				else 
					newSnakeDir = Direction::RIGHT;
				game.snake().setHaveNewDir();
			}
		}
	}
}