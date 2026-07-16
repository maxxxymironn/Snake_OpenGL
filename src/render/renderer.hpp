#pragma once

#include "../game/snake_parts_enum.hpp"

#include <memory>

using GLuint = unsigned int;
using GLsizei = int;
using GLfloat = float;

class ShaderProgram;

class Renderer {
private:
    std::unique_ptr<ShaderProgram> shaderProgram;
    std::unique_ptr<ShaderProgram> shaderTexProgram;
    bool successShaderCompilation;

    // Objects
    GLuint vaoID = 0;
    GLuint vaoTexID = 0;
    GLuint vboID = 0;
    GLuint eboID = 0;

    // Uniforms
    GLuint modelLoc = 0;
    GLuint modellLoc = 0;
    GLuint colorLoc = 0;
    GLuint texCoordLoc = 0;

    // Textures
    GLuint fieldTex = 0;
    GLuint appleTex = 0;
    GLuint pauseTex = 0;
    GLuint snakeBodyTex = 0;
    GLuint snakeHeadTex = 0;
    GLuint snakeTailTex = 0;
    GLuint snakeTurnTex = 0;
    GLuint tailTailTex = 0;
    GLuint capTex = 0;
    GLuint tailCornerTex = 0;
    GLuint eyeTex = 0;
    GLuint eyePointTex = 0;
    
    // GLuint snakeAtlasTex = 0;

    float NDCcellWidth;
    float NDCcellHeight;

    void init();

    void generateTextures();
    void generateTextureObject(
        GLuint& texture, 
        const unsigned char textureArray[], 
        GLsizei textureWidth, 
        GLsizei textureHeight,
        bool isRepeatingWrap=false
    );

public:
    Renderer();
    ~Renderer();

    bool getInitializeInfo() const { return successShaderCompilation; }

    void setFieldSize(const int& width, const int& height) { NDCcellWidth = 2.f / width; NDCcellHeight = 2.f / height; };

    void beginFrame();
    void useDefaultProgram();
    void useTextureProgram();
    void endFrame();

    void drawField();
    void drawPause();
    void drawApple(const float x, const float y, const float scale);
    void drawSnake(const float x, const float y, const SnakeType snakeType, const float rotateAngle);
    void drawEyes(const float x, const float y, const float eyeAngle, const float eyePointAngle=0.f);

    void drawObject(const GLfloat* const modelPtr, const bool useBlend, const GLuint texture, const float textureSize=1.f);
};