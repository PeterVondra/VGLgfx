#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 UV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColorDiscarded;

layout(binding = 1) uniform samplerCube image;


void main()
{
	outColor = texture(image, UV);
	outColorDiscarded = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}