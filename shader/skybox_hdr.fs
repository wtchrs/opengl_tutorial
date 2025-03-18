#version 330 core

in vec3 localPos;
uniform samplerCube cubeMap;
out vec4 fragColor;

void main() {
    vec3 envColor = textureCube(cubeMap, localPos).rgb;
    envColor = envColor / (envColor + vec3(1.0)); // Reinhard tone mapping
    envColor = pow(envColor, vec3(1.0 / 2.2)); // Convert to SRGB
    fragColor = vec4(envColor, 1.0);
}
