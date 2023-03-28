#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 view_projection[6];
}ubo;

layout (push_constant) uniform PushConstants {
	mat4 model_transform;
	vec3 light_position;
} pushConstants;

layout(location = 0) out vec3 out_Position;
layout(location = 1) out vec3 out_Light_Position;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main()
{
	gl_Position = ubo.view_projection[gl_ViewIndex] * pushConstants.model_transform * vec4(inPosition, 1.0f);
	out_Position = (pushConstants.model_transform * vec4(inPosition, 1.0f)).xyz;
	out_Light_Position = pushConstants.light_position;
}