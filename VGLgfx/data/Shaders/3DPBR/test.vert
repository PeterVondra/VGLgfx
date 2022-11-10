#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 mvp;
	mat4 model;
	mat4 lightSpaceMat;
	vec3 cameraPos;
}ubo;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 UV;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out vec3 viewPos;
layout(location = 5) out mat3 toModelSpace;
layout(location = 8) out vec4 lightPosFrag;

void main()
{
	UV = vec2(inUv.x, 1.0f - inUv.y);

	mat3 nmat = mat3(transpose(inverse(ubo.model)));

	gl_Position =  ubo.mvp * vec4(inPosition, 1.0f);
	
	Normal = normalize(nmat * inNormal); 
	vec3 T = normalize(nmat * inTangent);
	vec3 B = normalize(nmat * inBitangent);//cross(Normal, T));

	mat3 TBN = mat3(T, B, Normal);

	toModelSpace = TBN;

	fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
	viewPos = ubo.cameraPos;

	const mat4 bias = mat4( 
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.5, 0.5, 0.0, 1.0 
	);

	lightPosFrag = bias * ubo.lightSpaceMat * vec4(inPosition, 1.0f);
}