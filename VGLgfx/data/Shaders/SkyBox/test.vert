#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 vp;
	mat4 p;
}ubo;

layout(location = 0) out vec3 UV;

void main()
{
	UV = inPosition;
	gl_Position =  ubo.vp * vec4(inPosition, 1.0f);
}