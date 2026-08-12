#include "renderer.hpp"

#include "../../resources/textures/apple.hpp"
#include "../../resources/textures/body.hpp"
#include "../../resources/textures/corner.hpp"
#include "../../resources/textures/field.hpp"
#include "../../resources/textures/tail.hpp"

#include "../config/draw_config.hpp"
#include "../core/rectangle.hpp"
#include "../core/logger.hpp"
#include "shaders.hpp"

#include <glad/glad.h>

#include <cstddef>
#include <cmath>

namespace {
    GLuint createShader(GLenum typeShader, const char* shaderSource) {
        GLuint shaderID = glCreateShader(typeShader);
        glShaderSource(shaderID, 1, &shaderSource, nullptr);
        glCompileShader(shaderID);

        int success;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        if (!success) {
            Logger::getInstance().printError((typeShader == GL_VERTEX_SHADER ? "SHADER::VERTEX" : "SHADER::FRAGMENT"), "COMPILATION_FAILED");
            shaderID = 0;
        }

        return shaderID;
    }

    GLuint linkProgram(const GLenum vertexShaderID, const GLenum fragmentShaderID) {
        GLuint programID = glCreateProgram();
        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, fragmentShaderID);
        glLinkProgram(programID);

        int success;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success) {
            Logger::getInstance().printError("SHADER_PROGRAM", "LINK_FAILED");
            programID = 0;
        }

        glDetachShader(programID, fragmentShaderID);
        glDetachShader(programID, vertexShaderID);
        glDeleteShader(fragmentShaderID);
        glDeleteShader(vertexShaderID);

        return programID;
    }

    vec2f getRotatedPoint(const vec2f point, const float rotateAngle) {
        return {
            point.x * cosf(rotateAngle) - point.y * sinf(rotateAngle),
            point.x * sinf(rotateAngle) + point.y * cosf(rotateAngle)
        };
    }
}

void Renderer::init() {
    constexpr int MAX_RECTANGLES = 100;

    // setting ebo
    constexpr int MAX_INDICES = MAX_RECTANGLES * 6;
    unsigned int indices[MAX_INDICES];

    for (int i = 0, offset = 0; i < MAX_INDICES; i += 6, offset += 4) {
        indices[i] = indices[i + 5] = offset;
        indices[i + 1] = offset + 1;
        indices[i + 2] = indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3; 
    }

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

    // setting vbo
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_RECTANGLES * sizeof(Rectangle), nullptr, GL_DYNAMIC_DRAW);

    // setting vao
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // setting attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, sizeof(Vertex::pos) / sizeof(float), 
        GL_FLOAT, GL_FALSE, 
        sizeof(Vertex), (void*)(offsetof(Vertex, pos))
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, sizeof(Vertex::texCoord) / sizeof(float), 
        GL_FLOAT, GL_FALSE, 
        sizeof(Vertex), (void*)(offsetof(Vertex, texCoord))
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, sizeof(Vertex::color) / sizeof(float), 
        GL_FLOAT, GL_FALSE, 
        sizeof(Vertex), (void*)(offsetof(Vertex, color))
    );
    glad_glEnableVertexAttribArray(3);
    glVertexAttribIPointer(
        3, 1, 
        GL_INT, 
        sizeof(Vertex), (void*)(offsetof(Vertex, texLayer))
    );

    // setting textures
    constexpr GLsizei TEX_SIZE = 128;
    constexpr GLsizei LAYER_COUNT = 5;
    constexpr const unsigned char* TEX_DATA_ARRAY[LAYER_COUNT] = {
        &field[0],
        &apple[0],
        &body[0],
        &tail[0],
        &corner[0]
    };

    glGenTextures(1, &_textureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _textureArray);
    
    glUseProgram(_shaderProgram);

    glUniform1i(glGetUniformLocation(_shaderProgram, "uTexArray"), 0);

    // glm::mat4 projection = glm::ortho(
    //     0.0f, 2.f,
    //     0.0f, 2.f
    // );

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage3D(
        GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
        TEX_SIZE, TEX_SIZE, LAYER_COUNT, 
        0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr 
    );

    for (GLsizei layer = 0; layer < LAYER_COUNT; ++layer) {
        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY, 0,
            0, 0, layer, 
            TEX_SIZE, TEX_SIZE, 1,
            GL_RGBA, GL_UNSIGNED_BYTE, 
            TEX_DATA_ARRAY[layer]
        );
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void Renderer::updateBuffer() {
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    if (!_needUpdateStatic) {
        glBufferSubData(
            GL_ARRAY_BUFFER, sizeof(Rectangle) * (_staticDrawingIndices + _offsetSemistaticIndices) / 6,
            sizeof(Rectangle) * _rectangleArray.size(), _rectangleArray.data()
        );
        _offsetSemistaticIndices = _semistaticDrawingIndices;
    }
    else {
        glBufferSubData(
            GL_ARRAY_BUFFER, 0, 
            sizeof(Rectangle) * _rectangleArray.size(), _rectangleArray.data()
        );
        _needUpdateStatic = false;
    }
    glBindVertexArray(0);
}

