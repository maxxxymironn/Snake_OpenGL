#pragma once

#include "src/core/vector.hpp"
#include "rectangle.hpp"
#include "renderer.hpp"
#include "src/game/game.hpp"
#include "src/core/clock.hpp"

#include <string>

class Drawer {
  public:
      Drawer();
      Drawer& operator=(const Drawer&) = delete;

      void updateSnakeColor(const vec4f color);
      void updateThemeColor(const vec4f color);
      void updateViewSize(const vec2f size);
      void updateZenMode(const bool zenMode);
      void updateContentScale(const float scaleCoeff);

      bool getRendererStatus() { return _rendererHandler.getInitializeInfo(); }

      void changePravednovStatus();

      void draw(const Game& game, const Clock& clock);
  private:
      Renderer _rendererHandler;
      RectangleData _data;

      bool _zenMode;
      bool _updateStaticBuffer;
      bool _isPrevStatusMenu;
      bool _isPravednovHead;

      float _contentScale;
      vec2f _origin;
      vec2f _viewSize;

      vec2f _fieldSize;
      vec2f _lbFieldPos;
      vec2f _lbPanelPos;

      vec4f _themeColor;
      vec4f _auxThemeColor;
      vec4f _snakeColor;

      vec2f _addPanel();
      void _addPanelIcons(const int fps, const int score, const int bestScore, 
                         const vec2i fieldSize, const char* gameMode);
      void _addField(const vec2i cellsNum);
      void _addInvisibleLines();
      void _addApple(const vec2f position, const float breathCoeff);
      void _addDynamicSnake(const Snake& snake);
      void _addStreamSnake(const Snake& snake, const float snakeMovingCoeff, const bool eatingApple, 
                          const vec2i applePos, bool& straightLook, const bool isBlinkTime);
      void _addPlayButton(const float breathCoeff);

      void _printText(const std::string& str);
      void _add(const RectangleData& data);
};
