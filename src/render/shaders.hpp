namespace shaders {
    constexpr const char* vertexShaderSource = 
R"(#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;
layout (location = 3) in int aIndex;

out vec2 fTexCoord;
out vec4 fColor;
flat out int layerIndex;

void main() {
    gl_Position = vec4(aPos, 1.0);
    fTexCoord = aTexCoord;
    fColor = aColor;
    layerIndex = aIndex;
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
    vec4 texColor = texture(uTexArray, vec3(fTexCoord, float(layerIndex)));
    if (texColor.w < 0.1)
        discard;
    FragColor = texColor * fColor;
}
)";
}