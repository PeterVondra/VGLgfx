#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUvs;

layout(location = 2) in float inInstanceLife;
layout(location = 3) in vec4 inInstanceColor;
layout(location = 4) in vec4 inInstanceOutlineColor;
layout(location = 5) in vec2 inInstanceOutlineThickness;
layout(location = 6) in mat4 inInstanceTransform;

layout (push_constant) uniform PushConstants {
	mat4 view_projection;
} pushConstants;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 position;
layout(location = 2) out vec3 outlineColor;
layout(location = 3) out vec2 outlineThickness;
layout(location = 4) out float life;

void main()
{
	gl_Position = pushConstants.view_projection * inInstanceTransform * vec4(inPosition * 0.5, 0.0f, 1.0f);

	life = inInstanceLife;
	position = inPosition;
	fragColor = inInstanceColor;
	outlineColor = inInstanceOutlineColor.xyz;
	outlineThickness = inInstanceOutlineThickness.xy;
}