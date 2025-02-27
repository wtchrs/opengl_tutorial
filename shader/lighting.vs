#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 transform;
uniform mat4 modelTransform;

out vec2 texCoord;
out vec3 normal;
out vec3 position;

void main() {
    gl_Position = transform * vec4(aPos, 1.0);
    texCoord = aTexCoord;
    // Normal vector and position in world space is needed to calculate diffuse light.
    // `transpose(inverse(modelTransform))` may be replaced with separate uniform variable.
    normal = (transpose(inverse(modelTransform)) * vec4(aNormal, 0.0)).xyz;
    position = (modelTransform * vec4(aPos, 1.0)).xyz;
}
