#version 330 core

in vec3 texCoord;
uniform samplerCube skybox;
out vec4 fragColor;

void main() {
    fragColor = textureCube(skybox, texCoord);
}
