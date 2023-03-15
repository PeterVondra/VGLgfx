layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_Uv;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec3 in_Tangent;
layout(location = 4) in vec3 in_Bitangent;

#ifdef MODEL_ARRAY
layout(location = 5) in mat4 inInstanceTransform;
#endif

layout(binding = 0) uniform UniformBufferObject
{
	mat4 mvp;
	mat4 model;
	mat4 view;

	vec3 cameraPos;
}ubo;

layout(location = 0) out float out_Depth;
layout(location = 1) out vec2 out_UV;
layout(location = 2) out vec3 out_Normal;
layout(location = 3) out vec3 out_FragPosition;
layout(location = 4) out vec3 out_ViewPosition;

#if defined NORMAL_MAPPING || defined PARALLAX_OCCLUSION_MAPPING
layout(location = 5) out mat3 out_TBN;
#endif

void main()
{
	mat4 model = ubo.model;

	#ifdef MODEL_ARRAY
	model = inInstanceTransform;
	#endif

	out_UV = vec2(in_Uv.x, 1.0f - in_Uv.y);
	out_ViewPosition = ubo.cameraPos;

	mat3 nmat = mat3(transpose(inverse(model)));

	gl_Position =  ubo.mvp * vec4(in_Position, 1.0f);
	
	out_Depth = (gl_Position.z - 1.0f) / 10.0f;

	out_Normal = normalize(nmat * in_Normal); 
	#if defined NORMAL_MAPPING || defined PARALLAX_OCCLUSION_MAPPING

	vec3 T = normalize(nmat * in_Tangent);
	vec3 B = normalize(nmat * in_Bitangent);//cross(Normal, T));

	out_TBN = mat3(T, B, in_Normal);
	#endif

	out_FragPosition = vec3(model * vec4(in_Position, 1.0));
}