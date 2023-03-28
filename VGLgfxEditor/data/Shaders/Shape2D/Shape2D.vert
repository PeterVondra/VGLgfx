#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUvs;
//layout(location = 2) in 

layout(binding = 0) uniform UniformBufferObject
{
	mat4 mvp;
	vec4 color;
	vec4 outlineColor;
	vec2 outlineThickness;
}ubo;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 position;
layout(location = 2) out vec3 outlineColor;
layout(location = 3) out vec2 outlineThickness;

void main()
{
	gl_Position = ubo.mvp * vec4(inPosition * 0.5, 0.0f, 1.0f);
	position = inPosition;
	fragColor = ubo.color;
	outlineColor = ubo.outlineColor.xyz;
	outlineThickness = ubo.outlineThickness.xy;
}