#version 330 core

struct Light {
    vec3 position;
    vec3 attenuation; // K_c, K_l, K_q

    // spot light
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

    // Calculate attenuation
    float dist = length(light.position - position);
    vec3 distPoly = vec3(1.0, dist, dist * dist);
    float attenuation = 1.0 / dot(distPoly, light.attenuation);

    // Ambient light
    vec3 result = texColor * light.ambient;

    vec3 lightDir = (light.position - position) / dist; // normalize
    float cosTheta = dot(lightDir, normalize(-light.direction));
    float intensity = clamp((cosTheta - light.cutoff.y) / (light.cutoff.x - light.cutoff.y), 0.0, 1.0);

    // Apply diffuse and specular light if the fragment is in the light cone.
    if (intensity > 0.0) {
        // Diffuse light
        vec3 pixelNorm = normalize(normal);
        float diff = max(dot(pixelNorm, lightDir), 0.0);
        vec3 diffuse = diff * texColor * light.diffuse;

        // Specular light
        vec3 viewDir = normalize(viewPos - position);
        vec3 reflectDir = reflect(-lightDir, pixelNorm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = spec * specColor * light.specular;

        // Multiply intensity to diffuse and specular light for smooth border.
        result += (diffuse + specular) * intensity;
    }

    // Apply attenuation.
    result *= attenuation;
    fragColor = vec4(result, 1.0f);
}
