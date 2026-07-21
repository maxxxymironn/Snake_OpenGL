#include "shader_program.hpp"
#include "../core/logger.hpp"

ShaderProgram::ShaderProgram(const char* vertSource, const char* fragSource)
    : createSuccess(true) {
    int success = 0;

    /* Compile shaders */
    bool isVertexShader = true;
    GLuint vertShaderID = createShader(GL_VERTEX_SHADER, vertSource, success, isVertexShader);
    GLuint fragShaderID = createShader(GL_FRAGMENT_SHADER, fragSource, success, !isVertexShader);

    /* Create ShaderProgram */
    if (createSuccess)
        createProgram(vertShaderID, fragShaderID, success);
}

GLuint ShaderProgram::createShader(GLenum typeShader, const char*& shaderSource, int& success, const bool isVertexShader) {
    GLuint shaderID = glCreateShader(typeShader);
    glShaderSource(shaderID, 1, &shaderSource, nullptr);
    glCompileShader(shaderID);

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        Logger::getInstance().printError((isVertexShader ? "SHADER::VERTEX" : "SHADER::FRAGMENT"), "COMPILATION_FAILED");
        createSuccess = false;
    }

    return shaderID;
}

void ShaderProgram::createProgram(const GLenum vertShaderID, const GLenum fragShaderID, int& success) {
    ID = glCreateProgram();
    glAttachShader(ID, vertShaderID);
    glAttachShader(ID, fragShaderID);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        Logger::getInstance().printError("SHADER_PROGRAM", "LINK_FAILED");
        createSuccess = false;
    }

    /* shader objects not needed anymore */
    glDetachShader(ID, vertShaderID);
    glDetachShader(ID, fragShaderID);
    glDeleteShader(vertShaderID);
    glDeleteShader(fragShaderID);
}