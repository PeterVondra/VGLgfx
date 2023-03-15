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
	float SSR_MaxDistance;
	float SSR_Resolution;
	float SSR_Steps;
	float SSR_Thickness;
	mat4 projection;
	mat4 view;
}ubo;

const float rayStep = 0.01;
const float minRayStep = 0.01;
const float maxSteps = 50;
const float searchDist = 10;
const float searchDistInv = 0.1;
const int numBinarySearchSteps = 5;
const float maxDDepth = 1.0;
const float maxDDepthInv = 1.0;
const float reflectionSpecularFalloffExponent = 3.0;

#define Scale vec3(.8, .8, .8)
#define K 19.19

vec3 BinarySearch(inout vec3 p_Direction, inout vec3 hit_st, inout float dDepth);
 
vec4 rayCast(vec3 p_Direction, inout vec3 hit_st, out float dDepth);

vec3 fresnelSchlick(float cosTheta, vec3 F0);

vec3 hash(vec3 a);

float metallic;
float roughness;

mat4 view;

void main()
{
	view = inverse(ubo.projection)* ubo.view;
	
	vec2 image_size = textureSize(in_Position, 0);

	vec2 mra = texture(in_MRAA, UV).rg;
	
	vec3 normal = mat3(view)*texture(in_Normal, UV).xyz;
	vec3 position = (view*texture(in_Position, UV)).xyz;
	vec3 albedo = texture(in_SceneImage, UV).rgb;
	metallic = mra.r;
	roughness = 1.0f - mra.g;

	if(metallic < 0.01)
		if(roughness < 0.01)
			discard;
		else metallic = roughness;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	vec3 fresnel = fresnelSchlick(max(dot(normalize(normal), normalize(position)), 0.0f), F0);

	vec3 reflected = normalize(reflect(normalize(position), normalize(normal)));

	vec3 hit_position = position;
	float dDepth;

	vec3 wp = vec3(vec4(position, 1.0f));
	vec3 jitter = mix(vec3(0.0f), normalize(vec3(hash(wp))), roughness);
	vec4 st = rayCast(reflected * max(minRayStep, -position.z), hit_position, dDepth);
	vec2 d_st = smoothstep(0.2f, 0.6f, abs(vec2(0.5f, 0.5f) - st.xy));

	float sef = clamp(1.0f - (d_st.x + d_st.y), 0.0f, 1.0f);

	float rm = pow(metallic, reflectionSpecularFalloffExponent) * sef * -reflected.z;

	vec3 ssr = texture(in_SceneImage, vec2(st.x, 1.0f - st.y)).rgb * clamp(rm, 0.0, 0.99) * fresnel; 

	outColor = vec4(ssr, metallic);
}

vec3 BinarySearch(inout vec3 p_Direction, inout vec3 hit_st, inout float dDepth)
{
	float depth;

	vec4 projected_st;

	for(int i = 0; i < numBinarySearchSteps; i++){
		projected_st = ubo.projection * vec4(hit_st, 1.0f);
		projected_st.xy /= projected_st.w;
		projected_st.xy = projected_st.xy * 0.5f + 0.5f;

		depth = (view*texture(in_Position, vec2(projected_st.s, 1.0f - projected_st.t))).z;
		dDepth = hit_st.z - depth;

		p_Direction *= 0.5f;

		hit_st += (dDepth > 0.0f) ? p_Direction : -p_Direction;
	}

	projected_st = ubo.projection * vec4(hit_st, 1.0f);
	projected_st.xy /= projected_st.w;
	projected_st.xy = projected_st.xy * 0.5f + 0.5f;

	return vec3(projected_st.xy, depth);
}

