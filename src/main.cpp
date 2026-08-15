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
		vec2f fieldSize(static_cast<vec2f>(rawFieldSize) * cellSize);

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

	size = vec2f{cellSize, cellSize};
	color = {0.15f, 0.4f, 0.2f, 1.f};

	const float snakeMovingCoeff = clock.getSnakeMovingCoeff();
	std::vector<vec2i> snakeBody = game.snake().getBody();
	vec2i direction = game.snake().getDirection();
	vec2i prevTail = game.snake().getPrevTail();
	vec2i head = *snakeBody.begin();
	static vec2i lastHead(*(snakeBody.begin() + 1));
	
	/* ###### dynamic objects ###### */
	// =================== Snake body
	if (updatedStaticData || (head != lastHead && snakeMovingCoeff > 0.5f)) {
		lastHead = head;
		texCoord = {0.f, 0.f, 1.f, 0.99f };
		for (auto it = snakeBody.rbegin() + 1; it + 1 != snakeBody.rend(); ++it) {
			size = cellSize;
			pos = static_cast<vec2f>(*it) * cellSize + size * 0.5f;
			size = { cellSize, cellSize * 1.05f };
			rotateAngle = getRotateAngle(*(it + 1), *it);
			TexType texType = TexType::SNAKE_BODY;

			vec2i prev = *(it - 1);
			vec2i next = *(it + 1);
			vec2i diff = next - prev;

			// corner
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

				size = cellSize;
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
	size = cellSize;
	// =================== apple
	if (!(snakeBody[0] == game.apple().getPosition() && snakeMovingCoeff > 0.9f)) {
		pos = static_cast<vec2f>(game.apple().getPosition()) * cellSize + size * 0.5f;
		color = {1.f, 1.f, 1.f, 1.f};
		rotateAngle = 0.f;
		renderer.addObject(
			size * clock.getAppleBreathingCoeff(), pos,
			TexType::APPLE,
			texCoord, color, rotateAngle
		);
	}

	bool drawHead = true;
	bool drawTail = true;
	static bool newDirectionTail = false;
	static vec2i lastPrevTail;

	color = {0.15f, 0.4f, 0.2f, 1.f};

	if (snakeMovingCoeff <= 0.5f) { 
		vec2i cur = snakeBody[1];
		vec2i next = head;
		vec2i prev = snakeBody.size() > 2 ? snakeBody[2] : prevTail;
		vec2i diff = next - prev;

		// turning head
		if (diff.x != 0 && diff.y != 0) {
			rotateAngle = getRotateAngle(cur, prev);
			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f;

			diff = cur - prev;
			if (diff.x != 0)
				pos.x += (diff.x == -1 || diff.x > 1) ? 2.f : -2.f;
			else
				pos.y += (diff.y == -1 || diff.y > 1) ? 2.f : -2.f;
			size = { cellSize, cellSize * 0.9f };
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				{ 0.f, 0.f, 1.f, 0.9f }, color, rotateAngle
			);

			size = cellSize;
			pos = (static_cast<vec2f>(cur) + static_cast<vec2f>(direction) * 0.5f) * cellSize + size * 0.5f;
			rotateAngle = getRotateAngle(head + direction, head);
			texCoord = { 0.f, -0.5f + snakeMovingCoeff, 1.f, 0.5f + snakeMovingCoeff};
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle
			);

			drawHead = false;
		}
		// aux straight head
		else if (snakeBody.size() > 2) {
			rotateAngle = getRotateAngle(next, cur);
			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f
				- static_cast<vec2f>(direction) * cellSize * 0.25f;
			size = { cellSize, cellSize * 0.65f };
			texCoord = { 0.f, 0.f, 1.f, 0.5f };
			renderer.addObject(
				size, pos, TexType::SNAKE_BODY,
				texCoord, color, rotateAngle
			);
		}

		if (newDirectionTail) {
			vec2i next = snakeBody.back();
			vec2i cur = prevTail;
			vec2i prev = lastPrevTail;
			diff = cur - prev;

			size = cellSize;

			float tmpSnakeMovingCoeff = snakeMovingCoeff;
			if (head == game.apple().getPosition()) {
				tmpSnakeMovingCoeff = 0.f;
				next = *(snakeBody.rbegin() + 1);
				cur = snakeBody.back();
				prev = prevTail;
				diff = cur - prev;
			}

			// dynamic tail
			pos = (static_cast<vec2f>(cur) - static_cast<vec2f>(diff) * 0.5f) * cellSize + size * 0.5f;

			texCoord = {0.f, -0.5f - tmpSnakeMovingCoeff, 1.f, 0.5f - tmpSnakeMovingCoeff};
			rotateAngle = getRotateAngle(prev, cur);

			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle
			);

			// waiting tail
			rotateAngle = getRotateAngle(cur, next);
			diff = next - cur;

			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f;

			if (diff.x != 0) pos.x += (diff.x == -1 || diff.x > 1) ? -4.f : 4.f;
			else pos.y += (diff.y == -1 || diff.y > 1) ? -4.f : 4.f;

			size = { cellSize, cellSize };
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				{ 0.f, 0.f, 1.f, 0.9f }, color, rotateAngle
			);

			drawTail = false;
			if (tmpSnakeMovingCoeff > 0.1f)
				newDirectionTail = false;
		}
	}
	else {
		size = cellSize;
		auto rIt = snakeBody.rbegin();
		vec2i cur = *rIt;
		vec2i next = *(rIt + 1);
		vec2i prev = prevTail;
		vec2i diff = next - prev;

		// turning tail
		if ((diff.x != 0 && diff.y != 0)) {
			rotateAngle = getRotateAngle(cur, next);
			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f;

			diff = cur - next;
			if (diff.x != 0)
				pos.x += (diff.x == -1 || diff.x > 1) ? 2.f : -2.f;
			else
				pos.y += (diff.y == -1 || diff.y > 1) ? 2.f : -2.f;
			size = { cellSize, cellSize * 0.9f };
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				{ 0.f, 0.f, 1.f, 0.9f }, color, rotateAngle
			);

			diff = cur - prev;
			rotateAngle = getRotateAngle(prev, cur);

			size = cellSize;
			pos = (static_cast<vec2f>(cur) - static_cast<vec2f>(diff) * 0.5f) * cellSize + size * 0.5f;
			texCoord = {0.f, 0.5f - snakeMovingCoeff, 1.f, 1.5f - snakeMovingCoeff};
			rotateAngle = getRotateAngle(prev, cur);

			if (head == game.apple().getPosition()) {
				texCoord = {0.f, -0.5f, 1.f, 0.5f };
			}
			
			renderer.addObject(
				size, pos, TexType::SNAKE_TAIL,
				texCoord, color, rotateAngle	
			);
			drawTail = false;
			newDirectionTail = true;
			lastPrevTail = prev;

		}
		// aux straight tail
		else if (snakeBody.size() > 2) {
			rotateAngle = getRotateAngle(cur, next);
			pos = static_cast<vec2f>(cur) * cellSize + size * 0.5f
				+ static_cast<vec2f>(next - cur) * cellSize * 0.25f;
			size = { cellSize, cellSize * 0.5f };
			texCoord = { 0.f, 0.f, 1.f, 0.5f };
			renderer.addObject(
				size, pos, TexType::SNAKE_BODY, 
				texCoord, color, rotateAngle
			);
		}
	}

	color = {0.15f, 0.4f, 0.2f, 1.f};
	size = cellSize;
	// =================== Snake head
	vec2i prevHead = *(snakeBody.begin() + 1);
	vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;

	if (drawHead) {
		rotateAngle = getRotateAngle(head + direction, head);
		pos = (static_cast<vec2f>(prevHead) + alpha) * cellSize + size * 0.5f;
		texCoord = {0.f, 0.f, 1.f, 0.99f};
		renderer.addObject(
			size, pos, TexType::SNAKE_TAIL,
			texCoord, color, rotateAngle
		);
	}
	color = {0.15f, 0.4f, 0.2f, 1.f};
	size = cellSize;
	// =================== Snake tail
	vec2i tail = *snakeBody.rbegin();
	vec2i preTail = *(snakeBody.rbegin() + 1);

	if (drawTail) {
		rotateAngle = getRotateAngle(prevTail, tail);

		size = cellSize;
		alpha = static_cast<vec2f>(tail - prevTail) * snakeMovingCoeff;
		if (head == game.apple().getPosition()) {
			alpha = 0.f;
			prevTail = tail;
		}
		pos = (static_cast<vec2f>(prevTail) + alpha) * cellSize + size * 0.5f;
		texCoord = {0.f, 0.f, 1.f, 0.99f};
		renderer.addObject(
			size, pos, TexType::SNAKE_TAIL,
			texCoord, color, rotateAngle
		);
	}

	// // drawSnake(renderer, game, clock.getSnakeMovingCoeff());
	// // if  (game.status() == GameStatus::PAUSE)
	// // 	renderer.drawPause();

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
