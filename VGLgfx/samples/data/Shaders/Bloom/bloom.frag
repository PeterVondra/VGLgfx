#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define HORIZONTAL 1
#define VERTICAL 2
#define PI 3.141592
#define EPSILON 0.0000001

layout(binding = 0) uniform sampler2D imageDOF;
layout(binding = 1) uniform sampler2D imageBloom;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;

void main()
{
   outColor.rgb = texture(imageDOF, UV).rgb + texture(imageBloom, UV).rgb;
   outColor.a = 1.0f;
}