#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define EPSILON 1.0e-4

// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
layout(binding = 0) uniform sampler2D src_image;

layout (push_constant) uniform PushConstants {
	int mip_level;
} pushConstants;

layout(binding = 1) uniform UniformBufferObject
{
    float threshold;
    vec3 knee;
}ubo;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 downsample;

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 1.0 / 2.2;
vec3 ToSRGB(vec3 v) { return PowVec3(v, invGamma); }

float RGBToLuminance(vec3 col)
{
    return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
}

float KarisAverage(vec3 col)
{
    // Formula is 1 / (1 + luma)
    float luma = RGBToLuminance(ToSRGB(col)) * 0.25f;
    return 1.0f / (1.0f + luma);
}

vec3 QuadraticThreshold(vec3 p_Color, float threshold, vec3 curve)
{
    vec3 color = p_Color;
    // Pixel brightness
    float br = max(max(color.r, color.g), color.b);

    // Under-threshold part: quadratic curve
    float rq = clamp(br - curve.x, 0.0, curve.y);
    rq = curve.z * rq * rq;

    // Combine and apply the brightness response curve.
    color *= max(rq, br - threshold) / max(br, EPSILON);

    return color;
}

void main()
{
    vec2 srcTexelSize = 1.0 / textureSize(src_image, 0);
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(src_image, vec2(UV.x - 2*x, UV.y + 2*y)).rgb;
    vec3 b = texture(src_image, vec2(UV.x,       UV.y + 2*y)).rgb;
    vec3 c = texture(src_image, vec2(UV.x + 2*x, UV.y + 2*y)).rgb;

    vec3 d = texture(src_image, vec2(UV.x - 2*x, UV.y)).rgb;
    vec3 e = texture(src_image, vec2(UV.x,       UV.y)).rgb;
    vec3 f = texture(src_image, vec2(UV.x + 2*x, UV.y)).rgb;

    vec3 g = texture(src_image, vec2(UV.x - 2*x, UV.y - 2*y)).rgb;
    vec3 h = texture(src_image, vec2(UV.x,       UV.y - 2*y)).rgb;
    vec3 i = texture(src_image, vec2(UV.x + 2*x, UV.y - 2*y)).rgb;

    vec3 j = texture(src_image, vec2(UV.x - x, UV.y + y)).rgb;
    vec3 k = texture(src_image, vec2(UV.x + x, UV.y + y)).rgb;
    vec3 l = texture(src_image, vec2(UV.x - x, UV.y - y)).rgb;
    vec3 m = texture(src_image, vec2(UV.x + x, UV.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1

    // Check if we need to perform Karis average on each block of 4 samples
    vec3 groups[5];
    switch (pushConstants.mip_level)
    {
    case 0:
        // We are writing to mip 0, so we need to apply Karis average to each block
        // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
        // artifacts).
        groups[0] = (a+b+d+e) * (0.125f/4.0f);
        groups[1] = (b+c+e+f) * (0.125f/4.0f);
        groups[2] = (d+e+g+h) * (0.125f/4.0f);
        groups[3] = (e+f+h+i) * (0.125f/4.0f);
        groups[4] = (j+k+l+m) * (0.5f/4.0f);
        groups[0] *= KarisAverage(groups[0]);
        groups[1] *= KarisAverage(groups[1]);
        groups[2] *= KarisAverage(groups[2]);
        groups[3] *= KarisAverage(groups[3]);
        groups[4] *= KarisAverage(groups[4]);
        downsample.rgb = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
        break;
    default:
        downsample.rgb = e*0.125;
        downsample.rgb += (a+c+g+i)*0.03125;
        downsample.rgb += (b+d+f+h)*0.0625;
        downsample.rgb += (j+k+l+m)*0.125;
        break;
    }

    //downsample.rgb = e*0.125;
    //downsample.rgb += (a+c+g+i)*0.03125;
    //downsample.rgb += (b+d+f+h)*0.0625;
    //downsample.rgb += (j+k+l+m)*0.125;
    downsample.a = 1.0f;

    downsample = max(downsample, 0.0001f);
    downsample.rgb = QuadraticThreshold(min(downsample.rgb, 65504.0), ubo.threshold, ubo.knee);
}