#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define HORIZONTAL 1
#define VERTICAL 2
#define PI 3.141592

layout(binding = 1) uniform sampler2D in_Position;
layout(binding = 2) uniform sampler2D in_Normal;
layout(binding = 3) uniform sampler2D in_Noise;

layout(location = 0) in vec2 UV;
layout(location = 0) out float outColor;

layout(binding = 0) uniform UniformBufferObject
{
	float intensity;
	float radius;
	float bias;
	float kernel_size;
	mat4 projection;
	mat4 view;
	vec4 samples[64];
}ubo;

vec2 noiseScale;

vec3 reconstructVSPosFromDepth(vec2 uv)
{
  float depth = texture(in_Normal, uv).w;
  float x = uv.x * 2.0 - 1.0;
  float y = (1.0 - uv.y) * 2.0 - 1.0;
  vec4 pos = vec4(x, y, depth, 1.0);
  vec4 posVS = inverse(ubo.projection) * pos;
  return posVS.xyz / posVS.w;
}

void main()
{
	mat4 view = inverse(ubo.projection) * ubo.view;
	noiseScale = textureSize(in_Position, 0);
	vec2 noiseSize = textureSize(in_Noise, 0);
	noiseScale /= noiseSize;
	
	vec3 fragPos = (view * vec4(texture(in_Position, UV).xyz, 1.0f)).xyz;
	
	vec4 normal_t = texture(in_Normal, UV);
	if(normal_t == vec4(0.0f)) discard;
	vec3 normal = normalize(mat3(view) * normal_t.xyz);
	
	vec3 randomVec = texture(in_Noise, UV * noiseScale).rgb;
	
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);
	
	float occlusion = 0.0f;
	for(int i = 0; i < ubo.kernel_size; i++){
		vec3 samplePos = (TBN * ubo.samples[i].xyz);
		samplePos = fragPos + samplePos * ubo.radius;
		
		vec4 offset = ubo.projection * vec4(samplePos, 1.0f);
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5f + 0.5f;
		offset.y = 1.0f - offset.y;
		
		float sampleDepth = (view*texture(in_Position, offset.xy)).z;
	
		float range = smoothstep(0.0f, 1.0f, ubo.radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + ubo.bias ? 1.0f : 0.0f) * range;
	}

	//noiseScale = textureSize(in_Position, 0);
	//vec2 noiseSize = textureSize(in_Noise, 0);
	//noiseScale /= noiseSize;
	//
	//vec3 fragPos = (vec4(texture(in_Position, UV).xyz, 1.0f)).xyz;
	//
	//vec4 normal_t = texture(in_Normal, UV);
	//if(normal_t == vec4(0.0f)) discard;
	//vec3 normal = normalize(normal_t.xyz);
	//
	//vec3 randomVec = texture(in_Noise, UV * noiseScale).rgb;
	//
	//vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	//vec3 bitangent = cross(normal, tangent);
	//mat3 TBN = mat3(tangent, bitangent, normal);
	//
	//float radius = 100.f;
	//
	//float occlusion = 0.0f;
	//for(int i = 0; i < 64; i++){
	//	vec3 samplePos = (TBN * ubo.samples[i].xyz);
	//	samplePos = fragPos + samplePos * radius;
	//	
	//	float sampleZ = dot(samplePos - ubo.view_pos.xyz, normalize(-ubo.view_dir.xyz));
	//
	//	vec4 offset = ubo.projection * vec4(samplePos, 1.0f);
	//	offset.xyz /= offset.w;
	//	offset.xyz = offset.xyz * 0.5f + 0.5f;
	//	offset.y = 1.0f - offset.y;
	//	
	//	float sampleDepth = dot(texture(in_Position, offset.st).xyz - ubo.view_pos.xyz, normalize(-ubo.view_dir.xyz));
	//
	//	float range = smoothstep(0.0f, 1.0f, radius / abs(fragPos.z - sampleZ));
	//	occlusion += (sampleDepth >= sampleZ + 0.025 ? 1.0f : 0.0f) * range;
	//}

	occlusion = 1.0f - (occlusion / ubo.kernel_size);
	outColor = pow(occlusion, ubo.intensity);
}