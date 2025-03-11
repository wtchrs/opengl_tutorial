#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;
in vec3 tangent;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform int blinn;

uniform sampler2D diffuse;
uniform sampler2D normalMap;

out vec4 fragColor;

void main() {
    vec3 texColor = texture2D(diffuse, texCoord).xyz;
    vec3 texNorm = normalize(texture2D(normalMap, texCoord).xyz * 2.0 - 1.0);
    vec3 N = normalize(normal);
    vec3 T = normalize(tangent);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    vec3 pixelNorm = normalize(TBN * texNorm);

    vec3 ambient = texColor * 0.2;

    vec3 lightDir = normalize(lightPos - position);
    float diff = max(0.0, dot(pixelNorm, lightDir));
    vec3 diffuse = diff * texColor * 0.8;

    vec3 viewDir = normalize(viewPos - position);
    float spec;
    if (blinn == 0) {
        vec3 reflectDir = reflect(-lightDir, pixelNorm);
        spec = pow(max(0.0, dot(viewDir, reflectDir)), 32.0);
    } else {
        vec3 halfDir = normalize(lightDir + viewDir);
        spec = pow(max(0.0, dot(halfDir, pixelNorm)), 32.0);
    }
    vec3 specular = spec * vec3(0.5);

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}
