#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUvs;
layout(location = 2) in int inInstanceGlyphIndex;
layout(location = 3) in mat4 inInstancePosition;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 proj;
	vec4 color;
}ubo;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 position;
layout(location = 3) out vec2 UV;
layout(location = 4) out int glyphIndex;

void main()
{
	gl_Position = ubo.proj * inInstancePosition * vec4(inPosition * 0.5, 0.0f, 1.0f);
	position = inPosition;
	UV = inUvs;
	fragColor = ubo.color.xyz;

	glyphIndex = inInstanceGlyphIndex;
}