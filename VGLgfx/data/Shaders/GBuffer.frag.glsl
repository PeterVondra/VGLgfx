#define PI 3.1415926535897932384626433832795
#define BLOCKER_SEARCH 100
#define PENUMBRA_ESTIMATE 3
#define NEAR 1

layout(location = 0) in float in_Depth;
layout(location = 1) in vec2 in_UV;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec3 in_FragPosition;
layout(location = 4) in vec3 in_ViewPosition;

#if defined NORMAL_MAPPING || defined PARALLAX_OCCLUSION_MAPPING
layout(location = 5) in mat3 in_TBN;
#endif

layout(location = 0) out vec4 out_Position;
layout(location = 1) out vec4 out_Normal;
layout(location = 2) out vec4 out_Albedo;
// R = Metallic, G = Roughness, B = Ambient Occlusion
layout(location = 3) out vec4 out_MRAA; // A = Depth


layout(binding = 1) uniform UniformBufferObject
{
	vec4 albedo;
	vec4 mrao;
}ubo;

// Parallax Mapping
#ifdef PARALLAX_OCCLUSION_MAPPING
vec2 ParallaxMapping(vec2 texCoords);
#endif

float checker(vec2 uv, float repeats) 
{
  float cx = floor(repeats * uv.x);
  float cy = floor(repeats * uv.y); 
  float result = mod(cx + cy, 2.0);
  return sign(result);
}

float linearDepth(float depth)
{
	//float z = depth * 2.0f - 1.0f; 
	return (1.0f * 1000.0f) / (1000.0f + 1.0f - depth * (1000.0f - 1.0f));	
}

void main()
{
	#ifdef PARALLAX_OCCLUSION_MAPPING
	vec2 uv = ParallaxMapping(in_UV);
	#else
	vec2 uv = in_UV;
	#endif

	#ifdef ALBEDO_MAP
	out_Albedo = texture(albedoMap, uv);
	if(out_Albedo.a < 0.5)
		discard;
	#else
	out_Albedo = vec4(ubo.albedo.rgb, 1.0f);
	#endif

	if(isinf(out_Albedo.r) || isinf(out_Albedo.g) || isinf(out_Albedo.b) || isnan(out_Albedo.r) || isnan(out_Albedo.g) || isnan(out_Albedo.b)){ out_Albedo = vec4(0.0f, 0.0f, 0.0f, 1.0f); }

	out_Position = vec4(in_FragPosition, 1.0f);

	#ifdef NORMAL_MAPPING
	out_Normal = vec4(normalize(in_TBN*(normalize(texture(normalMap, uv).rgb * 2.0f - 1.0f))), 1.0f);
	#else
	out_Normal = vec4(normalize(in_Normal), 1.0f);
	#endif

	#ifdef METALLIC_MAP
	out_MRAA.r = min(texture(metallicMap, uv).r, 1.0f);
	#else
	out_MRAA.r = ubo.mrao.r;
	#endif

	#ifdef ROUGHNESS_MAP
	out_MRAA.g = min(texture(roughnessMap, uv).r * ubo.mrao.g, 1.0f);
	#else
	out_MRAA.g = ubo.mrao.g;
	#endif

	#ifdef AO_MAP
	out_MRAA.b = min(texture(aoMap, uv).r * ubo.mrao.b, 1.0f);
	#else
	out_MRAA.b = ubo.mrao.b;
	#endif

	out_MRAA.a = in_Depth;
}

// Parallax Mapping
#ifdef PARALLAX_OCCLUSION_MAPPING
vec2 ParallaxMapping(vec2 texCoords)
{ 
	vec3 viewDir = normalize(transpose(in_TBN) * normalize(in_ViewPosition - in_FragPosition));

	const float minLayers = 32;
	const float maxLayers = 128;

	const float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0f), viewDir)));
	float layerDepth = ubo.albedo.a / numLayers;
	float currentLayerDepth = 0.0f;

	vec2 P = viewDir.xy * 0.02;
	vec2 deltaUV = P / numLayers;

	vec2 currentUV = in_UV;
	float currentDepth = 1.0f - texture(imageDisp, in_UV).r;

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

	#ifdef POM_EDGE_FIX
	if(finalUV.x > 1.0 || finalUV.y > 1.0 || finalUV.x < 0.0 || finalUV.y < 0.0)
    	discard;
	#endif

	return finalUV;
} 
#endif