namespace shaders {
    constexpr const char* vertSource = R"(#version 330 core
    uniform mat4 model;
    uniform mat4 projection;

    layout (location = 0) in vec2 vertexPos;

    void main() {
        gl_Position = projection * model * vec4(vertexPos.xy, 0.0, 1.0);
    }
    )";

    constexpr const char* fragSource = R"(#version 330 core
    out vec4 FragColor;

    uniform vec3 color;

    void main() {
        FragColor = vec4(color.xyz, 1.0);
    }
    )";

    constexpr const char* vertSrcTex = R"(#version 330 core
    uniform mat4 modell;
    uniform mat4 projectionn;

    layout (location = 0) in vec2 vPos;
    layout (location = 1) in vec2 vtCoord;

    out vec2 tCoord;

    void main() {
        gl_Position = projectionn * modell * vec4(vPos.xy, 0.0, 1.0);
        tCoord = vtCoord;
    }
    )";

    constexpr const char* fragSrcTex = R"(#version 330 core
    uniform sampler2D uTexture;
    uniform vec2 texScale;

    in vec2 tCoord;
    out vec4 FragColor;

    void main() {
        FragColor = texture(uTexture, tCoord.xy * texScale.xy);
    }
    )";
}