vec4 rayCast(in vec3 p_Direction, inout vec3 hit_st, out float dDepth)
{
	p_Direction *= rayStep;

	float depth;
	int steps;
	vec4 projected_st;

	for(int i = 0; i < maxSteps; i++){
		hit_st += p_Direction;

		projected_st = ubo.projection * vec4(hit_st, 1.0f);
		projected_st.xy /= projected_st.w;
		projected_st.xy = projected_st.xy * 0.5f + 0.5f;

		depth = (view*texture(in_Position, vec2(projected_st.s, 1.0f - projected_st.t))).z;

		if(depth > 1000.0f)
			continue;

		dDepth = hit_st.z - depth;

		if((p_Direction.z - dDepth)  < 1.2f){
			if(dDepth <= 0.0f){
				vec4 result = vec4(BinarySearch(p_Direction, hit_st, dDepth), 1.0f);
				return result;
			}
		}

		steps++;
	}

	return vec4(projected_st.xy, depth, 0.0f);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 hash(vec3 a)
{
    a = fract(a * Scale);
    a += dot(a, a.yxz + K);
    return fract((a.xxy + a.yxx)*a.zyx);
}

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
	float SSR_MaxDistance;
	float SSR_Resolution;
	float SSR_Steps;
	float SSR_Thickness;
	mat4 projection;
	mat4 view;
}ubo;

mat4 view;
float metallic;
float roughness;

void main()
{
	view = inverse(ubo.projection)* ubo.view;
	
	vec2 image_size = textureSize(in_Position, 0);
	
	vec2 mra = texture(in_MRAA, UV).rg;
	
	metallic = mra.r;
	roughness = 1.0f - mra.g;
	
	if(metallic < 0.03)
		if(roughness < 0.03)
			discard;
		else metallic = roughness;
	
	vec3 normal = mat3(view)*texture(in_Normal, UV).xyz;
	vec4 position = (view*texture(in_Position, UV));
	vec3 albedo = texture(in_SceneImage, UV).rgb;
	
	vec3 unit_position = normalize(position.xyz);
	vec3 pivot = normalize(reflect(unit_position, normal));
	
	vec4 start_view = vec4(position.xyz, 1.0f);
	vec4 end_view = vec4(position.xyz + pivot * ubo.SSR_MaxDistance, 1.0f);
	
	vec4 start_f = start_view;
	start_f = ubo.projection * start_f;
	start_f.xyz /= start_f.w;
	start_f.xy = start_f.xy * 0.5f + 0.5f;
	//start_f.xy *= image_size;
	
	vec4 end_f = end_view;
	end_f = ubo.projection * end_f;
	end_f.xyz /= end_f.w;
	end_f.xy = end_f.xy * 0.5f + 0.5f;
	//end_f.xy *= image_size;
	
	vec4 uv;
	vec2 fragment = start_f.xy;
	uv.xy = fragment / image_size;
	
	float delta_x = end_f.x - start_f.x;
	float delta_y = end_f.y - start_f.y;
	
	float use_x = abs(delta_x) >= abs(delta_y) ? 1.0f : 0.0f;
	float delta = mix(abs(delta_y), abs(delta_x), use_x) * clamp(ubo.SSR_Resolution, 0.0f, 1.0f);
	
	vec2 increment = vec2(delta_x, delta_y) / max(delta, 0.001f);
	
	float search0 = 0.0f;
	float search1 = 0.0f;
	
	float hit0 = 0.0f;
	float hit1 = 0.0f;
	
	float view_dst = start_view.z;
	float depth = ubo.SSR_Thickness;
	
	vec4 position_t = position;
	
	for(int i = 0; i < int(delta); ++i){
		fragment += increment;
		uv.xy = fragment;
		position_t = (view*texture(in_Position, vec2(uv.x, 1.0f - uv.y)));
	
		search1 = mix((fragment.y - start_f.y) / delta_y, (fragment.x - start_f.x) / delta_x, use_x);
		search1 = clamp(search1, 0.0f, 1.0f);
		
		view_dst = (start_view.y * end_view.y) / mix(end_view.y, start_view.y, search1);
		
		depth = view_dst - position_t.z;
		
		if(depth > 0 && depth < ubo.SSR_Thickness){
			hit0 = 1.0f;
			break;
		} else search0 = search1;
	}
	
	search1 = search0 + ((search1 - search0) / 2.0f);
	
	float steps = ubo.SSR_Steps;
	
	steps *= hit0;
	
	for(int i = 0; i < int(steps); ++i){
		fragment = mix(start_f.xy, end_f.xy, search1);
		uv.xy = fragment;
		position_t = (view*texture(in_Position, vec2(uv.x, 1.0f - uv.y)));
	
		view_dst = (start_view.y * end_view.y) / mix(end_view.y, start_view.y, search1);
		depth = view_dst - position_t.z;
	
		if(depth > 0.0f && depth < ubo.SSR_Thickness){
			hit1 = 1.0f;
			search1 = search0 + ((search1 - search0) / 2.0f);
		} else {
			float tmp = search1;
			search1 = search1 + ((search1 - search0) / 2.0f);
			search0 = tmp;
		}
	}
	
	float visibility = hit1 * position_t.w * (1 - max(dot(-unit_position, pivot), 0)) *
	(1 - clamp(depth / ubo.SSR_Thickness, 0, 1)) *
	(1 - clamp(length(position_t - position) / ubo.SSR_MaxDistance, 0.0f, 1.0f)) *
    (uv.x < 0 || uv.x > 1 ? 0 : 1) *
    (uv.y < 0 || uv.y > 1 ? 0 : 1);
	
	visibility = clamp(visibility, 0, 1);
	
	uv.ba = vec2(visibility);
	outColor = uv;
	outColor = texture(in_SceneImage, vec2(uv.x, 1.0f - uv.y));
	outColor.a = 1.0f;
}

