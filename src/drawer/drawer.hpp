#pragma once

#include "enums.hpp"
#include "rectangle.hpp"
#include "renderer.hpp"

#include "src/core/vector.hpp"
#include "src/game/enums.hpp"
#include "src/game/game.hpp"
#include "src/core/clock.hpp"

#include <string>

class Drawer {
  public:
    Drawer();
    Drawer& operator=(const Drawer&) = delete;

    ~Drawer();

    void updateSnakeColor(const vec4f color);
    void updateThemeColor(const vec4f color);
    void updateZenMode();
    void updateContentScale(const bool increase);
    void updateFieldSize(const vec2i fieldCellsNum);

    auto getContentScale() const -> float { return static_cast<float>(_contentScale) / 100.f; }
    auto getRendererStatus() const -> bool { return _rendererHandler.getInitializeInfo(); }
    auto getViewSize() const -> vec2f { return _viewSize; }
    auto getZenMode() const -> bool { return _zenMode; }

    void changePravednovStatus();

    void draw(const Game& game, const Clock& clock);
  private:
    Renderer _rendererHandler;
    RectangleData _data;

    bool _zenMode;
    bool _zenModeChanged;
    bool _updateStaticBuffer;
    bool _isPravednovHead;

    int _contentScale;
    vec2f _origin;
    vec2f _viewSize;

    vec2f _fieldSize;
    vec2f _lbFieldPos;
    vec2f _lbPanelPos;

    vec4f _themeColor;
    vec4f _auxThemeColor;
    vec4f _snakeColor;

    vec2f _addPanel();
    void _addPanelIcons();
    void _addPanelIconsValues(const int fps, const vec2i fieldSize, const GameMode mode,
                              const float stepTime, const int bestScore, const int score);

    void _addMenuButtons();

    void _addAuxPanel(const GameStatus status);
    void _addAuxPanelTitle(const std::string& str);

    void _addField(const vec2i cellsNum);
    void _addInvisibleLines();

    void _addApple(const vec2f position, const float breathCoeff);
    void _addDynamicSnake(const Snake& snake);
    void _addStreamSnake(const Snake& snake, const float snakeMovingCoeff, const bool eatingApple,
                        const vec2i applePos, bool& straightLook, const bool isBlinkTime);

    void _printText(const std::string& str, const Alignment alignment=Alignment::LEFT, const float scale=1.f);
    void _add(const RectangleData& data);
};
