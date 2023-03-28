#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define HORIZONTAL 1
#define VERTICAL 2
#define PI 3.141592

layout(binding = 1) uniform sampler2D in_SceneImage;
layout(binding = 2) uniform sampler2D in_Position;
layout(binding = 3) uniform sampler2D in_Normal;
layout(binding = 4) uniform sampler2D in_MRAA;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject
{
	float SSR_StepLength;
	float SSR_MaxSteps;
	float SSR_BinarySearchSteps;
	float SSR_ReflectionSpecularFalloffExponent;
	mat4 projection;
	mat4 view;
}ubo;

#define Scale vec3(.8, .8, .8)
#define K 19.19

const float minRayStep = 0.01;

vec3 BinarySearch(inout vec3 p_Direction, inout vec3 hit_st, inout float dDepth);
 
vec4 rayCast(vec3 p_Direction, inout vec3 hit_st, out float dDepth);

vec3 fresnelSchlick(float cosTheta, vec3 F0);

vec3 hash(vec3 a);

float roughness;

mat4 view;
mat4 projection;
mat4 invView;
mat4 invProjection;

const mat4 bias = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);

vec3 PositionFromDepth(float depth);

void main()
{
	view = ubo.view;
	invView = inverse(view);
	projection = ubo.projection;
	invProjection = inverse(projection);
	
	vec2 image_size = textureSize(in_Position, 0);

	vec2 mra = texture(in_MRAA, UV).rg;
	
	vec3 vnormal = normalize(mat3(view)*(texture(in_Normal, UV).xyz));
	vec3 vposition = (view*vec4(texture(in_Position, UV).xyz, 1.0f)).xyz;
	vec4 albedo = texture(in_SceneImage, UV);
	roughness = 1.0f - mra.g;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo.rgb, roughness);
	vec3 fresnel = fresnelSchlick(max(dot(normalize(vnormal), normalize(vposition)), 0.0f), F0);

	vec3 reflected = normalize(reflect(normalize(vposition), normalize(vnormal)));

	vec3 hit_position = vposition;
	float dDepth;

	vec3 wp = texture(in_Position, UV).xyz;
	vec3 jitter = mix(vec3(0.0f), normalize(vec3(hash(wp))), roughness);
	vec4 st = rayCast(reflected * max(minRayStep, -vposition.z), hit_position, dDepth);
	vec2 d_st = smoothstep(0.2f, 0.6f, abs(vec2(0.5f, 0.5f) - st.xy));

	float sef = clamp(1.0f - (d_st.x + (d_st.y)), 0.0f, 1.0f);

	float rm = pow(roughness, ubo.SSR_ReflectionSpecularFalloffExponent) * sef * -reflected.z;

	vec3 ssr = texture(in_SceneImage, vec2(st.x, st.y)).rgb * clamp(rm, 0.0, 1.0) * fresnel;

	vec3 scene = albedo.rgb + ssr;

	outColor = vec4(scene, albedo.w);
}

vec3 BinarySearch(inout vec3 p_Direction, inout vec3 hit_st, inout float dDepth)
{
	float depth;

	vec4 projected_st;

	for(int i = 0; i < ubo.SSR_BinarySearchSteps; i++){
		projected_st = bias * ubo.projection * vec4(hit_st, 1.0f);
		projected_st.xy /= projected_st.w;
		//projected_st.xy = projected_st.xy * 0.5f + 0.5f;
		projected_st.t = 1.0f - projected_st.t;

		depth = (view*vec4(texture(in_Position, vec2(projected_st.s, projected_st.t)).xyz, 1.0f)).z;
		dDepth = hit_st.z - depth;

		p_Direction *= 0.5f;

		hit_st += (dDepth > 0.0f) ? p_Direction : -p_Direction;
	}

	projected_st = bias * ubo.projection * vec4(hit_st, 1.0f);
	projected_st.xy /= projected_st.w;
	//projected_st.xy = projected_st.xy * 0.5f + 0.5f;
	projected_st.t = 1.0f - projected_st.t;

	return vec3(projected_st.xy, depth);
}

vec4 rayCast(in vec3 p_Direction, inout vec3 hit_st, out float dDepth)
{
	p_Direction *= ubo.SSR_StepLength;

	float depth;
	int steps;
	vec4 projected_st;

	for(int i = 0; i < ubo.SSR_MaxSteps; i++){
		hit_st += p_Direction;
		//hit_st += (dDepth < 0.0f) ? p_Direction : -p_Direction;

		projected_st = bias * ubo.projection * vec4(hit_st, 1.0f);
		projected_st.xy /= projected_st.w;
		//projected_st.xy = projected_st.xy * 0.5f + 0.5f;
		projected_st.t = 1.0f - projected_st.t;

		depth = (view*vec4(texture(in_Position, vec2(projected_st.s, projected_st.t)).xyz, 1.0f)).z;

		if(depth > 1000.0f)
			continue;

		dDepth = hit_st.z - depth;

		if((p_Direction.z - dDepth)  < 1.2f){
			if(dDepth < 0.0f){
				vec4 result = vec4(BinarySearch(p_Direction, hit_st, dDepth), 1.0f);
				return result;
			}
		}

		steps++;
	}

	return vec4(projected_st.xy, depth, 0.0f);
}


vec3 PositionFromDepth(float depth) {
    float z = depth;

    vec4 clipSpacePosition = vec4(UV, z, 1.0);
    vec4 viewSpacePosition = inverse(bias*projection) * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 hash(vec3 a)
{
	vec3 p3 = fract(vec3(a) * Scale);
	p3 += dot(p3, p3.yzx+K);
	return fract((p3.xxy+p3.yzz)*p3.zyx); 
}