#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform struct Material {
    sampler2D diffuse;
    sampler2D specular;
} material;

// gbuffers
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

void main() {
    gPosition = vec4(position, 1.0);
    gNormal = vec4(normalize(normal), 1.0);
    gAlbedoSpec.rgb = texture2D(material.diffuse, texCoord).rgb;
    gAlbedoSpec.a = texture2D(material.specular, texCoord).r;
}
