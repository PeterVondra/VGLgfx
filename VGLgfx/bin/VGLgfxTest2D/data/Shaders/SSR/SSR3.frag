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