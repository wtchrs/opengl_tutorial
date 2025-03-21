#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aOffset; // xz values: position offset, y value: rotation

uniform mat4 transform;

out vec2 texCoord;

void main() {
    float c = cos(aOffset.y);
    float s = sin(aOffset.y);
    mat4 offsetTransform = mat4(
            c,         0.0, -s,        0.0,
            0.0,       1.0, 0.0,       0.0,
            s,         0.0, c,         0.0,
            aOffset.x, 0.0, aOffset.z, 1.0
    );
    gl_Position = transform * offsetTransform * vec4(aPos, 1.0);
    texCoord = aTexCoord;
}
