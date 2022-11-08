#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout (push_constant) uniform PushConstants {
	mat4 mvp;
} pushConstants;


out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main()
{
	gl_Position = pushConstants.mvp * vec4(inPosition, 1.0f);
}