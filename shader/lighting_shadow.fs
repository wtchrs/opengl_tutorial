#version 330 core

struct Light {
    vec3 position;
    vec3 attenuation; // K_c, K_l, K_q

    int directional;
    vec3 direction;
    vec2 cutoff; // cos(inner cutoff angle), cos(outer offset angle)

    // Phong reflection model (Local illumination model) parameter
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D diffuse; // Use texture as diffuse map.
    sampler2D specular; // Use texture as specular map.
    float shininess;
};

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
    vec4 fragPosLight;
} fsIn;

uniform Light light;
uniform Material material;
uniform vec3 viewPos;
uniform int blinn;
uniform sampler2D shadowMap;

out vec4 fragColor;

float computeShadow(vec4 fragPosLight, vec3 normal, vec3 lightDir) {
    // Transform from canonical coordinates to [0, 1] range.
    vec3 projCoord = fragPosLight.xyz / fragPosLight.w * 0.5 + 0.5;
    // float bias = max(0.0005, 0.005 * (1.0 - dot(normal, lightDir)));
    float bias = 0.0;
    float currentDepth = projCoord.z;

    // PCF (Percentage Closer Filtering)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture2D(shadowMap, projCoord.xy + vec2(x, y) * texelSize * 1.5).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;

    // float closestDepth = texture2D(shadowMap, projCoord.xy).r;
    // return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

void main() {
    vec3 texColor = texture2D(material.diffuse, fsIn.texCoord).xyz;
    vec3 specColor = texture2D(material.specular, fsIn.texCoord).xyz;

    vec3 lightDir;
    float attenuation = 1.0;
    float intensity = 1.0;

    if (light.directional == 1) {
        lightDir = normalize(-light.direction);
    } else {
        // attenuation
        float dist = length(light.position - fsIn.fragPos);
        vec3 distPoly = vec3(1.0, dist, dist * dist);
        attenuation = 1.0 / dot(distPoly, light.attenuation);
        // light direction
        lightDir = (light.position - fsIn.fragPos) / dist; // normalize
        // intensity
        vec3 lightDir = (light.position - fsIn.fragPos) / dist; // normalize
        float cosTheta = dot(lightDir, normalize(-light.direction));
        intensity = clamp((cosTheta - light.cutoff.y) / (light.cutoff.x - light.cutoff.y), 0.0, 1.0);
    }

    // Ambient light
    vec3 result = texColor * light.ambient;

    vec3 pixelNorm = normalize(fsIn.normal);
    float shadow = computeShadow(fsIn.fragPosLight, pixelNorm, lightDir);

    // Apply diffuse and specular light if the fragment is in the light cone.
    if (intensity > 0.0 && shadow < 1.0) {
        // Diffuse light
        float diff = max(dot(pixelNorm, lightDir), 0.0);
        vec3 diffuse = diff * texColor * light.diffuse;

        // Specular light
        float spec = 0.0;
        vec3 viewDir = normalize(viewPos - fsIn.fragPos);
        if (blinn == 0) {
            // Phong shading
            vec3 reflectDir = reflect(-lightDir, pixelNorm);
            spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        } else {
            // Blinn-Phong shading
            vec3 halfDir = normalize(lightDir + viewDir);
            spec = pow(max(dot(halfDir, pixelNorm), 0.0), material.shininess);
        }
        vec3 specular = spec * specColor * light.specular;

        // Multiply intensity to diffuse and specular light for smooth border.
        result += (diffuse + specular) * intensity * (1.0 - shadow);
    }

    // Apply attenuation.
    result *= attenuation;

    fragColor = vec4(result, 1.0f);
    // fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}
