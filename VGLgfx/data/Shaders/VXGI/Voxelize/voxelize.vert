#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout (push_constant) uniform PushConstants {
	mat4 model;
} pushConstants;

layout(location = 0) out vec3 World_Position;
layout(location = 1) out vec3 World_Normal;
layout(location = 2) out vec2 UV;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main()
{
	gl_Position = pushConstants.model * vec4(inPosition, 1.0f);
	World_Position = gl_Position.xyz;
	World_Normal = (mat3(pushConstants.model) * inNormal).xyz;
}