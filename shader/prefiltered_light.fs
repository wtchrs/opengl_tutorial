#version 330 core

in vec3 localPos;
uniform samplerCube cubeMap;
uniform float roughness;
out vec4 fragColor;

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

float distributionGGX(vec3 normal, vec3 halfDir, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float dotNH = max(0.0, dot(normal, halfDir));
    float dotNH2 = dotNH * dotNH;

    float num = a2;
    float denom = (dotNH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

void main() {
    vec3 N, R, V; // normal, reflection, view
    N = R = V = normalize(localPos);

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);
    // Monte Carlo Integration for each sample.
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = hammersley(i, SAMPLE_COUNT); // generate random point.
        vec3 H = importanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        float dotNL = max(0.0, dot(N, L));
        // Integrate.
        if (dotNL > 0.0) {
            float D = distributionGGX(N, H, roughness);
            float dotNH = max(0.0, dot(N, H));
            float dotHV = max(0.0, dot(H, V));
            float pdf = (D * dotNH / (4.0 * dotHV)) + 0.0001;
            float resolution = 512; // Resolution of the face of source cube map.
            float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            prefilteredColor += textureCubeLod(cubeMap, L, mipLevel).rgb * dotNL;
            totalWeight += dotNL;
        }
    }
    prefilteredColor /= totalWeight;

    fragColor = vec4(prefilteredColor, 1.0);
}
