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

void cut(
	vec2f& size, vec2f& pos, vec4f& texCoord, const vec2f fieldSize, 
	const bool enter, const bool forward, const bool horizontal
);

void addThroughObjects(
	Renderer& renderer, const rectangleData& data, const vec2i direction, 
	const vec2f fieldSize, const bool forward
);

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
	
	{
		vec4f clearColor = game.getThemeColor() - vec4f{0.1f, 0.1f, 0.1f, 0.f};
		renderer.setClearColor(clearColor);
	}

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
			game.updateStatus(GameStatus::GAME_START);
			game.reset();
			window.showDetailedTitle(true);
			window.updateScore(false);
			inputManager.turnOffBuffer();
			clock.resetGameStepAccumulator();
			clock.freezeSnake();
		}
	}

	else if (game.status() != GameStatus::MENU) {
		if (inputManager.isKeyPressed(Action::Pause)) {
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
		cellSize, {0.f, 0.f}, {1.f, 1.f, 1.f, 1.f}, 
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

		constexpr vec2f minViewSize{1200.f, 800.f};
		constexpr vec2f fieldTexSize{80.f, 80.f};
		vec2i rawFieldSize(game.field().getFieldSize());

		vec2f startPos(0.f, 0.f);
		if (!renderer.isZenMode()) {
			startPos.x = fieldSize.x < minViewSize.x - 400.f ? (minViewSize.x - fieldSize.x) * 0.5f : 200.f;
			startPos.y = fieldSize.y < minViewSize.y ? (minViewSize.y - fieldSize.y) * 0.5f : 20.f;
		}
		startPos += fieldTexSize * 0.5f;
		const vec2f fieldStartPos = startPos - fieldTexSize * 0.5f;
		
		// =================== panels
		if (!renderer.isZenMode()) {
			data.color = {themeColor.x - 0.05f, themeColor.y - 0.05f, themeColor.z - 0.05f, themeColor.w};
			data.size = { 200.f, 800.f };
			data.texType = TexType::SNAKE_BODY;
			data.texCoord = { 0.f, 0.f, 1.f, 1.f };

			// =================== left panel
			vec2f panelPos = data.pos = vec2f{
				0.f, 
				(fieldSize.y < minViewSize.y ? 0.f : (fieldSize.y - data.size.y) * 0.5f) + 20.f
			} + data.size * 0.5f;
			renderer.addObject(data);

			// ~~~~~~~~~~~~~~~~~~~ icons on the left panel

			// =================== right panel
			data.color = {themeColor.x - 0.05f, themeColor.y - 0.05f, themeColor.z - 0.05f, themeColor.w};
			data.size = { 200.f, 800.f };
			data.texType = TexType::SNAKE_BODY;
			constexpr float minRightPanelX = 1000.f;
			float rightPanelX = fieldStartPos.x + fieldSize.x;
			
			data.pos = panelPos;
			data.pos.x = (rightPanelX < minRightPanelX ? minRightPanelX : rightPanelX) + data.size.x * 0.5f;
			renderer.addObject(data);

			// ~~~~~~~~~~~~~~~~~~~ icons on the right panel
		}

		// =================== field
		if (gameStatus != GameStatus::MENU) {
			isMenuStatus = false;

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

			if (!evenXCellsCount || !evenYCellsCount) {
				data.color = {themeColor.x - 0.1f, themeColor.y - 0.1f, themeColor.z - 0.1f, themeColor.w};
				data.texCoord = { 0.1f, 0.1f };

				vec2f hiddenLinePos = fieldStartPos + fieldSize + vec2f(cellSize) * 0.5f;

				// ~~~~~~~~~~~~~~~~~~~ right hidden line
				data.size = { cellSize, fieldSize.y + cellSize };
				data.pos = { hiddenLinePos.x, hiddenLinePos.y - fieldSize.y * 0.5f };
				renderer.addObject(data);

				// ~~~~~~~~~~~~~~~~~~~ left hidden line
				data.size = { fieldSize.x + cellSize, cellSize };
				data.pos = { hiddenLinePos.x - fieldSize.x * 0.5f, hiddenLinePos.y };
				renderer.addObject(data);
			}
		}

		updatedStaticData = true;
		renderer.setOrigin(fieldStartPos);
		renderer.refreshStaticBuffer();
	}

	data.size = cellSize;
	data.color = snakeColor;
	data.texCoord = { 0.f, 0.f, 1.f, 1.f };

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

				if (tailThroughBorder) {
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

			if (!headThroughBorder)
				renderer.addObject(data);
			else
				addThroughObjects(renderer, data, direction, fieldSize, true);
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
			if (!tailThroughBorder || eatingApple)
				renderer.addObject(data);
			else
				addThroughObjects(renderer, data, diff, fieldSize, false);
		}

		// eyes
		data.size = cellSize;
		const vec2f eyePlaceSize = cellSize * 0.45f;
		const vec2f eyeSize = cellSize * 0.3f;
		const vec2f alpha = static_cast<vec2f>(direction) * snakeMovingCoeff;
		const vec2f eyePos = (static_cast<vec2f>(snakeBody[1]) + alpha) * cellSize + data.size * 0.5f;

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

		data.color = { snakeColor.x - 0.03f, snakeColor.y - 0.03f, snakeColor.z - 0.03f, snakeColor.w };
		data.texCoord = {0.f, 0.f, 1.f, 1.f};
		data.rotateAngle = getRotateAngle(head + direction, head);
		data.texType = TexType::EYE_ORBIT;
		data.pos = eye1Pos;
		data.size = eyePlaceSize;

		// eye places
		if (!headThroughBorder)
			renderer.addObject(data);		
		else
			addThroughObjects(renderer, data, direction, fieldSize, true);
		
		data.pos = eye2Pos;
		if (!headThroughBorder)
			renderer.addObject(data);
		else
			addThroughObjects(renderer, data, direction, fieldSize, true);

		data.color = {1.f, 1.f, 1.f, 1.f};
		data.texCoord = { 0.f, 0.f, 1.f, 1.f };
		// eyes
		if (!game.getPravednovBool()) {
			if (!(headThroughBorder && snakeMovingCoeff > 0.6f) && !clock.isBlinkTime()) {
				if (!eatingApple && !straightView) {
					vec2f diff = eyePos - (static_cast<vec2f>(applePos) * cellSize + data.size * 0.5f);
					diff.y = -diff.y;
					data.rotateAngle = std::atan2(diff.x, diff.y);
				} else {
					straightView = true;
				}

				data.texType = TexType::EYE;
				data.size = eyeSize;
				data.pos = eye1Pos;
				renderer.addObject(data);

				data.pos = eye2Pos;
				renderer.addObject(data);
			}
		}
		else {
			data.texType = TexType::PRAVEDNOV;
			// data.size = direction == Direction::RIGHT ? vec2f{ 80.f, -80.f } : 80.f;
			data.size = 80.f;
			if (direction == Direction::RIGHT)
				data.texCoord = { 0.f, 1.f, 1.f, 0.f };
			data.pos = eyePos;
			data.rotateAngle -= 1.570796f;
			if (!headThroughBorder)
				renderer.addObject(data);
			else
				addThroughObjects(renderer, data, direction, fieldSize, true);
		}

		renderer.refreshStreamBuffer();
	} 
	else {
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

void cut(
	vec2f& size, vec2f& pos, vec4f& texCoord, const vec2f fieldSize, 
	const bool enter, const bool forward, const bool horizontal
) {
	float diff = 0.f;
	if (horizontal) {
		float right = pos.x + size.y * 0.5f - fieldSize.x;
		float left = pos.x - size.y * 0.5f;
		diff = right > 0.f ? right : (left < 0.f ? left : 0.f);
	}
	else {
		float top = pos.y + size.y * 0.5f - fieldSize.y;
		float bottom = pos.y - size.y * 0.5f;
		diff = top > 0.f ? top : (bottom < 0.f ? bottom : 0.f);
	}

	if (horizontal && diff != 0.f)
		pos.x -= diff * 0.5f;
	else pos.y -= diff * 0.5f;

	if (enter) {
		if (forward) {
			if (diff > 0.f) {
				texCoord.y += diff / size.y;
				size.y -= diff;
			}
			else {
				texCoord.y -= diff / size.y;
				size.y += diff;
			}
		}
		else {
			if (diff > 0.f) {
				texCoord.w -= diff / size.y;
				size.y -= diff;
			}
			else {
				texCoord.w += diff / size.y;
				size.y += diff;
			}
		}
	}
	else {
		float cell = size.y;
		if (forward) {
			if (diff > 0.f) {
				size.y -= diff;
				texCoord.w += size.y / cell;
			}
			else {
				size.y += diff;
				texCoord.w += size.y / cell;
			}
		}
		else {
			if (diff > 0.f) {
				size.y -= diff;
				texCoord.y -= size.y / cell;
			}
			else {
				size.y += diff;
				texCoord.y -= size.y / cell;
			}
		}
	}
}

void addThroughObjects(
	Renderer& renderer, const rectangleData& data, const vec2i direction, 
	const vec2f fieldSize, const bool forward
) {
	const bool horizontal = direction.x != 0;

	vec2f _size = data.size;
	vec2f _pos = data.pos;
	vec4f _texCoord = data.texCoord;

	cut(_size, _pos, _texCoord, fieldSize, true, forward, horizontal);
	renderer.addObject({
		_size, _pos, data.color, _texCoord, 
		data.texType, data.rotateAngle
	});

	_pos = data.pos;
	if (direction.x != 0)
		_pos.x += fieldSize.x * (direction.x > 0 ? -1.f : 1.f);
	else
		_pos.y += fieldSize.y * (direction.y > 0 ? -1.f : 1.f);

	_texCoord = forward ? vec4f{ 0.f, 0.f, 1.f, 0.f } 
						: vec4f{ 0.f, 1.f, 1.f, 1.f };
	_size = data.size;
	cut(_size, _pos, _texCoord, fieldSize, false, forward, horizontal);
	renderer.addObject({
		_size, _pos, data.color, _texCoord, 
		data.texType, data.rotateAngle
	});	
}
