namespace shaders {
    constexpr const char* vertexShaderSource = 
R"(#version 330 core

layout (location = 0) in vec2 vPosition;
layout (location = 1) in vec2 vTexCoord;
layout (location = 2) in vec4 vColor;
layout (location = 3) in int vIndex;

out vec2 fTexCoord;
out vec4 fColor;
flat out int layerIndex;

void main() {
    gl_Position = vec4(vPosition, 0.0, 1.0);
    fTexCoord = vTexCoord;
    fColor = vColor;
    layerIndex = vIndex;
}
)";

    constexpr const char* fragmentShaderSource = 
R"(#version 330 core

uniform sampler2DArray uTexArray;

in vec2 fTexCoord;
in vec4 fColor;
flat in int layerIndex;

out vec4 FragColor;

void main() {
    FragColor = texture(uTexArray, vec3(fTexCoord, float(layerIndex))) * fColor;
}
)";
}