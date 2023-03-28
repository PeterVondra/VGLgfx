#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
layout(binding = 0) uniform sampler2D src_image;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 denoised_image;

void main()
{
    float exponent = 2.0f;
    vec3 center = 

    vec4 color = vec4(0.0);
    float total = 0.0;
    for (float x = -4.0; x <= 4.0; x += 1.0) {
        for (float y = -4.0; y <= 4.0; y += 1.0) {
            vec4 sample0 = texture(src_image, UV + vec2(x, y) / textureSize(src_image, 0));
            float weight = 1.0 - abs(dot(sample0.rgb - center.rgb, vec3(0.25)));
            weight = pow(weight, exponent);
            color += sample0 * weight;
            total += weight;
        }
    }

    denoised_image = color / total;

}