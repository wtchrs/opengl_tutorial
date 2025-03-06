#version 330 core

in vec3 normal;
in vec3 position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

out vec4 fragColor;

void main() {
    vec3 eye = normalize(position - cameraPos);
    vec3 r = reflect(eye, normalize(normal));
    fragColor = vec4(textureCube(skybox, r).rgb, 1.0);
}
