#version 330 core

in vec4 vertexColor;
in vec2 texCoord;

uniform sampler2D tex;
uniform float gamma;

out vec4 fragColor;

void main() {
    vec4 pixel = texture2D(tex, texCoord);
    // Gamma correction
    fragColor = vec4(pow(pixel.rgb, vec3(gamma)), 1.0);
}
