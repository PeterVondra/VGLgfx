#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D image;

layout(location = 0) in vec2 UV;
layout(location = 1) in vec2 BLURSCALE;

layout(location = 0) out vec4 outColor;

void main() 
{	
    float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);


	vec2 tex_offset = BLURSCALE; // gets size of single texel
    vec3 result = texture(image, UV).rgb * weight[0]; // current fragment's contribution
    if(BLURSCALE.x > 0)
    {
        for(int i = 1; i < 16; ++i)
        {
            result += texture(image, UV + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, UV - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 16; ++i)
        {
            result += texture(image, UV + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, UV - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }

	outColor = vec4(result, 1.0f);
}