#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_multiview : enable

layout(set = 1, binding = 0) uniform sampler2D in_Albedo;

layout(location = 0) in vec2 inUv;
layout(location = 1) in vec3 in_Position;
layout(location = 2) in vec3 in_Light_Position;

layout (location = 0) out float out_FragmentColor;

void main() 
{	
	if(texture(in_Albedo, inUv).a < 0.5)
		discard;

	out_FragmentColor = length(in_Position - in_Light_Position);
}