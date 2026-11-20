#pragma once

#include "rectangle.hpp"
#include "src/core/vector.hpp"

#include <vector>

enum class BufferType: char {
    STATIC,
    DYNAMIC,
    STREAM,
};

class Renderer {
  public:
    Renderer();
    ~Renderer();

    bool getInitializeInfo() const { return _shaderProgram; }

    void bindBuffer(const BufferType buffer);
    void refreshBuffer(const BufferType buffer);
    void setBufferDrawStatus(const BufferType buffer, const bool draw);
    void clearBuffer(const BufferType buffer);

    void setClearColor(const vec4f color);

    void addObject(const Rectangle& rect);
    void draw();

  private:
    unsigned int _shaderProgram;
    unsigned int _ebo;
    unsigned int _textureArray;

    struct _Buffer {
        unsigned int vao;
        unsigned int vbo;
        unsigned int indices;
        std::vector<Rectangle> data;

        _Buffer();
        ~_Buffer();

        void refresh(const bool newAllocate=false);
        void draw();
    };

    _Buffer _staticBuffer;
    _Buffer _dynamicBuffer;
    _Buffer _streamBuffer;
    _Buffer* _bindedBuffer;

    void init();
};
