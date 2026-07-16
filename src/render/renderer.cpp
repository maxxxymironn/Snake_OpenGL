#include "renderer.hpp"

#include "shaders.hpp"
#include "shader_program.hpp"
#include "quad_vertices.hpp"
#include "textures/field_texture.hpp"
#include "textures/apple_texture.hpp"
#include "textures/pause_texture.hpp"
#include "textures/snake_body_texture.hpp"
#include "textures/snake_head_texture.hpp"
#include "textures/snake_tail_texture.hpp"
#include "textures/snake_turn_texture.hpp"
#include "textures/tail-tail_texture.hpp"
#include "textures/cap_texture.hpp"
#include "textures/tail-corner_texture.hpp"
#include "textures/eye_texture.hpp"
#include "textures/eye-point_texture.hpp"

#include <glad/glad.h>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

Renderer::Renderer(): successShaderCompilation(false) {
    shaderProgram = std::make_unique<ShaderProgram>(shaders::vertSource, shaders::fragSource);
    shaderTexProgram = std::make_unique<ShaderProgram>(shaders::vertSrcTex, shaders::fragSrcTex);

    if(!shaderProgram->getSuccessInfo()) {
        std::cout << "ShaderProgram creation failed" << std::endl;

        if (!shaderTexProgram->getSuccessInfo())
            std::cout << "ShaderTexProgram creation failed" << std::endl;
    }
    else if (!shaderTexProgram->getSuccessInfo()) {
        std::cout << "ShaderTexProgram creation failed" << std::endl;
    }
    else {
        init();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        successShaderCompilation = true;
    }    
}

Renderer::~Renderer() {
    if (vaoID) glDeleteVertexArrays(1, &vaoID);
    if (vaoTexID) glDeleteVertexArrays(1, &vaoTexID);
    if (vboID) glDeleteBuffers(1, &vboID);
    if (eboID) glDeleteBuffers(1, &eboID);
}

