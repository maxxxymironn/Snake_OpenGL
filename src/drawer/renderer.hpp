#pragma once

#include "rectangle.hpp"
#include "src/core/vector.hpp"

#include <vector>

class Renderer {  
  public:
    Renderer();
    ~Renderer();

    bool getInitializeInfo() const { return _shaderProgram; }

    void refreshStaticBuffer();
    void refreshDynamicBuffer();
    void refreshStreamBuffer();

    void setClearColor(const vec4f color);

    void addObject(const Rectangle& rect);
    void draw();
    
  private:
    unsigned int _shaderProgram;

    unsigned int _staticVAO;
    unsigned int _staticVBO;
    unsigned int _dynamicVAO;
    unsigned int _dynamicVBO;
    unsigned int _streamVAO;
    unsigned int _streamVBO;
    
    unsigned int _ebo;
    unsigned int _textureArray;

    unsigned int _drawingIndices;
    unsigned int _staticDrawingIndices;
    unsigned int _dynamicDrawingIndices;
    unsigned int _streamDrawingIndices;

    std::vector<Rectangle> _rectangleArray;

    void init();
};
