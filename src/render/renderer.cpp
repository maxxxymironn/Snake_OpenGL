#include "renderer.hpp"

#include "../../resources/textures/apple.hpp"
#include "../../resources/textures/body.hpp"
#include "../../resources/textures/corner.hpp"
#include "../../resources/textures/field.hpp"
#include "../../resources/textures/tail.hpp"
#include "../../resources/textures/eye_orbit.hpp"
#include "../../resources/textures/eye.hpp"


#include "../config/draw_config.hpp"
#include "../core/rectangle.hpp"
#include "../core/logger.hpp"
#include "shaders.hpp"
#include "texture_enum.hpp"

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

    void genBuffer(unsigned int& vbo, const GLenum drawType, const int rectangleCount) {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, rectangleCount * sizeof(Rectangle), nullptr, drawType);
    }

    void genVertexArray(unsigned int& vao, const unsigned int vbo, const unsigned int ebo) {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

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
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(
            3, 1, 
            GL_INT, 
            sizeof(Vertex), (void*)(offsetof(Vertex, texLayer))
        );
    }

    vec2f getRotatedPoint(const vec2f point, const float rotateAngle) {
        return {
            point.x * cosf(rotateAngle) - point.y * sinf(rotateAngle),
            point.x * sinf(rotateAngle) + point.y * cosf(rotateAngle)
        };
    }
}

void Renderer::init() {
    constexpr int STATIC_RECTANGLES = 1000;
    constexpr int DYNAMIC_RECTANGLES = 1600;
    constexpr int STREAM_RECTANGLES = 30;
    constexpr int MAX_RECTANGLES = STATIC_RECTANGLES + DYNAMIC_RECTANGLES + STREAM_RECTANGLES;

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

    // setting VBOs
    genBuffer(_staticVBO, GL_STATIC_DRAW, STATIC_RECTANGLES);
    genBuffer(_dynamicVBO, GL_DYNAMIC_DRAW, DYNAMIC_RECTANGLES);
    genBuffer(_streamVBO, GL_STREAM_DRAW, STREAM_RECTANGLES);

    // setting VAOs
    genVertexArray(_staticVAO, _staticVBO, _ebo);
    genVertexArray(_dynamicVAO, _dynamicVBO, _ebo);
    genVertexArray(_streamVAO, _streamVBO, _ebo);

    // setting textures
    constexpr GLsizei TEX_SIZE = 128;
    constexpr GLsizei LAYER_COUNT = 7;
    constexpr const unsigned char* TEX_DATA_ARRAY[LAYER_COUNT] = {
        &field[0],
        &apple[0],
        &body[0],
        &tail[0],
        &corner[0],
        &eye_orbit[0],
        &eye[0]
    };

    glGenTextures(1, &_textureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _textureArray);
    
    glUseProgram(_shaderProgram);

    glUniform1i(glGetUniformLocation(_shaderProgram, "uTexArray"), 0);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

void Renderer::refreshStaticBuffer() {
    glBindVertexArray(_staticVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _staticVBO);
    glBufferSubData(
        GL_ARRAY_BUFFER, 0, 
        sizeof(Rectangle) * _rectangleArray.size(), _rectangleArray.data()
    );
    glBindVertexArray(0);

    _rectangleArray.clear();
    _staticDrawingIndices = _drawingIndices;
    _drawingIndices = 0;
    _needRefreshStaticBuffer = false;
}

void Renderer::refreshDynamicBuffer() {
    glBindVertexArray(_dynamicVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _dynamicVBO);
    glBufferSubData(
        GL_ARRAY_BUFFER, 0,
        sizeof(Rectangle) * _rectangleArray.size(), _rectangleArray.data()
    );
    glBindVertexArray(0);

    _rectangleArray.clear();
    _dynamicDrawingIndices = _drawingIndices;
    _drawingIndices = 0;
}

void Renderer::refreshStreamBuffer() {
    glBindVertexArray(_streamVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _streamVBO);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(Rectangle) * _drawingIndices, 
        nullptr, GL_STREAM_DRAW
    );
    glBufferSubData(
        GL_ARRAY_BUFFER, 0, 
        sizeof(Rectangle) * _rectangleArray.size(), _rectangleArray.data()
    );
    glBindVertexArray(0);

    _rectangleArray.clear();
    _streamDrawingIndices = _drawingIndices;
    _drawingIndices = 0;
}

Renderer::Renderer()
    : _shaderProgram(0), 
      _zenMode(DrawConfig::zenMode),
      _needRefreshStaticBuffer(false),
      _staticVAO(0), _staticVBO(0),
      _dynamicVAO(0), _dynamicVBO(0),
      _streamVAO(0), _streamVBO(0),
      _drawingIndices(0),
      _staticDrawingIndices(0),
      _dynamicDrawingIndices(0),
      _streamDrawingIndices(0),
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
    if (_staticVAO) glDeleteVertexArrays(1, &_staticVAO);
    if (_staticVBO) glDeleteBuffers(1, &_staticVBO);

    if (_dynamicVAO) glDeleteVertexArrays(1, &_dynamicVAO);
    if (_dynamicVBO) glDeleteBuffers(1, &_dynamicVBO);

    if (_streamVAO) glDeleteVertexArrays(1, &_streamVAO);
    if (_streamVBO) glDeleteBuffers(1, &_streamVBO);
    
    if (_ebo) glDeleteBuffers(1, &_ebo);
    if (_shaderProgram) glDeleteProgram(_shaderProgram);
}

void Renderer::addObject(
    vec2f size, vec2f pos, const TexType texType, 
    const vec4f texCoord, const vec4f color, const float rotateAngle
) {
    _drawingIndices += 6;
    
    size *= _contentScale * 0.5f;
    pos *= _contentScale;
    vec2f lb = (getRotatedPoint({-size.x,-size.y }, rotateAngle) + pos + _origin) / _viewSize;
    vec2f rb = (getRotatedPoint({ size.x,-size.y }, rotateAngle) + pos + _origin) / _viewSize;
    vec2f rt = (getRotatedPoint({ size.x, size.y }, rotateAngle) + pos + _origin) / _viewSize;
    vec2f lt = (getRotatedPoint({-size.x, size.y }, rotateAngle) + pos + _origin) / _viewSize;

    int layer = static_cast<int>(texType);
    _rectangleArray.push_back({
        { lb, {texCoord.x, texCoord.w}, color, layer },
        { rb, {texCoord.z, texCoord.w}, color, layer },
        { rt, {texCoord.z, texCoord.y}, color, layer },
        { lt, {texCoord.x, texCoord.y}, color, layer }
    });
}

void Renderer::draw() {
    glUseProgram(_shaderProgram);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(_staticVAO);
    glDrawElements(GL_TRIANGLES, _staticDrawingIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(_dynamicVAO);
    glDrawElements(GL_TRIANGLES, _dynamicDrawingIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(_streamVAO);
    glDrawElements(GL_TRIANGLES, _streamDrawingIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}