#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define HORIZONTAL 1
#define VERTICAL 2
#define PI 3.141592

layout(binding = 1) uniform sampler2D in_SceneImage;
layout(binding = 2) uniform sampler2D in_Depth;
layout(binding = 3) uniform sampler2D in_Normal;
layout(binding = 4) uniform sampler2D in_MRAA;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject
{
	float SSR_MaxDistance;
	float SSR_Resolution;
	float SSR_Steps;
	float SSR_Thickness;
	mat4 projection;
	mat4 view;
}ubo;

mat4 view;
float metallic;
float roughness;

bool TraceScreenSpaceRay(vec3 pRayOrigin, vec3 pRayDirection, float pJitter, out vec2 pHitPixel, out vec3 pHitPoint, out float pItCount);

// Customize
float ray_length;
float pixelStrideZCuttoff;
float stride; // Pixel stride
float iterations;
float binary_search_iterations; 

vec2 image_size;

void main()
{
	view = inverse(ubo.projection)* ubo.view;
	
	image_size = textureSize(in_Depth, 0);
	
	vec2 mra = texture(in_MRAA, UV).rg;
	
	metallic = mra.r;
	roughness = 1.0f - mra.g;
	
	if(metallic < 0.03)
		if(roughness < 0.03)
			discard;
		else metallic = roughness;
	
	vec3 normal = mat3(view)*texture(in_Normal, UV).xyz;
	vec3 albedo = texture(in_SceneImage, UV).rgb;
	
	vec3 start_depth = texture(in_Depth, UV).xyz;


}

float DistanceSquared(vec2 a, vec2 b) {
    a -= b;
    return dot(a, a);
}

void SwapIfBigger (inout float aa, inout float bb) {
    if( aa > bb) {
        float tmp = aa;
        aa = bb;
        bb = tmp;
    }
}

float fetchLinearDepth (vec2 depthUV) {
    float cameraFarPlusNear = 10.0f + 1.0f;
    float cameraFarMinusNear = 10.0f - 1.0f;
    float cameraCoef = 2.0 * 1.0f;
    return cameraCoef / (cameraFarPlusNear - texture2D(in_Depth, depthUV ).x * cameraFarMinusNear);
}

bool rayIntersectsDepthBuffer (float minZ, float maxZ, vec2 depthUV) {
    float z = fetchLinearDepth(depthUV);
    
    /*
    * Based on how far away from the camera the depth is,
    * adding a bit of extra thickness can help improve some
    * artifacts. Driving this value up too high can cause
    * artifacts of its own.
    */
    float depthScale = min(1.0f, z * pixelStrideZCuttoff);
    z += ubo.SSR_Thickness + mix(0.0f, 2.0f, depthScale);
    return (maxZ >= z) && (minZ - ubo.SSR_Thickness <= z);
}

