#pragma once
#include "vector.hpp"

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