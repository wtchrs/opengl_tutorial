#version 330 core

in vec3 localPos;
uniform sampler2D tex;
out vec4 fragColor;

const vec2 invPi = vec2(0.1591549, 0.3183098862);

vec2 sampleSphericalMap(vec3 v) {
    return vec2(atan(v.z, v.x), asin(v.y)) * invPi + 0.5;
}

void main() {
    vec2 uv = sampleSphericalMap(normalize(localPos));
    vec3 color =  texture2D(tex, uv).rgb;
    fragColor = vec4(color, 1.0);
}
