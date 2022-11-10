layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

#ifdef MODEL_ARRAY
layout(location = 5) in mat4 inInstanceTransform;
#endif

layout(binding = 0) uniform UniformBufferObject
{
	mat4 mvp;
	mat4 model;

	#ifdef SHADOW_MAPPING
	mat4 lightSpaceMat;
	#endif

	vec3 cameraPos;
}ubo;

layout(location = 0) out vec3 depth;
layout(location = 1) out vec2 UV;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out vec3 viewPos;
layout(location = 9) out mat4 lightProjection;

#if defined NORMAL_MAPPING || defined PARALLAX_OCCLUSION_MAPPING
layout(location = 5) out mat3 toModelSpace;
#endif

#ifdef SHADOW_MAPPING_PCF
layout(location = 8) out vec4 lightPosFrag;
#endif

void main()
{
	mat4 model = ubo.model;

	#ifdef MODEL_ARRAY
	model = inInstanceTransform;
	#endif

	UV = vec2(inUv.x, 1.0f - inUv.y);

	mat3 nmat = mat3(transpose(inverse(model)));

	gl_Position =  ubo.mvp * vec4(inPosition, 1.0f);
	
	depth = vec3((gl_Position.z - 1.0f) / 10.0f);

	Normal = normalize(nmat * inNormal); 

	#if defined NORMAL_MAPPING || defined PARALLAX_OCCLUSION_MAPPING
	vec3 T = normalize(nmat * inTangent);
	vec3 B = normalize(nmat * inBitangent);//cross(Normal, T));

	mat3 TBN = mat3(T, B, Normal);

	toModelSpace = TBN;
	#endif

	fragPos = vec3(model * vec4(inPosition, 1.0));
	viewPos = ubo.cameraPos;

	#ifdef SHADOW_MAPPING_PCF
	const mat4 bias = mat4( 
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.5, 0.5, 0.0, 1.0 
	);

	lightProjection = ubo.lightSpaceMat;
	lightPosFrag = bias * ubo.lightSpaceMat * vec4(inPosition, 1.0f);
	#endif
}