#include "config/config_manager.hpp"
#include "core/clock.hpp"
#include "core/input_manager.hpp"
#include "core/vector.hpp"
#include "game/enums.hpp"
#include "game/game.hpp"
#include "game/snake.hpp"
#include "render/texture_enum.hpp"
#include "render/renderer.hpp"
#include "window/window.hpp"

#include <cstdlib>
#include <iostream>

void checkPressedKeys(Window& window, Renderer& renderer, Clock& clock, Game& game, InputManager& inputManager);
void updateDir(Snake& snake, InputManager& inputManager);
void draw(Renderer& renderer, const Game& game, const Clock& clock);
float getRotateAngle(const vec2i v1, const vec2i v2);

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
	const vec2f fieldSize = static_cast<vec2f>(game.field().getFieldSize()) * cellSize;

	vec2f size{};
	vec2f pos{};
	vec4f texCoord(0.f, 0.f, 1.f, 1.f);
	vec4f color{};
	float rotateAngle = 0.f;

	bool updatedStaticData = false;
	/* ###### static objects ###### */
	if (renderer.needRefreshStaticBuffer()) {
		renderer.setOrigin(vec2f{ 0.f, 0.f });

		// =================== field
		size = cellSize * 2.f;
		color = {0.2, 0.2, 0.2, 1.f};

		constexpr vec2f minViewSize{1200.f, 800.f};

		vec2i rawFieldSize(game.field().getFieldSize());

		vec2f startPos(0.f, 0.f);
		if (!renderer.isZenMode()) {
			startPos.x = fieldSize.x < minViewSize.x - 400.f ? (minViewSize.x - fieldSize.x) * 0.5f : 200.f;
			startPos.y = fieldSize.y < minViewSize.y ? (minViewSize.y - fieldSize.y) * 0.5f : 0.f;
		}
		startPos += size * 0.5f;
		const vec2f fieldStartPos = startPos - size * 0.5f;

		bool evenXCellsCount = rawFieldSize.x % 2 == 0;
		bool evenYCellsCount = rawFieldSize.y % 2 == 0;
		vec2i fieldCellsCount(
			(rawFieldSize.x + (evenXCellsCount ? 0 : 1)) / 2,
			(rawFieldSize.y + (evenYCellsCount ? 0 : 1)) / 2
		);
		for (int x = 0; x < fieldCellsCount.x; ++x) {
			for (int y = 0; y < fieldCellsCount.y; ++y) {
				pos = startPos + vec2f{ size.x * x, size.y * y }; 
				renderer.addObject(
					size, pos,
					TexType::FIELD,
					texCoord, color, rotateAngle
				);
			}
		}

		if (!evenXCellsCount || !evenYCellsCount) {
			vec2f hiddenLinePos = fieldStartPos + fieldSize + vec2f(cellSize) * 0.5f;
			color = vec4f{0.1f, 0.1f, 0.1f, 1.f};

			// ~~~~~~~~~~~~~~~~~~~ right hidden line
			size = { cellSize, fieldSize.y + cellSize };
			pos = { hiddenLinePos.x, hiddenLinePos.y - fieldSize.y * 0.5f };
			renderer.addObject(
				size, pos,
				TexType::FIELD,
				{0.1f, 0.1f}, color, rotateAngle
			);

			// ~~~~~~~~~~~~~~~~~~~ left hidden line
			size = { fieldSize.x + cellSize, cellSize };
			pos = { hiddenLinePos.x - fieldSize.x * 0.5f, hiddenLinePos.y };
			renderer.addObject(
				size, pos,
				TexType::FIELD,
				{0.1f, 0.1f}, color, rotateAngle
			);
		}
		
		if (!renderer.isZenMode()) {
			size = { 200.f, 800.f };
			color = {0.15f, 0.15f, 0.15f, 1.f};

			// =================== left panel
			pos = vec2f{0.f, (fieldSize.y < minViewSize.y ? 0.f : (fieldSize.y - size.y) * 0.5f) } + size * 0.5f;
			renderer.addObject(
				size, pos,
				TexType::SNAKE_BODY,
				texCoord, color, rotateAngle
			);

			// ~~~~~~~~~~~~~~~~~~~ icons on the left panel

			// =================== right panel
			constexpr float minRightPanelX = 1000.f;
			float rightPanelX = fieldStartPos.x + fieldSize.x;
			
			pos.x = (rightPanelX < minRightPanelX ? minRightPanelX : rightPanelX) + size.x * 0.5f;
			renderer.addObject(
				size, pos,
				TexType::SNAKE_BODY,
				texCoord, color, rotateAngle
			);

			// ~~~~~~~~~~~~~~~~~~~ icons on the right panel
		}
		updatedStaticData = true;
		renderer.setOrigin(fieldStartPos);
		renderer.refreshStaticBuffer();
	}

	size = cellSize;
	color = {0.15f, 0.4f, 0.2f, 1.f};

	TexType texType{};

	std::vector<vec2i> snakeBody = game.snake().getBody();
	const vec2i head = snakeBody.front();
	const vec2i tail = snakeBody.back();
	const vec2i direction = game.snake().getDirection();
	const vec2i applePos = game.apple().getPosition();
	const size_t snakeSize = snakeBody.size();
	const float snakeMovingCoeff = clock.getSnakeMovingCoeff();
	const bool eatingApple = head == applePos;

	vec2i prevTail = game.snake().getPrevTail();
	static vec2i lastHead(*(snakeBody.begin() + 1));

	const bool headThroughBorder = std::abs(head.x - snakeBody[1].x) > 1 
								|| std::abs(head.y - snakeBody[1].y) > 1;
	const bool tailThroughBorder = std::abs(tail.x - prevTail.x) > 1 
								|| std::abs(tail.y - prevTail.y) > 1;

	/* ###### dynamic objects ###### */
	// snake body
	if (updatedStaticData || (head != lastHead && snakeMovingCoeff > 0.5f)) {
		lastHead = head;

		for (size_t i = snakeSize - 2; i > 0; --i) {
			vec2i prev = snakeBody[i - 1];
			vec2i cur = snakeBody[i];
			vec2i next = snakeBody[i + 1];

			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f;
			rotateAngle = getRotateAngle(next, cur);
			texType = TexType::SNAKE_BODY;

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
					rotateAngle = rotateAngle + 3.14159265359f / 2.f;

				texType = TexType::SNAKE_CORNER;
			}

			renderer.addObject(
				size, pos, texType,
				texCoord, color, rotateAngle
			);
		}

		renderer.refreshDynamicBuffer();
	}

	/* ###### stream objects ###### */
	// apple
	if (!(eatingApple && snakeMovingCoeff > 0.9f)) {
		constexpr vec4f appleColor(1.f, 1.f, 1.f, 1.f);
		constexpr float appleRotateAngle = 0.f;
		pos = static_cast<vec2f>(applePos) * cellSize + size * 0.5f;
		renderer.addObject(
			size * clock.getAppleBreathingCoeff(), pos,
			TexType::APPLE,
			texCoord, appleColor, appleRotateAngle
		);
	}

	static bool newDirectionTail = false;
	bool drawHead = true;
	bool drawTail = true;

	if (snakeMovingCoeff <= 0.5f) { 
		size = cellSize;

		vec2i prev = snakeSize > 2 ? snakeBody[2] : prevTail;
		vec2i cur = snakeBody[1];
		vec2i next = head;

		vec2i diff = next - prev;
		// turning head
		if (diff.x != 0 && diff.y != 0) {
			// waiting head
			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f;
			texCoord = {0.f, -0.1f, 1.f, 0.9f };
			rotateAngle = getRotateAngle(cur, prev);
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle
			);

			// dynamic head
			pos = (static_cast<vec2f>(cur) + static_cast<vec2f>(direction) * 0.5f) * cellSize + size * 0.5f;
			rotateAngle = getRotateAngle(next + direction, next);
			texCoord = { 0.f, -0.5f + snakeMovingCoeff, 1.f, 0.5f + snakeMovingCoeff};

			if (headThroughBorder) {
				pos = static_cast<vec2f>(head) * cellSize + size * 0.5f;
				texCoord = { 0.f, -1.f + snakeMovingCoeff, 1.f, snakeMovingCoeff };
				renderer.addObject(
					size, pos, TexType::SNAKE_TAIL,
					texCoord, color, rotateAngle
				);	

				pos = static_cast<vec2f>(snakeBody[1]) * cellSize + size * 0.5f + static_cast<vec2f>(direction) * cellSize * 0.25f;
				texCoord = { 0.f, 0.0f + snakeMovingCoeff, 1.f, 0.5f + snakeMovingCoeff};
				size = { cellSize, cellSize * 0.5f };
			}
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle
			);
			size = cellSize;

			drawHead = false;
		}
		// aux straight head
		else if (snakeSize > 2) {
			rotateAngle = getRotateAngle(next, cur);
			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f
				- static_cast<vec2f>(direction) * cellSize * 0.25f;
			size = { cellSize, cellSize * 0.5f };
			texCoord = { 0.f, 0.f, 1.f, 0.5f };
			renderer.addObject(
				size, pos, TexType::SNAKE_BODY,
				texCoord, color, rotateAngle
			);
			size = cellSize;
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
			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f;
			texCoord = {0.f, -0.1f, 1.f, 0.9f };
			rotateAngle = getRotateAngle(cur, next);
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle
			);

			// aux static body to fill space between head and tail in 2sized snake
			pos = static_cast<vec2f>(next) * cellSize + size * 0.5f;
			texCoord = { 0.f, 0.5f, 1.f, 1.5f };
			renderer.addObject(
				size, pos, TexType::SNAKE_BODY,
				texCoord, color, rotateAngle
			);

			// new step after turned tail -- little dynamic tail 
			diff = cur - prev;
			if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
			else  			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;
			pos = (static_cast<vec2f>(cur) - static_cast<vec2f>(diff) * 0.5f) * cellSize + size * 0.5f;
			texCoord = {0.f, -0.5f - auxSnakeMovingCoeff, 1.f, 0.5f - auxSnakeMovingCoeff};
			rotateAngle = getRotateAngle(prev, cur);
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle
			);

			drawTail = false;
			if (auxSnakeMovingCoeff > 0.1f)
				newDirectionTail = false;
		}
	}
	else {
		size = cellSize;

		vec2i prev = prevTail;
		vec2i cur = tail;
		vec2i next = *(snakeBody.rbegin() + 1);

		vec2i diff = next - prev;
		// turning tail
		if ((diff.x != 0 && diff.y != 0)) {
			// waiting tail
			size = cellSize;
			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f;
			texCoord = {0.f, -0.1f, 1.f, 0.9f };
			rotateAngle = getRotateAngle(cur, next);
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle
			);

			// dynamic tail
			diff = cur - prev;
			if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
			else  			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;
			pos = (static_cast<vec2f>(cur) - static_cast<vec2f>(diff) * 0.5f) * cellSize + size * 0.5f;
			rotateAngle = getRotateAngle(prev, cur);
			texCoord = eatingApple ? vec4f{0.f, -0.5f, 1.f, 0.5f }
								   : vec4f{0.f, 0.5f - snakeMovingCoeff, 1.f, 1.5f - snakeMovingCoeff};
			
			if (tailThroughBorder) {
				pos = static_cast<vec2f>(prevTail) * cellSize + size * 0.5f;
				texCoord = { 0.f, -snakeMovingCoeff , 1.f, 1.f - snakeMovingCoeff};
				renderer.addObject(
					size, pos, TexType::SNAKE_TAIL,
					texCoord, color, rotateAngle
				);	

				pos = static_cast<vec2f>(tail) * cellSize + size * 0.5f - static_cast<vec2f>(diff) * cellSize * 0.25f;
				texCoord = { 0.f, 1.f - snakeMovingCoeff, 1.f, 1.5f - snakeMovingCoeff };
				size = { cellSize, cellSize * 0.5f };
			}
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle	
			);

			drawTail = false;
			newDirectionTail = true;
		}
		// aux straight tail
		else if (snakeSize > 2) {
			vec2i diff = next - cur;
			if (diff.x != 0) diff.x = (diff.x == -1 || diff.x > 1) ? -1 : 1;
			else 			 diff.y = (diff.y == -1 || diff.y > 1) ? -1 : 1;

			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f
				+ static_cast<vec2f>(diff) * cellSize * 0.25f;
			size = { cellSize, cellSize * 0.5f };
			texCoord = { 0.f, 0.f, 1.f, 0.5f };
			rotateAngle = getRotateAngle(cur, next);
			renderer.addObject(
				size, pos, TexType::SNAKE_BODY, 
				texCoord, color, rotateAngle
			);
		}
	}

	size = cellSize;

	// snake head
	if (drawHead) {
		vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;

		texCoord = {0.f, 0.f, 1.f, 1.f};
		rotateAngle = getRotateAngle(head + direction, head);
		pos = (static_cast<vec2f>(snakeBody[1]) + alpha) * cellSize + size * 0.5f;
		if (headThroughBorder) {
			pos = static_cast<vec2f>(head) * cellSize + size * 0.5f;
			texCoord = { 0.f, -1.f + snakeMovingCoeff, 1.f, snakeMovingCoeff };
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle
			);	

			pos = static_cast<vec2f>(snakeBody[1]) * cellSize + size * 0.5f;
			texCoord = { 0.f, snakeMovingCoeff , 1.f, 1.f + snakeMovingCoeff};
		}
		renderer.addObject(
			size, pos, TexType::SNAKE_TAIL,
			texCoord, color, rotateAngle
		);
	}

	// snake tail
	if (drawTail) {
		vec2f alpha = static_cast<vec2f>(tail - prevTail) * snakeMovingCoeff;

		texCoord = { 0.f, 0.f, 1.f, 1.f };
		rotateAngle = getRotateAngle(prevTail, tail);
		size = cellSize;
		pos = (static_cast<vec2f>(prevTail) + alpha) * cellSize + size * 0.5f;

		if (eatingApple) {
			pos = static_cast<vec2f>(tail) * cellSize + size * 0.5f;
		}
		else if (tailThroughBorder) {
			pos = static_cast<vec2f>(tail) * cellSize + size * 0.5f;
			texCoord = { 0.f, 1.f - snakeMovingCoeff, 1.f, 2.f - snakeMovingCoeff };
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle
			);	

			pos = static_cast<vec2f>(prevTail) * cellSize + size * 0.5f;
			texCoord = { 0.f, -snakeMovingCoeff , 1.f, 1.f - snakeMovingCoeff};
		}

		renderer.addObject(
			size, pos, TexType::SNAKE_TAIL,
			texCoord, color, rotateAngle
		);
	}

	// eyes
	vec2f eyeSize = cellSize * 0.45f;
	vec2f eyeDotSize = cellSize * 0.2f;
	vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;
	vec2f eyePos = (static_cast<vec2f>(snakeBody[1]) + alpha) * cellSize + size * 0.5f;

	color = {0.15f, 0.37f, 0.2f, 1.f};
	size = cellSize;
	pos = eyePos;
	texCoord = {0.f, 0.f, 1.f, 1.f};
	rotateAngle = getRotateAngle(head + direction, head);

	// eye place
	if (direction.x == 0) {
		pos.x += cellSize * 0.35f * (direction.y > 0 ? -1.f : 1.f);
		pos.y += cellSize * 0.3f * (direction.y > 0 ? -1.f : 1.f);
	} else {
		pos.x += cellSize * 0.3f * (direction.x > 0 ? -1.f : 1.f);
		pos.y += cellSize * 0.35f * (direction.x > 0 ? -1.f : 1.f);
	}

	if (headThroughBorder && snakeMovingCoeff > 0.6f) {
		texCoord = {0.f, (snakeMovingCoeff - 0.6f) * 2.22222f, 1.f, 1.f + (snakeMovingCoeff - 0.6f) * 2.22222f};

		pos = static_cast<vec2f>(snakeBody[1]) * cellSize + size * 0.5f;

		color = { 1.f, 0.f, 0.f, 1.f };

		if (direction.x == 0) {
			pos.x += cellSize * 0.35f * (direction.y > 0 ? -1.f : 1.f);
			pos.y += cellSize * 0.3f * (direction.y > 0 ? -1.f : 1.f) + (direction.y > 0 ? cellSize - eyeSize.y + 1.f : -cellSize + eyeSize.y - 1.f);
		} else {
			pos.x += cellSize * 0.3f * (direction.x > 0 ? -1.f : 1.f) + (direction.x > 0 ? cellSize - eyeSize.x + 1.f : -cellSize + eyeSize.x - 1.f);
			pos.y += cellSize * 0.35f * (direction.x > 0 ? -1.f : 1.f);
		}
	}

	renderer.addObject(
		eyeSize, pos, TexType::EYE_ORBIT,
		texCoord, color, rotateAngle
	);

	pos = eyePos;
	if (direction.x == 0) {
		pos.x += cellSize * 0.35f * (direction.y > 0 ? 1.f : -1.f);
		pos.y += cellSize * 0.3f * (direction.y > 0 ? -1.f : 1.f);
	} else {
		pos.x += cellSize * 0.3f * (direction.x > 0 ? -1.f : 1.f);
		pos.y += cellSize * 0.35f * (direction.x > 0 ? 1.f : -1.f);
	}
	renderer.addObject(
		eyeSize, pos, TexType::EYE_ORBIT,
		texCoord, color, rotateAngle
	);
	color = {0.15f, 0.37f, 0.2f, 1.f};

	// eye
	if (!(headThroughBorder && snakeMovingCoeff > 0.6f)) {
		vec2f diff = eyePos - (static_cast<vec2f>(applePos) * cellSize + size * 0.5f);
		diff.y = -diff.y;
		rotateAngle = std::atan2(diff.x, diff.y);

		color = {1.f, 1.f, 1.f, 1.f};
		eyeSize = cellSize * 0.3f;
		pos = eyePos;
		if (direction.x == 0) {
			pos.x += cellSize * 0.35f * (direction.y > 0 ? -1.f : 1.f);
			pos.y += cellSize * 0.3f * (direction.y > 0 ? -1.f : 1.f);
		} else {
			pos.x += cellSize * 0.3f * (direction.x > 0 ? -1.f : 1.f);
			pos.y += cellSize * 0.35f * (direction.x > 0 ? -1.f : 1.f);
		}
		renderer.addObject(
			eyeSize, pos, TexType::EYE,
			texCoord, color, rotateAngle
		);

		pos = eyePos;
		if (direction.x == 0) {
			pos.x += cellSize * 0.35f * (direction.y > 0 ? 1.f : -1.f);
			pos.y += cellSize * 0.3f * (direction.y > 0 ? -1.f : 1.f);
		} else {
			pos.x += cellSize * 0.3f * (direction.x > 0 ? -1.f : 1.f);
			pos.y += cellSize * 0.35f * (direction.x > 0 ? 1.f : -1.f);
		}
		renderer.addObject(
			eyeSize, pos, TexType::EYE,
			texCoord, color, rotateAngle
		);
	}

	// cloding eye
	if (headThroughBorder && snakeMovingCoeff <= 0.6f) {
		color = {0.15f, 0.37f, 0.2f, 1.f};
		pos = eyePos;
		texCoord = {0.f, 0.f, 1.f, 1.f};
		rotateAngle = getRotateAngle(head + direction, head);
		texCoord = { 0.49f, 1.f - snakeMovingCoeff * 1.f, 0.51f, 2 - snakeMovingCoeff * 1.f };

		if (direction.x == 0) {
			pos.x += cellSize * 0.35f * (direction.y > 0 ? -1.f : 1.f);
			pos.y += cellSize * 0.3f * (direction.y > 0 ? -1.f : 1.f);
		} else {
			pos.x += cellSize * 0.3f * (direction.x > 0 ? -1.f : 1.f);
			pos.y += cellSize * 0.35f * (direction.x > 0 ? -1.f : 1.f);
		}
		renderer.addObject(
			eyeSize, pos, TexType::EYE_ORBIT,
			texCoord, color, rotateAngle
		);

		pos = eyePos;
		if (direction.x == 0) {
			pos.x += cellSize * 0.35f * (direction.y > 0 ? 1.f : -1.f);
			pos.y += cellSize * 0.3f * (direction.y > 0 ? -1.f : 1.f);
		} else {
			pos.x += cellSize * 0.3f * (direction.x > 0 ? -1.f : 1.f);
			pos.y += cellSize * 0.35f * (direction.x > 0 ? 1.f : -1.f);
		}
		renderer.addObject(
			eyeSize, pos, TexType::EYE_ORBIT,
			texCoord, color, rotateAngle
		);

		pos = eyePos;
		texCoord = { 0.49f, -1.f + snakeMovingCoeff * 1.f, 0.51f, 0 + snakeMovingCoeff * 1.f };
		if (direction.x == 0) {
			pos.x += cellSize * 0.35f * (direction.y > 0 ? -1.f : 1.f);
			pos.y += cellSize * 0.3f * (direction.y > 0 ? -1.f : 1.f);
		} else {
			pos.x += cellSize * 0.3f * (direction.x > 0 ? -1.f : 1.f);
			pos.y += cellSize * 0.35f * (direction.x > 0 ? -1.f : 1.f);
		}
		renderer.addObject(
			eyeSize, pos, TexType::EYE_ORBIT,
			texCoord, color, rotateAngle
		);

		pos = eyePos;
		if (direction.x == 0) {
			pos.x += cellSize * 0.35f * (direction.y > 0 ? 1.f : -1.f);
			pos.y += cellSize * 0.3f * (direction.y > 0 ? -1.f : 1.f);
		} else {
			pos.x += cellSize * 0.3f * (direction.x > 0 ? -1.f : 1.f);
			pos.y += cellSize * 0.35f * (direction.x > 0 ? 1.f : -1.f);
		}
		renderer.addObject(
			eyeSize, pos, TexType::EYE_ORBIT,
			texCoord, color, rotateAngle
		);
	}

	renderer.refreshStreamBuffer();
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
