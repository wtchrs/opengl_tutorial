#version 330 core

layout (location = 0) in vec3 aPos;
uniform mat4 transform;
out vec3 texCoord;

void main() {
    gl_Position = transform * vec4(aPos, 1.0);
    texCoord = aPos;
}