// Return true if ray hits a pixel in depth buffer, output: hit pixel, hit point(View Space), iteration count
// Based on Morgan McGuire & Mike Mara's GLSL implementation:
// http://casual-effects.blogspot.com/2014/08/screen-space-ray-tracing.html
bool TraceScreenSpaceRay(vec3 pRayOrigin, vec3 pRayDirection, float pJitter, out vec2 pHitPixel, out vec3 pHitPoint, out float pItCount)
{
	// 1.0f == Near
	float ray_length = ((pRayOrigin.z + pRayDirection.z * ubo.SSR_MaxDistance) > -1.0f) ? (-1.0f - pRayOrigin.z) / pRayDirection.z : ubo.SSR_MaxDistance;
	vec3 ray_end = pRayOrigin + pRayDirection * ray_length;

	vec4 H0 = ubo.projection * vec4(pRayOrigin, 1.0f);
	vec4 H1 = ubo.projection * vec4(ray_end, 1.0f);

	float k0 = 1.0f/H0.w;
	float k1 = 1.0f/H1.w;

	vec3 Q0 = pRayOrigin * k0;
	vec3 Q1 = ray_end * k1;

	vec2 P0 = H0.xy * k0;
	vec2 P1 = H1.xy * k1;

	P1 += (DistanceSquared(P0, P1) < 0.0001f) ? 0.01f : 0.0f;

	vec2 delta = P1 - P0;

	bool permute = false;
	if(abs(delta.x) < abs(delta.y)){ permute = true; delta = delta.yx; P1 = P1.yx; }

	float step_direction = sign(delta.x);
	float invdx = step_direction / delta.x;

	vec3 dQ = (Q1 - Q0) * invdx;
	float dk = (k1 - k0) * invdx;
	vec2 dP = vec2(step_direction, delta.y * invdx);

	float strie_scale = 1.0f - min(1.0f, -pRayOrigin.z / pixelStrideZCuttoff);
	float pixel_stride = 1.0f + strie_scale * stride;

	dP *= pixel_stride; 
	dQ *= pixel_stride;
	dk *= pixel_stride;

	P0 += dP * pJitter;
	Q0 += dQ * pJitter;
	k0 += dk * pJitter;

	float i = 0.0f;
	float zA = 0.0f;
	float zB = 0.0f;

	vec4 pqk = vec4(P0, Q0.z, k0);
	vec4 dPQK = vec4(dP, dQ.z, dk);

	bool intersect = false;

	vec2 odcis = 1.0f / image_size;

	for(i = 0; i < iterations && intersect == false; i++){
		pqk += dPQK;

		zA = zB;
		zB = (dPQK.z * 0.5f + pqk.z) / (dPQK.w * 0.5f + pqk.w);
		SwapIfBigger(zB, zA);

		pHitPixel = permute ? pqk.yx : pqk.xy;
		pHitPixel *= odcis;

		intersect = rayIntersectsDepthBuffer(zA, zB, pHitPixel);
	}

	if(pixel_stride > 1.0f && intersect){
		pqk -= dPQK;
		dPQK /= pixel_stride;

		float original_stride = pixel_stride * 0.5f;
		float stride_ = original_stride;

		zA = pqk.z / pqk.w;
		zB = zA;

		for(float j = 0; j < binary_search_iterations; j++){
			pqk += dPQK * stride_;

			zA = zB;
			zB = (dPQK.z * -0.5f + pqk.z)/(dPQK.w * -0.5f + pqk.w);
			SwapIfBigger(zA, zB);

			pHitPixel = permute ? pqk.yx : pqk.xy;
			pHitPixel *= odcis;

			original_stride *= 0.5f;
			stride_ = rayIntersectsDepthBuffer(zA, zB, pHitPixel) ? -original_stride : original_stride;
		}
	}

	Q0.xy = dQ.xy * i;
	Q0.z = pqk.z;
	pHitPoint = Q0/pqk.w;
	pItCount = i;
	return intersect;

	return true;
}