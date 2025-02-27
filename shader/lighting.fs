#version 330 core

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec2 texCoord;
in vec3 normal;
in vec3 position;
out vec4 fragColor;

uniform vec3 viewPos;
uniform Light light;
uniform Material material;

void main() {
    // Ambient light
    vec3 ambient = material.ambient * light.ambient;
    // Diffuse light
    vec3 lightDir = normalize(light.position - position);
    vec3 pixelNorm = normalize(normal);
    float diff = max(dot(pixelNorm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * material.diffuse;
    // Specular light
    vec3 viewDir = normalize(viewPos - position);
    vec3 reflectDir = reflect(-lightDir, pixelNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * material.specular * light.specular;
    // Result light
    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0f);
}
