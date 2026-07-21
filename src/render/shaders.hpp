namespace shaders {
    constexpr const char* vertSrcTex = 
R"(#version 330 core
uniform mat4 uModel;
uniform mat4 uProjection;

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 vTexPos;

out vec2 texPos;

void main() {
    gl_Position = uProjection * uModel * vec4(vPos.xy, 0.0, 1.0);
    texPos = vTexPos;
}
)";

    constexpr const char* fragSrcTex = 
R"(#version 330 core
uniform sampler2D uTex;
uniform vec2 uTexScale;
uniform vec4 uColor;

in vec2 texPos;
out vec4 FragColor;

void main() {
    FragColor = texture(uTex, texPos.xy * uTexScale.xy) * uColor;
}
)";
}