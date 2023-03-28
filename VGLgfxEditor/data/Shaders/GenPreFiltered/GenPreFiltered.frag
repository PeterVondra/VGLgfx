#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform samplerCube hdrCubeMap;

layout(location = 0) in vec4 localPos;
layout(location = 1) in float roughness;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

float radicalInverse_VdC(uint bits);
vec2 hammersley(uint i, uint N);
vec3 importanceSampleGGX(vec2 Xi, vec3 N, float rgh);

void main()
{
    vec3 N = normalize(localPos.xyz);
    vec3 R = N;
    vec3 V = R;

    const uint sampleCount = 1024u;
    float totalWeight = 0.0f;
    vec3 preFilteredColor = vec3(0.0f);
    for(uint i = 0u; i < sampleCount; ++i){
        vec2 Xi = hammersley(i, sampleCount);
        vec3 H = importanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0f);
        if(NdotL > 0)
        {
            preFilteredColor += texture(hdrCubeMap, localPos.xyz).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    preFilteredColor /= totalWeight;

    outColor = vec4(preFilteredColor, 1.0f);
}

float radicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}  
vec3 importanceSampleGGX(vec2 Xi, vec3 N, float rgh)
{
    float a = rgh * rgh;

    float phi = 2.0f * PI * Xi.x;
    float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a*a - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}