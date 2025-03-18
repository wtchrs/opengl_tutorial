#version 330 core

in vec2 texCoord;
out vec2 fragColor;

const float PI = 3.14159265359;

float radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // 0x100000000
}

vec2 hammersley(uint i, uint N) {
    // Generate random point.
    return vec2(float(i) / float(N), radicalInverse_VdC(i));
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    // Calculate cartesian coordinates from spherical coordinates.
    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    // Calculate world-space sample vector from tangent-space vector.
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float geometrySchlickGGX(float dotNV, float roughness) {
    float a = roughness;
    float k = a * a / 2.0;
    float nom = dotNV;
    float denom = dotNV * (1.0 - k) + k;
    return nom / denom;
}

float geometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness) {
    float dotNV = max(0.0, dot(normal, viewDir));
    float dotNL = max(0.0, dot(normal, lightDir));
    float ggx2 = geometrySchlickGGX(dotNV, roughness);
    float ggx1 = geometrySchlickGGX(dotNL, roughness);
    return ggx1 * ggx2;
}

vec2 integrateBRDF(float dotNV, float roughness) {
    vec3 V = vec3(sqrt(1.0 - dotNV * dotNV), 0.0, dotNV);
    float A = 0.0;
    float B = 0.0;
    vec3 N = vec3(0.0, 0.0, 1.0);
    const uint SAMPLE_COUNT = 1024u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = hammersley(i, SAMPLE_COUNT);
        vec3 H = importanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float dotNL = max(0.0, L.z);
        float dotNH = max(0.0, H.z);
        float dotVH = max(0.0, dot(V, H));

        if (dotNL > 0.0) {
            float G = geometrySmith(N, V, L, roughness);
            float G_Vis = (G * dotVH) / (dotNH * dotNV);
            float Fc = pow(1.0 - dotVH, 5.0);
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    A /= float (SAMPLE_COUNT);
    B /= float (SAMPLE_COUNT);
    return vec2(A, B);
}

void main() {
    vec2 integratedBRDF = integrateBRDF(texCoord.x, texCoord.y);
    fragColor = integratedBRDF;
}
