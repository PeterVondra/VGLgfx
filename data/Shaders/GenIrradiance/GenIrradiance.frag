#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform samplerCube hdrCubeMap;

layout(location = 0) in vec4 localPos;
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

void main()
{
    vec3 N = normalize(localPos.xyz);

    vec3 irradiance = vec3(0);

    vec3 up = vec3(0.0f, 1.0f, 0.0f);
    vec3 right = cross(up, N);
    up = cross(N, right);

    float sampleDelta = 0.025f;
    float nrSamples = 0.0f;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(hdrCubeMap, localPos.xyz).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    outColor = vec4(irradiance, 1.0f);
}