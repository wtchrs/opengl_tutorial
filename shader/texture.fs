#version 330 core

in vec4 vertexColor;
in vec2 texCoord;
out vec4 fragColor;

uniform sampler2D tex1;
uniform sampler2D tex2;

void main() {
    vec4 color1 = texture2D(tex1, texCoord);
    vec4 color2 = texture2D(tex2, texCoord);
    fragColor = color1 * (1 - color2.a) + color2;
}
