#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 position;
layout(location = 3) in vec2 UV;
layout(location = 4) flat in int glyphIndex;

layout(binding = 1) uniform sampler2D glyphs[127];

layout(location = 0) out vec4 outColor;

void main()
{
	if(glyphIndex == 32)
		discard;
	outColor = vec4(fragColor, texture(glyphs[glyphIndex - 1], UV).r);
}