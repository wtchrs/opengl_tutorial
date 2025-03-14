#version 330 core

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;

uniform vec3 viewPos;

const int LIGHT_COUNT = 4;
uniform struct {
    vec3 position;
    vec3 color;
} lights[LIGHT_COUNT];

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};
uniform Material material;

const float PI = 3.14159265359;

out vec4 fragColor;

float distributionGGX(vec3 normal, vec3 halfDir, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float dotNH = max(0.0, dot(normal, halfDir));
    float dotNH2 = dotNH * dotNH;
    float denom = dotNH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float geometrySchlickGGX(float dotNV, float roughness) {
    float r = roughness + 1.0;
    float k = r * r / 8.0;
    float num = dotNV;
    float denom = dotNV * (1.0 - k) + k;
    return num / denom;
}

float geometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness) {
    float dotNV = max(0.0, dot(normal, viewDir));
    float dotNL = max(0.0, dot(normal, lightDir));
    float ggx2 = geometrySchlickGGX(dotNV, roughness);
    float ggx1 = geometrySchlickGGX(dotNL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 albedo = material.albedo;
    float metallic = material.metallic;
    float roughness = material.roughness;
    float ao = material.ao;
    vec3 fragNormal = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);
    float dotNV = max(0.0, dot(fragNormal, viewDir));

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic); // Linear interpolation

    // Reflectance equation
    vec3 outRadiance = vec3(0.0);
    for (int i = 0; i < LIGHT_COUNT; ++i) {
        vec3 lightDir = normalize(lights[i].position - fragPos);
        vec3 halfDir = normalize(lightDir + viewDir);

        // Calculate per-light radiance
        float dist = length(lights[i].position - fragPos);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance = lights[i].color * attenuation;

        // Cook-Torrance BRDF
        float ndf = distributionGGX(fragNormal, halfDir, roughness);
        float geometry = geometrySmith(fragNormal, viewDir, lightDir, roughness);
        float cosTheta = max(0.0, dot(halfDir, viewDir));
        vec3 fresnel = fresnelSchlick(cosTheta, F0);

        vec3 kS = fresnel;
        vec3 kD = 1.0 - kS;
        kD *= (1.0 - metallic);

        float dotNL = max(0.0, dot(fragNormal, lightDir));
        vec3 numerator = ndf * geometry * fresnel;
        float denominator = 4.0 * dotNV * dotNL;
        vec3 specular = numerator / max(0.001, denominator);

        // Add to outgoing radiance Lo
        outRadiance += (kD * albedo / PI + specular) * radiance * dotNL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + outRadiance;

    // Reinhard tone mapping (HDR) + gamma correction
    color = color / (color + 1.0);
    color = pow(color, vec3(1.0 / 2.2));

    fragColor = vec4(color, 1.0);
}