void Renderer::init() {
    glGenVertexArrays(1, &vaoID);
    glGenVertexArrays(1, &vaoTexID);
    glGenBuffers(1, &vboID);

    // Create EBO
    glGenBuffers(1, &eboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices, GL_STATIC_DRAW);

    // Allocate memory for common cells
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    /* Setting VAO */
    glBindVertexArray(vaoID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    /* End VAO */

    /* Setting TexVAO */
    glBindVertexArray(vaoTexID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    /* End TexVAO */

    generateTextures();

    /* Setting uniforms */
    GLuint programID;
    // projection
    programID = shaderProgram->getID();
    shaderProgram->use();
    glm::mat4 projection = glm::ortho(
        0.0f, 2.f,
        0.0f, 2.f
    );
    glUniformMatrix4fv(glGetUniformLocation(programID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    modelLoc = glGetUniformLocation(programID, "model");
    colorLoc = glGetUniformLocation(programID, "color");

    // texture
    programID = shaderTexProgram->getID();
    shaderTexProgram->use();
    glUniform1i(glGetUniformLocation(programID, "uTexture"), 0);
    glUniformMatrix4fv(glGetUniformLocation(programID, "projectionn"), 1, GL_FALSE, glm::value_ptr(projection));
    modellLoc = glGetUniformLocation(programID, "modell");
    texCoordLoc = glGetUniformLocation(programID, "texScale");

    glUseProgram(0);
    /* End uniforms */
}

void Renderer::generateTextures() {
    // field texture
    GLsizei fieldTexWidth = 80;
    GLsizei fieldTexHeight = 80;
    bool isRepeatingWrap = true;
    generateTextureObject(fieldTex, fieldPixels, fieldTexWidth, fieldTexHeight, isRepeatingWrap);

    // apple texture 
    GLsizei appleTexWidth = 48;
    GLsizei appleTexHeight = 48;
    generateTextureObject(appleTex, applePixels, appleTexWidth, appleTexHeight);

    // pause texture
    GLsizei pauseTexWidth = 40;
    GLsizei pauseTexHeight = 200;
    generateTextureObject(pauseTex, pausePixels, pauseTexWidth, pauseTexHeight);

    // snake texture
    GLsizei snakePartTexSize = 40;
    generateTextureObject(snakeBodyTex, snakeBodyPixels, snakePartTexSize, snakePartTexSize);
    generateTextureObject(snakeHeadTex, snakeHeadPixels, snakePartTexSize, snakePartTexSize);
    generateTextureObject(snakeTailTex, snakeTailPixels, snakePartTexSize, snakePartTexSize);
    generateTextureObject(snakeTurnTex, snakeTurnPixels, snakePartTexSize, snakePartTexSize);
    // extra snake texture
    generateTextureObject(tailTailTex, tailTailPixels, snakePartTexSize, snakePartTexSize);
    generateTextureObject(capTex, capPixels, snakePartTexSize, snakePartTexSize);
    generateTextureObject(tailCornerTex, TailCornerPixels, snakePartTexSize, snakePartTexSize);
    GLsizei snakeEyeTexSize = 13;
    generateTextureObject(eyeTex, eyePixels, snakeEyeTexSize, snakeEyeTexSize);
    generateTextureObject(eyePointTex, eyePointPixels, snakeEyeTexSize, snakeEyeTexSize);
}

void Renderer::generateTextureObject(
        GLuint& texture, 
        const unsigned char textureArray[], 
        GLsizei textureWidth, 
        GLsizei textureHeight,
        bool isRepeatingWrap
    ) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // setting repeat texture (that was GL_CLAMP_TO_BORDER)
    if (!isRepeatingWrap) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    // setting filter texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        textureWidth,
        textureHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        textureArray
    );
}

void Renderer::beginFrame() { glClear(GL_COLOR_BUFFER_BIT); }

void Renderer::endFrame() {
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::useDefaultProgram() {
    shaderProgram->use();
    glBindVertexArray(vaoID);
}

void Renderer::useTextureProgram() {
    shaderTexProgram->use();
    glBindVertexArray(vaoTexID);
}

void Renderer::drawField() {
    glm::mat4 model(1.f);
    model = glm::translate(model, glm::vec3(1.f,1.f,0.f));

    float texOverSize = 10.f;
    bool useBlend = false;
    drawObject(glm::value_ptr(model), useBlend, fieldTex, texOverSize);
}

void Renderer::drawApple(const float x, const float y, const float scale) {
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(
        x * NDCcellWidth + NDCcellWidth / 2.f,
        y * NDCcellHeight + NDCcellHeight / 2.f,
        0.f
    ));
    model = glm::scale(model, glm::vec3(NDCcellWidth / 2.f * scale));

    bool useBlend = true;
    drawObject(glm::value_ptr(model), useBlend, appleTex);
}

// TODO: set default snakeType = BODY
void Renderer::drawSnake(const float x, const float y, const SnakeType snakeType, const float rotateAngle) {
    GLuint snakePartTex;
    switch(snakeType) {
        case SnakeType::BODY: snakePartTex = snakeBodyTex; break;
        case SnakeType::HEAD: snakePartTex = snakeHeadTex; break;
        case SnakeType::TAIL: snakePartTex = snakeTailTex; break;
        case SnakeType::CORNER: snakePartTex = snakeTurnTex; break;
        case SnakeType::TAIL_TAIL: snakePartTex = tailTailTex; break;
        case SnakeType::CAP: snakePartTex = capTex; break;
        case SnakeType::TAIL_CORNER: snakePartTex = tailCornerTex; break;
    }

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(
        x * NDCcellWidth + NDCcellWidth / 2.f,
        y * NDCcellHeight + NDCcellHeight / 2.f,
        0.f
    ));
    model = glm::scale(model, glm::vec3(NDCcellWidth /  2.f));
    // rotate in radians (pi, 2pi, 3pi etc.)
    model = glm::rotate(model, rotateAngle, {0, 0, 1.f});

    bool useBlend = true;
    drawObject(glm::value_ptr(model), useBlend, snakePartTex);
}

void Renderer::drawEyes(const float x, const float y, const float eyeAngle, const float eyePointAngle) {
    constexpr bool useBlend = true;
    constexpr float xEyeOffsetPxl = 0.02;
    constexpr float yEyeOffsetPxl = 0.015;
    
    glm::mat4 model(1.f);
    model = glm::translate(model, glm::vec3(
        x * NDCcellWidth,
        y * NDCcellHeight,
        0.f
    ));
    model = glm::translate(model, glm::vec3(
        NDCcellHeight / 2.f,
        NDCcellHeight / 2.f,
        0.f
    ));
    model = glm::rotate(model, eyeAngle, {0.f, 0.f, 1.f});
    model = glm::translate(model, glm::vec3( 
        -NDCcellHeight / 2.f,
        -NDCcellHeight / 2.f,
        0.f
    )); 
    model = glm::translate(model, glm::vec3(xEyeOffsetPxl, yEyeOffsetPxl, 0.f));

    glm::mat4 model2(model);
    model2 = glm::translate(model2, glm::vec3(0.f, 0.07f, 0.f));

    model = glm::rotate(model, eyePointAngle, {0.f, 0.f, 1.f});
    model = scale(model, glm::vec3(NDCcellWidth / 6.154f));
    model2 = glm::rotate(model2, eyePointAngle, {0.f, 0.f, 1.f});
    model2 = scale(model2, glm::vec3(NDCcellWidth / 6.154f));
    drawObject(value_ptr(model), useBlend, eyeTex);
    drawObject(value_ptr(model2), useBlend, eyeTex);

    drawObject(value_ptr(model), useBlend, eyePointTex);
    drawObject(value_ptr(model2), useBlend, eyePointTex);
}

void Renderer::drawPause() {
    // left stick
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(
        8.f * NDCcellWidth + NDCcellWidth / 2.f,
        10.f * NDCcellWidth,
        0.f
    ));
    model = glm::scale(model, glm::vec3(0.05f, 0.25f, 0.f));

    // right stick
    glm::mat4 model2(1.0f);
    model2 = glm::translate(model2, glm::vec3(
        11.f * NDCcellWidth + NDCcellWidth / 2.f,
        10.f * NDCcellWidth,
        0.f
    ));
    model2 = glm::scale(model2, glm::vec3(0.05f, 0.25f, 0.f));

    bool useBlend = true;
    drawObject(glm::value_ptr(model), useBlend, pauseTex);
    drawObject(glm::value_ptr(model2), useBlend, pauseTex);
}

void Renderer::drawObject(const GLfloat* const modelPtr, const bool useBlend, const GLuint texture, const float textureSize) {
    glUniformMatrix4fv(modellLoc, 1, GL_FALSE, modelPtr);
    glUniform2f(texCoordLoc, textureSize, textureSize);

    if (useBlend) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    if (useBlend) {
        glDisable(GL_BLEND);
    }
}