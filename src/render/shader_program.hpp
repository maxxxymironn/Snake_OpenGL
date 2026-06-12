#pragma once
#include <glad/glad.h>

class ShaderProgram {
    GLuint ID;
    bool createSuccess;

    GLuint createShader(const GLenum typeShader, const char*& shaderSource, int& success, const bool isVertexShader);
    void createProgram(const GLenum vertShaderID, const GLenum fragShaderID, int& success);
    
public:
    ShaderProgram(const char* vertSource, const char* fragSource);
    ~ShaderProgram() { glDeleteProgram(ID); }

    bool getSuccessInfo() const { return createSuccess; }

    void use() { glUseProgram(ID); }

    GLuint getID() const { return ID; }
};