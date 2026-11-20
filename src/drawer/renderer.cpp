#include "renderer.hpp"

#include "resources/textures/apple.hpp"
#include "resources/textures/body.hpp"
#include "resources/textures/corner.hpp"
#include "resources/textures/field.hpp"
#include "resources/textures/tail.hpp"
#include "resources/textures/eye_orbit.hpp"
#include "resources/textures/eye.hpp"
#include "resources/textures/play.hpp"
#include "resources/textures/pravednov.hpp"
#include "resources/textures/star.hpp"
#include "resources/textures/alphabet.hpp"

#include "rectangle.hpp"
#include "../core/logger.hpp"
#include "shaders.hpp"

#include <glad/glad.h>

#include <cstddef>

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
}

void Renderer::init() {
    constexpr int STATIC_RECTANGLES = 1000;
    constexpr int DYNAMIC_RECTANGLES = 1600;
    constexpr int STREAM_RECTANGLES = 2000;
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
    genBuffer(_staticBuffer.vbo, GL_STATIC_DRAW, STATIC_RECTANGLES);
    genBuffer(_dynamicBuffer.vbo, GL_DYNAMIC_DRAW, DYNAMIC_RECTANGLES);
    genBuffer(_streamBuffer.vbo, GL_STREAM_DRAW, STREAM_RECTANGLES);

    // setting VAOs
    genVertexArray(_staticBuffer.vao, _staticBuffer.vbo, _ebo);
    genVertexArray(_dynamicBuffer.vao, _dynamicBuffer.vbo, _ebo);
    genVertexArray(_streamBuffer.vao, _streamBuffer.vbo, _ebo);

    // setting textures
    constexpr GLsizei TEX_SIZE = 128;
    constexpr GLsizei LAYER_COUNT = 11;
    constexpr const unsigned char* TEX_DATA_ARRAY[LAYER_COUNT] = {
        &field[0],
        &apple[0],
        &body[0],
        &tail[0],
        &corner[0],
        &eye_orbit[0],
        &eye[0],
        &play[0],
        &pravednov[0],
        &star[0],
        &alphabet[0]
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
    glEnable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

Renderer::Renderer()
    : _shaderProgram(0)
    , _bindedBuffer(nullptr)
{
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, shaders::vertexShaderSource);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, shaders::fragmentShaderSource);

    if (vertexShader && fragmentShader)
        _shaderProgram = linkProgram(vertexShader, fragmentShader);

    if (_shaderProgram)
        init();
}

Renderer::~Renderer() {
    if (_ebo) glDeleteBuffers(1, &_ebo);
    if (_shaderProgram) glDeleteProgram(_shaderProgram);
}

void Renderer::bindBuffer(const BufferType bufferType) {
    switch (bufferType) {
        case BufferType::STATIC: _bindedBuffer = &_staticBuffer; break;
        case BufferType::DYNAMIC: _bindedBuffer = &_dynamicBuffer; break;
        case BufferType::STREAM: _bindedBuffer = &_streamBuffer; break;
    }
}

void Renderer::refreshBuffer(const BufferType buffer) {
    switch (buffer) {
        case BufferType::STATIC: _staticBuffer.refresh(); break;
        case BufferType::DYNAMIC: _dynamicBuffer.refresh(); break;
        case BufferType::STREAM: _streamBuffer.refresh(true); break;
    }
}

void Renderer::setBufferDrawStatus(const BufferType buffer, const bool draw) {
    switch (buffer) {
        case BufferType::STATIC: _staticBuffer.indices = draw ? _staticBuffer.data.size() * 6 : 0; break;
        case BufferType::DYNAMIC: _dynamicBuffer.indices = draw ? _bindedBuffer->data.size() * 6 : 0; break;
        case BufferType::STREAM: _streamBuffer.indices = draw ? _bindedBuffer->data.size() * 6 : 0; break;
    }
}

void Renderer::clearBuffer(const BufferType buffer) {
    switch (buffer) {
        case BufferType::STATIC:
            _staticBuffer.data.clear();
            _staticBuffer.indices = 0;
            break;
        case BufferType::DYNAMIC:
            _dynamicBuffer.data.clear();
            _dynamicBuffer.indices = 0;
            break;
        case BufferType::STREAM:
            _streamBuffer.data.clear();
            _streamBuffer.indices = 0;
            break;
    }
}

void Renderer::setClearColor(const vec4f color) { glClearColor(color.x, color.y, color.z, color.w); }

void Renderer::addObject(const Rectangle& rect) {
    _bindedBuffer->indices += 6;
    _bindedBuffer->data.push_back(rect);
}

void Renderer::draw() {
    glUseProgram(_shaderProgram);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _staticBuffer.draw();
    _dynamicBuffer.draw();
    _streamBuffer.draw();

    glBindVertexArray(0);
    glUseProgram(0);
}

Renderer::_Buffer::_Buffer(): vao(0), vbo(0), indices(0), data(0) {}

Renderer::_Buffer::~_Buffer() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

void Renderer::_Buffer::refresh(const bool newAllocate) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (newAllocate)
        glBufferData(GL_ARRAY_BUFFER, sizeof(Rectangle) * data.size(), data.data(), GL_STREAM_DRAW);
    else
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Rectangle) * data.size(), data.data());
    glBindVertexArray(0);
}

void Renderer::_Buffer::draw() {
    if (indices) {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, 0);
    }
}
