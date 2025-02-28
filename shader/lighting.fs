#version 330 core

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D diffuse; // Use texture as diffuse map.
    sampler2D specular; // Use texture as specular map.
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
    vec3 texColor = texture2D(material.diffuse, texCoord).xyz;
    vec3 specColor = texture2D(material.specular, texCoord).xyz;
    // Ambient light
    vec3 ambient = texColor * light.ambient;
    // Diffuse light
    vec3 lightDir = normalize(light.position - position);
    vec3 pixelNorm = normalize(normal);
    float diff = max(dot(pixelNorm, lightDir), 0.0);
    vec3 diffuse = diff * texColor * light.diffuse;
    // Specular light
    vec3 viewDir = normalize(viewPos - position);
    vec3 reflectDir = reflect(-lightDir, pixelNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * specColor * light.specular;
    // Result light
    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0f);
}
