#pragma once
#include "vector.hpp"
#include "../render/texture_enum.hpp"

struct Vertex {
    vec2f pos;
    vec2f texCoord;
    vec4f color;
    int texLayer;
};

struct Rectangle {
    Vertex lb;
    Vertex rb;
    Vertex rt;
    Vertex lt;
};

struct rectangleData {
    vec2f size;
    vec2f pos;
    vec4f color;
    vec4f texCoord;
    TexType texType;
    float rotateAngle;

    rectangleData() = default;
    rectangleData(
        vec2f size, vec2f pos, vec4f color, vec4f texCoord, 
        TexType texType, float rotateAngle
    ) : size(size), pos(pos), color(color), texCoord(texCoord),
        texType(texType), rotateAngle(rotateAngle) {}
};