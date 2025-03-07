#version 330 core

in vec2 texCoord;

uniform sampler2D tex;

out vec4 fragColor;

void main() {
    vec4 pixel = texture2D(tex, texCoord);
    if (pixel.a < 0.05) {
        discard;
    }
    fragColor = pixel;
}
