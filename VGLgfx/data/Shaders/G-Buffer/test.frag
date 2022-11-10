#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in vec3 viewPos;

layout(location = 0) out vec3 out_Position;
layout(location = 1) out vec4 out_Albedo;
layout(location = 2) out vec4 out_MRAO; // Metallic (R) Roughness (G) AO (B)
layout(location = 3) out vec3 out_Normals;

#ifdef PARALLAX_OCCLUSION_MAPPING
vec2 ParallaxMapping(vec2 texCoords);
#endif

layout(binding = 1) uniform UniformBufferObject
{
	vec4 albedo;
	vec4 mrao;
}ubo;

void main() 
{	
	vec2 uv = UV;
	#ifdef PARALLAX_OCCLUSION_MAPPING
	uv = ParallaxMapping(UV);
	#endif

	// Albedo
	out_Albedo = ubo.albedo;
	#ifdef ALBEDO_MAP
	out_Albedo = pow(texture(albedoMap, uv).rgb, vec3(gamma));
	#endif

	// Normal
	out_Normals = Normal;
	#ifdef NORMAL_MAPPING
	out_Normals = normalize(texture(normalMap, uv).rgb * 2.0f - 1.0f);
	out_Normals = normalize(toModelSpace * normal);
	#endif

	// Metallic
	out_MRAO.x = ubo.mrao.x;
	#ifdef METALLIC_MAP
	out_MRAO.x = texture(metallicMap, uv).r;
	#endif

	// Roughness
	out_MRAO.y = ubo.mrao.y;
	#ifdef ROUGHNESS_MAP
	out_MRAO.y = texture(roughnessMap, uv).r;
	#endif

	// Ambient Occlusion
	out_MRAO.z = ubo.mrao.z;
	#ifdef AO_MAP
	out_MRAO.z = texture(aoMap, uv).r;
	#endif
}

// Parallax Mapping
#ifdef PARALLAX_OCCLUSION_MAPPING
vec2 ParallaxMapping(vec2 texCoords)
{ 
	vec3 viewDir = normalize(transpose(toModelSpace) * normalize(viewPos - fragPos));

	const float minLayers = 32;
	const float maxLayers = 128;

	const float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0f), viewDir)));
	float layerDepth = 1.0f / numLayers;
	float currentLayerDepth = 0.0f;

	vec2 P = viewDir.xy * 0.02;
	vec2 deltaUV = P / numLayers;

	vec2 currentUV = UV;
	float currentDepth = 1.0f - texture(imageDisp, UV).r;

	while(currentLayerDepth < currentDepth)
	{
		currentUV -= deltaUV;
		currentDepth = 1.0f - texture(imageDisp, currentUV).r;

		currentLayerDepth += layerDepth;
	}

	vec2 prevUV = currentUV + deltaUV;
	float afterDepth = currentDepth - currentLayerDepth;
	float beforeDepth = texture(imageDisp, prevUV).r - currentLayerDepth + layerDepth;

	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalUV = prevUV * weight + currentUV * (1.0f - weight);

	//if(finalUV.x > 1.0 || finalUV.y > 1.0 || finalUV.x < 0.0 || finalUV.y < 0.0)
    	//	discard;

	return finalUV;
} 
#endif