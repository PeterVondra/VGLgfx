#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define HORIZONTAL 1
#define VERTICAL 2
#define PI 3.141592

layout(binding = 0) uniform sampler2D imageHDR;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstants {
    float FXAA_Span_Max;// = 16.0f;
    float FXAA_REDUCTION_MIN;// = 1.0f/128.0f;
    float FXAA_REDUCTION_BIAS;// = 1.0f/8.0f;
} pushConstants;

void main()
{   
	vec3 color = vec3(0.0f);
	float depth = texture(imageHDR,UV.xy).w;

	// FXAA
	vec2 texSize = 1.0f/textureSize(imageHDR, 0);
	
	float FXAA_Span_Max = pushConstants.FXAA_Span_Max;
	float FXAA_REDUCTION_MIN = pushConstants.FXAA_REDUCTION_MIN;
	float FXAA_REDUCTION_BIAS = pushConstants.FXAA_REDUCTION_BIAS;
	
	vec3 luma = vec3(0.299, 0.587, 0.114);
	float lumaTL = dot(texture(imageHDR, UV + vec2(-1.0f, -1.0f)*texSize).rgb, luma);
	float lumaTR = dot(texture(imageHDR, UV + vec2(1.0f, -1.0f)*texSize).rgb, luma);
	float lumaBL = dot(texture(imageHDR, UV + vec2(-1.0f, 1.0f)*texSize).rgb, luma);
	float lumaBR = dot(texture(imageHDR, UV + vec2(1.0f, 1.0f)*texSize).rgb, luma);
	float lumaM  = dot(texture(imageHDR, UV).rgb, luma);
	
	vec2 blurDir;
	blurDir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
	blurDir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));
	
	float reduction = max((lumaTL + lumaTR + lumaBL + lumaBR) * 0.25 * FXAA_REDUCTION_BIAS, FXAA_REDUCTION_MIN);
	float scale = 1.0f/(min(abs(blurDir.x), abs(blurDir.y)) + reduction);
	
	blurDir = min(vec2(FXAA_Span_Max), max(vec2(-FXAA_Span_Max), blurDir * scale)) * texSize;
	
	vec3 result1 = 0.5 * (texture(imageHDR, UV + blurDir * (1.0f/3.0f - 0.5)).rgb + texture(imageHDR, UV + blurDir * (2.0f/3.0f - 0.5)).rgb);
	vec3 result2 = result1 * 0.5 + 0.25 * (texture(imageHDR, UV + blurDir * -0.5).rgb + texture(imageHDR, UV + blurDir * 0.5).rgb);
	
	float lumaMin = min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
	
	float lumaMax = max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));
	
	float lumaResult2 = dot(luma, result2);
    
	color += result2;;
	if(lumaResult2 < lumaMin || lumaResult2 > lumaMax)
	    color += result1;

	outColor = vec4(color, depth);
}