Renderer::Renderer()
    : _shaderProgram(0), 
      _zenMode(DrawConfig::zenMode),
      _needUpdateStatic(false),
      _needUpdateSemiStatic(false),
      _staticMode(false),
      _vao(0), _vbo(0), _ebo(0), 
      _drawingIndices(0),
      _staticDrawingIndices(0),
      _semistaticDrawingIndices(0),
      _offsetSemistaticIndices(0),
      _origin(0.f, 0.f),
      _contentScale(DrawConfig::contentScale)
       {
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, shaders::vertexShaderSource);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, shaders::fragmentShaderSource);

    if (vertexShader && fragmentShader)
        _shaderProgram = linkProgram(vertexShader, fragmentShader);

    if (_shaderProgram)
        init();
}

Renderer::~Renderer() {
    if (_vao) glDeleteVertexArrays(1, &_vao);
    if (_vbo) glDeleteVertexArrays(1, &_vbo);
    if (_ebo) glDeleteBuffers(1, &_ebo);
    if (_shaderProgram) glDeleteProgram(_shaderProgram);
}

void Renderer::addObject(
    vec2f size, vec2f pos, const TexType texType, 
    const vec2f texCoord, const vec4f color, const float rotateAngle
) {
    if (!_staticMode)
        _drawingIndices += 6;
    else
        _staticDrawingIndices += 6;

    // size *= _contentScale * 0.5f;
    // pos = (pos + size) * _contentScale;
    size *= _contentScale * 0.5f;
    pos *= _contentScale;
    vec2f lb = (getRotatedPoint({-size.x,-size.y }, rotateAngle) + pos + _origin) / _viewSize;
    vec2f rb = (getRotatedPoint({ size.x,-size.y }, rotateAngle) + pos + _origin) / _viewSize;
    vec2f rt = (getRotatedPoint({ size.x, size.y }, rotateAngle) + pos + _origin) / _viewSize;
    vec2f lt = (getRotatedPoint({-size.x, size.y }, rotateAngle) + pos + _origin) / _viewSize;

    // size *= _contentScale;
    // pos *= _contentScale;
    // vec2f lb = (getRotatedPoint({    0.f,    0.f }, rotateAngle) + pos + _origin) / _viewSize;
    // vec2f rb = (getRotatedPoint({ size.x,    0.f }, rotateAngle) + pos + _origin) / _viewSize;
    // vec2f rt = (getRotatedPoint({ size.x, size.y }, rotateAngle) + pos + _origin) / _viewSize;
    // vec2f lt = (getRotatedPoint({    0.f, size.y }, rotateAngle) + pos + _origin) / _viewSize;

    int layer = static_cast<int>(texType);
    _rectangleArray.push_back({
        { lb, {0.f, texCoord.y},        color, layer },
        { rb, {texCoord.x, texCoord.y}, color, layer },
        { rt, {texCoord.x, 0.f},        color, layer },
        { lt, {0.f, 0.f},               color, layer }
    });
}

void Renderer::draw() {
    updateBuffer();
    glUseProgram(_shaderProgram);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(_vao);
    glDrawElements(
        GL_TRIANGLES, _drawingIndices + _staticDrawingIndices + _offsetSemistaticIndices, 
        GL_UNSIGNED_INT, 0
    );

    _rectangleArray.clear();
    _drawingIndices = 0;
    glBindVertexArray(0);
    glUseProgram(0);
}