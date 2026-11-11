#pragma once

#include "src/core/vector.hpp"
#include "src/core/rectangle.hpp"
#include "src/render/renderer.hpp"
#include "src/game/game.hpp"
#include "src/core/clock.hpp"

class Drawer {
  public:
      Drawer(Renderer& renderer): _rendererHandler(&renderer), _isPrevStatusMenu(true) {}
      Drawer& operator=(const Drawer&) = delete;

      void setSnakeColor(const vec4f color);
      void setThemeColor(const vec4f color);
      void setViewSize(const vec2f size);

      void draw(const Game& game, const Clock& clock);
  private:
      Renderer* _rendererHandler;
      
      vec2f _fieldSize;
      vec2f _lbFieldPos;

      rectangleData _data;

      vec4f _themeColor;
      vec4f _auxThemeColor;
      vec4f _snakeColor;

      vec2f _viewSize;

      void addPanel();
      void addPanelIcons();
      void addField(const vec2i cellsNum);
      void addInvisibleLines();
      void addApple(const vec2f position, const float breathCoeff);
      void addDynamicSnake(const Snake& snake);
      void addStreamSnake(const Snake& snake, const float snakeMovingCoeff, const bool eatingApple, 
                          const bool pravednovFace, const vec2i applePos, bool& straightLook, const bool isBlinkTime);
      void addPlayButton(const float breathCoeff);

      bool _isPrevStatusMenu;
};
