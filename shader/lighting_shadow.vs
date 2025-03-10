#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 transform;
uniform mat4 modelTransform;
uniform mat4 lightTransform; // (projection * view) matrix in light position.

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
    vec4 fragPosLight;
} vsOut;

void main() {
    gl_Position = transform * vec4(aPos, 1.0);
    vsOut.fragPos = (modelTransform * vec4(aPos, 1.0)).xyz;
    vsOut.normal = (transpose(inverse(modelTransform)) * vec4(aNormal, 1.0)).xyz;
    vsOut.texCoord = aTexCoord;
    vsOut.fragPosLight = lightTransform * vec4(vsOut.fragPos, 1.0);
}
