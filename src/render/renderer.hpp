#pragma once

#include "texture_enum.hpp"
#include "../core/rectangle.hpp"
#include "../core/vector.hpp"

#include <vector>

class ShaderProgram;

class Renderer {
private:
    unsigned int _shaderProgram;
    bool _zenMode;
    bool _needUpdateStatic;
    bool _needUpdateSemiStatic;
    bool _staticMode;

    unsigned int _vao;
    unsigned int _vbo;
    unsigned int _ebo;
    unsigned int _textureArray;

    unsigned int _drawingIndices;
    unsigned int _staticDrawingIndices;
    unsigned int _semistaticDrawingIndices;
    unsigned int _offsetSemistaticIndices;

    vec2f _origin;
    vec2f _viewSize;
    float _contentScale;

    std::vector<Rectangle> _rectangleArray;

    void init();
    void updateBuffer();

public:
    Renderer();
    ~Renderer();

    bool getInitializeInfo() const { return static_cast<bool>(_shaderProgram); }

    bool getNeedUpdateStatic() const { return _needUpdateStatic; }

    bool isZenMode() const { return _zenMode; }

    void setStaticMode(const bool staticMode) { _staticMode = staticMode; }

    void setOrigin(const vec2f origin) { _origin = origin * _contentScale - _viewSize; }

    void saveSemistaticIndices() { 
        _semistaticDrawingIndices = _drawingIndices;
        _drawingIndices = 0;
        _offsetSemistaticIndices = 0;
    }

    void addObject(
        vec2f size, vec2f pos, const TexType texType, 
        const vec2f texCoord, const vec4f color, const float rotateAngle
    );

    void draw();

    void setViewSize(const vec2i viewSize) { 
        _origin += _viewSize;
        _viewSize = static_cast<vec2f>(viewSize) * 0.5f; 
        _needUpdateStatic = true; 
        _staticDrawingIndices = 0;
        _origin -= _viewSize;
    }
    void setContentScale(const float contentScale) { _contentScale = contentScale; }
    void changeZenMode() { _zenMode = !_zenMode; }
};