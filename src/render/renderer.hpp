#pragma once

#include "texture_enum.hpp"
#include "../core/rectangle.hpp"
#include "../core/vector.hpp"

#include <vector>

class Renderer {
private:
    unsigned int _shaderProgram;
    bool _zenMode;
    bool _needRefreshStaticBuffer;
    bool _needUpdateSemiStatic;

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

    vec2f _origin;
    vec2f _viewSize;
    float _contentScale;

    std::vector<Rectangle> _rectangleArray;

    void init();

public:
    Renderer();
    ~Renderer();

    bool getInitializeInfo() const { return static_cast<bool>(_shaderProgram); }

    void refreshStaticBuffer();
    void refreshDynamicBuffer();
    void refreshStreamBuffer();

    bool needRefreshStaticBuffer() const { return _needRefreshStaticBuffer; }

    bool isZenMode() const { return _zenMode; }

    void setOrigin(const vec2f origin) { _origin = origin * _contentScale - _viewSize; }

    void addObject(
        vec2f size, vec2f pos, const TexType texType, 
        const vec2f texCoord, const vec4f color, const float rotateAngle
    );

    void draw();

    void setViewSize(const vec2i viewSize) { 
        _origin += _viewSize;
        _viewSize = static_cast<vec2f>(viewSize) * 0.5f; 
        _needRefreshStaticBuffer = true; 
        _origin -= _viewSize;
    }
    void setContentScale(const float contentScale) { _contentScale = contentScale; }
    void changeZenMode() { _zenMode = !_zenMode; }
};