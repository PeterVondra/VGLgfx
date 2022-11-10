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
	mat4 mvp;
	mat4 model;
	vec3 cameraPos;
}ubo;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 Normal;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec3 viewPos;

void main()
{
	viewPos = ubo.cameraPos;
	gl_Position = ubo.mvp * vec4(inPosition, 1.0f);
	fragColor = inPosition;
	fragPos = vec3(ubo.model * vec4(inPosition, 1.0f));
	Normal = mat3(transpose(inverse(ubo.model))) * inNormal;
}