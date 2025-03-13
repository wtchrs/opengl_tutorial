#version 330 core

in vec2 texCoord;
uniform sampler2D tex;
out vec4 fragColor;

void main() {
    vec2 texelSize = 1.0 / textureSize(tex, 0);
    vec4 result = vec4(0.0);
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture2D(tex, texCoord + offset);
        }
    }
    fragColor = result / 25.0;
}
