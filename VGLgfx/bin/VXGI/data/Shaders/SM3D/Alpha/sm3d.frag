#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D in_Albedo;

layout(location = 0) in vec2 inUv;

void main() 
{	
	if(texture(in_Albedo, inUv).a < 0.5)
		discard;
}