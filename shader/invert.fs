#version 330 core

in vec4 vertexColor;
in vec2 texCoord;

uniform sampler2D tex;

out vec4 fragColor;

void main() {
    vec4 pixel = texture2D(tex, texCoord);
    fragColor = vec4(1.0 - pixel.rgb, 1.0f);
}
