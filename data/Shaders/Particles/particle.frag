#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 position;
layout(location = 2) in vec3 outlineColor;
layout(location = 3) in vec2 outlineThickness;
layout(location = 4) in float life;

layout(location = 0) out vec4 out_Position;
layout(location = 1) out vec4 out_Normal;
layout(location = 2) out vec4 out_Albedo;
// R = Metallic, G = Roughness, B = Ambient Occlusion
layout(location = 3) out vec4 out_MRAA; // A = Depth

void main()
{
	if(life <= 0.0f)
		discard;

	out_Albedo = vec4(outlineColor.x, outlineColor.y, outlineColor.z, 1.0f);
	if
	(
		position.x > -(1.0f - outlineThickness.x * 0.5) &&
		position.x <  (1.0f - outlineThickness.x * 0.5) &&
		position.y > -(1.0f - outlineThickness.y * 0.5) &&
		position.y <  (1.0f - outlineThickness.y * 0.5)
	)
		out_Albedo = fragColor;

		out_Albedo=vec4(1, 1, 1, 1);
	out_Position.a = -1.0f;
}