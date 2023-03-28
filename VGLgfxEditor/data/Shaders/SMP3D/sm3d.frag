#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Light_Position;

layout (location = 0) out float out_FragmentColor;

void main() 
{
	out_FragmentColor = length(in_Position - in_Light_Position);
}