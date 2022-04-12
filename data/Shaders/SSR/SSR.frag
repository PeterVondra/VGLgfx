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

	vec4 view_direction;

	mat4 projection;
	mat4 view;
}ubo;

mat4 view;
float metallic;
float roughness;

float FetchLinearDepth(vec2 depthUV);
bool TraceScreenSpaceRay(vec3 pRayOrigin, vec3 pRayDirection, float pJitter, out vec2 pHitPixel, out vec3 pHitPoint, out float pItCount);

// Customize
float _Iterations = 100;							// maximum ray iterations
float _BinarySearchIterations = 100;				// maximum binary search refinement iterations
float _PixelZSize = 0.01;							// Z size in camera space of a pixel in the depth buffer
float _PixelStride = 1;							// number of pixels per ray step close to camera
float _PixelStrideZCuttoff = 1;					// ray origin Z at this distance will have a pixel stride of 1.0
float _MaxRayDistance = 1000;						// maximum distance of a ray
float _ScreenEdgeFadeStart = 0.1f;					// distance to screen edge that ray hits will start to fade (0.0 -> 1.0)
float _EyeFadeStart = 0.9f;						// ray direction's Z that ray hits will start to fade (0.0 -> 1.0)
float _EyeFadeEnd = 1.0f;

vec2 image_size;

float linearize_depth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

float fmod(float x, float y)
{
	return x - y * trunc(x/y);
}

float calculateAlphaForIntersection( bool intersect, 
	float iterationCount, 
	float specularStrength,
	vec2 hitPixel,
	vec3 hitPoint,
	vec3 vsRayOrigin,
	vec3 vsRayDirection);

void main()
{
	view = inverse(ubo.projection)* ubo.view;
	
	image_size = textureSize(in_Depth, 0);
	
	vec2 mra = texture(in_MRAA, UV).rg;
	
	metallic = mra.r;
	roughness = 1.0f - mra.g;
	
	if(metallic <= 0.03)
		if(roughness < 0.03)
			discard;
		else metallic = 1.0f - roughness;

	float specularStrength = metallic;
	
	vec3 normal = texture(in_Normal, UV).xyz;
	vec3 albedo = texture(in_SceneImage, UV).rgb;
	
	float d_depth = texture(in_Depth, UV).r;

	vec4 cameraRay = vec4(UV * 2.0 - 1.0, 1.0, 1.0);
	cameraRay = inverse(ubo.projection) * cameraRay;
	cameraRay = cameraRay / cameraRay.w;
	cameraRay = ubo.view_direction;

	vec3 ray_origin = cameraRay.xyz*d_depth;
	
	vec3 ray_direction = normalize(reflect(normalize(ray_origin), normalize(normal)));

	vec2 hit_pixel;
	vec3 hit_point;
	float iteration_count;

	vec2 uv2 = UV * image_size;
	float c = (uv2.x + uv2.y) * 0.25f;
	float jitter = fmod(c, 1.0f);

	bool intersect = TraceScreenSpaceRay(ray_origin, ray_direction, jitter, hit_pixel, hit_point, iteration_count);
	float alpha = calculateAlphaForIntersection(intersect, iteration_count, specularStrength, hit_pixel, hit_point, ray_origin, ray_direction);

	hit_pixel = mix(UV, hit_pixel, float(intersect));

	outColor.rgb = texture(in_SceneImage, vec2(hit_pixel.x, 1.0f - hit_pixel.y)).rgb;
	//outColor.rgb = vec3(d_depth);
	outColor.a = 1.0f;
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

float FetchLinearDepth(vec2 depthUV) {
    float cameraFarPlusNear = 10.0f + 1.0f;
    float cameraFarMinusNear = 10.0f - 1.0f;
    float cameraCoef = 1.0f * 10.0f;
    return cameraCoef / (10.0f - texture(in_Depth, vec2(depthUV.x, 1.0f - depthUV.y)).x * (1.0f - 10.0f));
}

bool rayIntersectsDepthBuffer (float minZ, float maxZ, vec2 depthUV) {
    float z = FetchLinearDepth(depthUV);
    
    /*
    * Based on how far away from the camera the depth is,
    * adding a bit of extra thickness can help improve some
    * artifacts. Driving this value up too high can cause
    * artifacts of its own.
    */
    float depthScale = min(1.0f, z * _PixelStrideZCuttoff);
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

	float strie_scale = 1.0f - min(1.0f, -pRayOrigin.z / _PixelStrideZCuttoff);
	float pixel_stride = 1.0f + strie_scale * _PixelStride;

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

	for(i = 0; i < _Iterations && intersect == false; i++){
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

		for(float j = 0; j < _BinarySearchIterations; j++){
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
}

float calculateAlphaForIntersection( bool intersect, 
	float iterationCount, 
	float specularStrength,
	vec2 hitPixel,
	vec3 hitPoint,
	vec3 vsRayOrigin,
	vec3 vsRayDirection)
{
	float alpha = min( 1.0, specularStrength * 1.0);
	
	// Fade ray hits that approach the maximum iterations
	alpha *= 1.0 - (iterationCount / _Iterations);
	
	// Fade ray hits that approach the screen edge
	float screenFade = _ScreenEdgeFadeStart;
	vec2 hitPixelNDC = (hitPixel * 2.0 - 1.0);
	float maxDimension = min( 1.0, max( abs( hitPixelNDC.x), abs( hitPixelNDC.y)));
	alpha *= 1.0 - (max( 0.0, maxDimension - screenFade) / (1.0 - screenFade));
	
	// Fade ray hits base on how much they face the camera
	float eyeFadeStart = _EyeFadeStart;
	float eyeFadeEnd = _EyeFadeEnd;
	SwapIfBigger( eyeFadeStart, eyeFadeEnd);
	
	float eyeDirection = clamp( vsRayDirection.z, eyeFadeStart, eyeFadeEnd);
	alpha *= 1.0 - ((eyeDirection - eyeFadeStart) / (eyeFadeEnd - eyeFadeStart));
	
	// Fade ray hits based on distance from ray origin
	alpha *= 1.0 - clamp( distance( vsRayOrigin, hitPoint) / _MaxRayDistance, 0.0, 1.0);
	
	alpha *= float(intersect);
	
	return alpha;
}