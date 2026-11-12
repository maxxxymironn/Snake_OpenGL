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

      void draw(const Game& game, const Clock& clock);
  private:
      Renderer _rendererHandler;

      RectangleData _data;

      bool _zenMode;
      bool _updateStaticBuffer;
      bool _isPrevStatusMenu;

      float _contentScale;
      vec2f _origin;
      vec2f _viewSize;

      
      
      vec2f _fieldSize;
      vec2f _lbFieldPos;
      vec2f _lbPanelPos;

      vec4f _themeColor;
      vec4f _auxThemeColor;
      vec4f _snakeColor;

      vec2f addPanel();
      void addPanelIcons(const int fps, const int score, const int bestScore, 
                         const vec2i fieldSize, const char* gameMode);
      void addField(const vec2i cellsNum);
      void addInvisibleLines();
      void addApple(const vec2f position, const float breathCoeff);
      void addDynamicSnake(const Snake& snake);
      void addStreamSnake(const Snake& snake, const float snakeMovingCoeff, const bool eatingApple, 
                          const bool pravednovFace, const vec2i applePos, bool& straightLook, const bool isBlinkTime);
      void addPlayButton(const float breathCoeff);

      void _printText(const std::string& str);
      void _add(const RectangleData& data);
};
