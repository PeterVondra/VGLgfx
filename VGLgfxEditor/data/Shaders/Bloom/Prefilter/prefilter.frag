#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define HORIZONTAL 1
#define VERTICAL 2
#define PI 3.141592
#define EPSILON 0.0000001

layout(binding = 0) uniform sampler2D image;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstants {
	float threshold;
} pushConstants;

void main()
{
	vec3 color = texture(image, UV).rgb;
	float luminosity = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	//if(luminosity < pushConstants.threshold)
	//	outColor.rgb = vec3(0.0f);
	//else
		outColor.rgb = color;

	outColor.a = 1.0f;
}