#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 position;
layout(location = 2) in vec3 outlineColor;
layout(location = 3) in vec2 outlineThickness;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformBufferObject
{
	vec4 color;
}ubo;

void main()
{
	outColor = fragColor;
	if(dot(position, position) > 1.0f - outlineThickness.x)
		outColor = vec4(outlineColor, 1.0f);
	else if(ubo.color.w < 1.0f)
		discard;
	if(dot(position, position) > 1.0f)
		discard;
}