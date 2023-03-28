#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 position;
layout(location = 2) in vec3 outlineColor;
layout(location = 3) in vec2 outlineThickness;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(outlineColor.x, outlineColor.y, outlineColor.z, 1.0f);
	if
	(
		position.x > -(1.0f - outlineThickness.x * 0.5) &&
		position.x <  (1.0f - outlineThickness.x * 0.5) &&
		position.y > -(1.0f - outlineThickness.y * 0.5) &&
		position.y <  (1.0f - outlineThickness.y * 0.5)
	)
		outColor = fragColor;
}