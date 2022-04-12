#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define HORIZONTAL 1
#define VERTICAL 2
#define PI 3.141592

layout(binding = 0) uniform sampler2D in_SSAO;

layout(location = 0) in vec2 UV;
layout(location = 0) out float outColor;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(in_SSAO, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(in_SSAO, UV + offset).r;
        }
    }
    outColor = result / (4.0 * 4.0);
}