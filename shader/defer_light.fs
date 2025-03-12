#version 330 core

const int NR_LIGHTS = 32;

in vec2 texCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 viewPos;
uniform struct {
    vec3 position;
    vec3 color;
} lights[NR_LIGHTS];

out vec4 fragColor;

void main() {
    vec3 fragPos = texture2D(gPosition, texCoord).rgb;
    vec3 normal = texture2D(gNormal, texCoord).rgb;
    vec4 albedoSpec = texture2D(gAlbedoSpec, texCoord);
    vec3 albedo = albedoSpec.rgb;
    float spec = albedoSpec.a;

    vec3 lighting = albedo * 0.1; // hard-coded ambient component
    vec3 viewDir = normalize(viewPos - fragPos);
    for (int i = 0; i < NR_LIGHTS; ++i) {
        // diffuse
        vec3 lightDir = normalize(lights[i].position - fragPos);
        vec3 diffuse = max(0.0, dot(lightDir, normal)) * albedo * lights[i].color;
        // Blinn-Phong specular
        // vec3 halfDir = normalize(viewDir + lightDir);
        // vec3 specular = max(0.0, dot(halfDir, normal)) * spec * lights[i].color;
        // lighting += diffuse + specular;
        lighting += diffuse;
    }
    fragColor = vec4(lighting, 1.0);
}
