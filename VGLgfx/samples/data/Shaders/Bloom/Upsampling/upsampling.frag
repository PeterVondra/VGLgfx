#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// This shader performs upsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
layout(binding = 0) uniform sampler2D src_image;
layout(binding = 1) uniform sampler2D downsampled_image;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 upsample;

layout (push_constant) uniform PushConstants {
	float filter_radius;
} pushConstants;

void main()
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = pushConstants.filter_radius;
    float y = pushConstants.filter_radius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(src_image, vec2(UV.x - x, UV.y + y)).rgb;
    vec3 b = texture(src_image, vec2(UV.x,     UV.y + y)).rgb;
    vec3 c = texture(src_image, vec2(UV.x + x, UV.y + y)).rgb;

    vec3 d = texture(src_image, vec2(UV.x - x, UV.y)).rgb;
    vec3 e = texture(src_image, vec2(UV.x,     UV.y)).rgb;
    vec3 f = texture(src_image, vec2(UV.x + x, UV.y)).rgb;

    vec3 g = texture(src_image, vec2(UV.x - x, UV.y - y)).rgb;
    vec3 h = texture(src_image, vec2(UV.x,     UV.y - y)).rgb;
    vec3 i = texture(src_image, vec2(UV.x + x, UV.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    upsample.rgb = e*4.0;
    upsample.rgb += (b+d+f+h)*2.0;
    upsample.rgb += (a+c+g+i);
    upsample.rgb *= 1.0 / 16.0;
    //upsample.rgb += texture(downsampled_image, UV).rgb;
    upsample.a = 1.0f;
}