#version 330 core

in vec2 texCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform mat4 view;
uniform mat4 projection;

uniform sampler2D texNoise;
uniform vec2 noiseScale;
uniform float radius;
uniform float power;

const int KERNEL_SIZE = 16;
const float BIAS = 0.025;
uniform vec3 samples[KERNEL_SIZE];

out float fragColor; // single channel output

void main() {
    vec4 worldPos = texture2D(gPosition, texCoord);
    if (worldPos.w <= 0.0) {
        discard;
    }
    vec3 fragPos = (view * vec4(worldPos.xyz, 1.0)).xyz;
    vec3 normal = (view * vec4(texture2D(gNormal, texCoord).xyz, 0.0)).xyz;
    vec3 randomVec = texture2D(texNoise, texCoord * noiseScale).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Screen Space Ambient Occlusion
    float occlusion = 0.0;
    for (int i = 0; i < KERNEL_SIZE; ++i) {
        vec3 sample = fragPos + TBN * samples[i] * radius;
        vec4 screenSample = projection * vec4(sample, 1.0);
        screenSample /= screenSample.w;
        screenSample.xyz = screenSample.xyz * 0.5 + 0.5;
        float sampleDepth = (view * texture2D(gPosition, screenSample.xy)).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z + BIAS ? 1.0 : 0.0) * rangeCheck; 
    }

    occlusion = 1.0 - occlusion / KERNEL_SIZE;
    fragColor = pow(occlusion, power);
}
