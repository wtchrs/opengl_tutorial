#version 330 core

in vec3 localPos;
uniform samplerCube cubeMap;
out vec4 fragColor;

const float PI = 3.14159265359;

void main() {
    vec3 normal = normalize(localPos);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up = cross(normal, right);
    // if (normal.y == 1.0) {
    //     right = vec3(1.0, 0.0, 0.0);
    //     up = vec3(0.0, 0.0, -1.0);
    // }

    vec3 irradiance = vec3(0.0);
    float sampleDelta = 0.025;
    int nrSamples = 0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // Transform spherical to cartesian in tangent space.
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // Transform tangent space to world space.
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
            // Product the factors represents the Lambertian cosine law(cos(theta)) and differential solid angle(sin(theta)).
            irradiance += textureCube(cubeMap, sampleVec).rgb * cos(theta) * sin(theta); 
            nrSamples++;
        }
    }
    irradiance = PI * irradiance / float(nrSamples);
    fragColor = vec4(irradiance, 1.0);
}
