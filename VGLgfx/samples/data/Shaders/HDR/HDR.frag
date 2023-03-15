#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define HORIZONTAL 1
#define VERTICAL 2
#define PI 3.141592
#define EPSILON 0.0000001

layout(binding = 0) uniform sampler2D scene_image;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstants {
	float deltaTime;

	float gamma; // 1.7
	float exposure; // 0.7
	float exposure_adapt_rate; // 0.001

	float filmicScale;
	float filmicStrength;

	int vignetting;		//use optical lens vignetting?
	float vignout;		//vignetting outer border
	float vignin;		//vignetting inner border
	float vignfade;		//f-stops until vignette fades
    float fstop;
} pushConstants;

layout(std140, set = 0, binding = 1) restrict buffer PrevFrameData
{
	float prevAvgLuma;
}sbo;

vec3 filmicgrain(vec4 color)
{
	float x = (UV.x + 4)*(UV.y+4)*sin(1.0f-pushConstants.deltaTime)*pushConstants.filmicScale;
	return vec3(mod((mod(x, 13) + 1) * (mod(x, 123) + 1), 0.01) - 0.005) * pushConstants.filmicStrength;
}

vec2 screenWH = textureSize(scene_image, 0);

float vignette()
{
	float dist = distance(UV.xy, vec2(0.5,0.5));
	dist = smoothstep(pushConstants.vignout+(pushConstants.fstop/pushConstants.vignfade), pushConstants.vignin+(pushConstants.fstop/pushConstants.vignfade), dist);
	return clamp(dist,0.0,1.0);
}

// Tonemapping
vec3 linearToneMapping(vec3 p_Color, float p_Exposure);
vec3 reinhardExtendedLuminance(vec3 color, float max_white_l);
vec3 HableToneMapping(vec3 color, float p_Exposure);
vec3 ACESFitted(vec3 color);
vec3 ACEST2(vec3 x);
vec3 LinearTosRGB(vec3 color);
vec3 SRGBToLinear(vec3 color);

float S = 100;
float K = 12.5;
float Q = 0.65;
float FocalLength = 2.79f;
float minISO = 100.0f;
float maxISO = 6400.0f;
float minShutterSpeed = 1.0f/4000.0f;
float maxShutterSpeed = 1.0f/30.0f;
float minAperture = 1.8f;
float maxAperture = 22.0f;
float middleGrey = 0.18f;

// Get exposure based on saturation-based speed method
float getSaturatedExposure(float p_Aperture, float p_ShutterSpeed, float p_ISO)
{
    float maxLuminance = (7800.0f / 65.0f) * sqrt(p_Aperture) / (p_ISO * p_ShutterSpeed);
    return 1.0f / maxLuminance;
}
float getStandardOutExposure(float p_Aperture, float p_ShutterSpeed, float p_ISO, float p_MiddleGrey)
{
    float avgLuminance = (1000.0f / 65.0f) * sqrt(p_Aperture) / (p_ISO * p_ShutterSpeed);
    return middleGrey / avgLuminance;
}

// Compute ISO value based on camera settings
float computeISO(float p_Aperture, float p_ShutterSpeed, float p_EV)
{
    return (sqrt(p_Aperture) * 100.0f) / (p_ShutterSpeed * pow(2.0f, p_EV));
}
// Compute current exposure based on camera settings
float computeEV(float p_Aperture, float p_ShutterSpeed, float p_ISO)
{
    return log2((sqrt(p_Aperture) * 100.0f) / (p_ShutterSpeed * p_ISO));
}
// Compute target exposure
float computeTargetEV(float p_AvgLuminance)
{
    return log2(p_AvgLuminance * 100.0f / K);
}
void setApertureWeight(float p_FocalLength, float p_TargetEV, inout float p_Aperture, inout float p_ShutterSpeed, inout float p_ISO)
{
    // Shutter speed = 1/f;
    p_ShutterSpeed = 1.0f / (FocalLength * 1000.0f);
    p_ISO = clamp(computeISO(p_Aperture, p_ShutterSpeed, p_TargetEV), minISO, maxISO);
    // Compute how far from target exposure
    float deltaEV = p_TargetEV - computeEV(p_Aperture, p_ShutterSpeed, p_ISO);
    p_ShutterSpeed = clamp(p_ShutterSpeed * pow(2.0f, -deltaEV), minShutterSpeed, maxShutterSpeed);
}
void setShutterWeight(float p_FocalLength, float p_TargetEV, inout float p_Aperture, inout float p_ShutterSpeed, inout float p_ISO)
{
    // Assume target aperture is 4.0f
    p_Aperture = 4.0f;
    p_ISO = clamp(computeISO(p_Aperture, p_ShutterSpeed, p_TargetEV), minISO, maxISO);
    float deltaEV = p_TargetEV - computeEV(p_Aperture, p_ShutterSpeed, p_ISO);
    p_Aperture = clamp(p_Aperture * pow(sqrt(2.0f), deltaEV), minAperture, maxAperture);
}
void setAutoExposure(float p_FocalLength, float p_TargetEV, inout float p_Aperture, inout float p_ShutterSpeed, inout float p_ISO)
{
    p_Aperture = 4.0f;
    p_ShutterSpeed = 1.0f / (p_FocalLength * 1000.0f);
    p_ISO = clamp(computeISO(p_Aperture, p_ShutterSpeed, p_TargetEV), minISO, maxISO);
    float deltaEV = p_TargetEV - computeEV(p_Aperture, p_ShutterSpeed, p_ISO);
    p_Aperture = clamp(p_Aperture * pow(sqrt(2.0f), deltaEV * 0.5f), minAperture, maxAperture);

    deltaEV = p_TargetEV - computeEV(p_Aperture, p_ShutterSpeed, p_ISO);
    p_ShutterSpeed = clamp(p_ShutterSpeed * pow(2.0f, -deltaEV), minShutterSpeed, maxShutterSpeed);
}

void main()
{
    float Aperture = 4.0f;
    float ShutterSpeed = 1.0f/100.0f;
    float ISO = 800.0f;

    float exposure = pushConstants.exposure;
    float time_c = (1.0f-exp(-pushConstants.deltaTime*pushConstants.exposure_adapt_rate));

    vec3 avgColor = textureLod(scene_image, vec2(0.5f, 0.5f), 1000.0).rgb + textureLod(scene_image, vec2(0.5f, 0.5f), 9.0).rgb/2;
    float avgLuma = dot(avgColor/1.5, vec3(0.2125, 0.7154, 0.0721));

	avgLuma = sbo.prevAvgLuma + (avgLuma - sbo.prevAvgLuma)*time_c;
	
    sbo.prevAvgLuma = avgLuma;

    //float EV100 = log2(avgLuma*(S/K));
	
	//exposure = (pushConstants.exposure)/((78/(Q*S))*exp2(EV100));
	
	//exposure = abs(sbo.prevExposure + (exposure - sbo.prevExposure)*time_c);
	
	//exposure = max(0.2, exposure);

	//exposure = pushConstants.exposure * mix(sbo.prevExposure, 0.5/avgLuma, 0.02);
	
	//sbo.prevExposure = exposure;

    setAutoExposure(FocalLength, computeTargetEV(avgLuma), Aperture, ShutterSpeed, ISO);
    exposure = getSaturatedExposure(Aperture, ShutterSpeed, ISO) * pushConstants.exposure;

    vec3 color = texture(scene_image, UV).rgb;
    vec3 result = vec3(1.0) - exp(-color * exposure);
	
    // also gamma correct while we're at it
    //outColor.rgb = pow(reinhardExtendedLuminance(result, 70.0f), vec3(1.0 / pushConstants.gamma));
    outColor.rgb = LinearTosRGB(reinhardExtendedLuminance(result, 30.0f));
    //outColor.rgb = LinearTosRGB(ACESFitted(result));

	if(pushConstants.vignetting > 0)
		outColor = vec4(outColor.rgb * vignette(), 1.0);

	outColor += vec4(filmicgrain(outColor), 1.0f);
}

vec3 linearToneMapping(vec3 p_Color, float p_Exposure)
{
	p_Color = vec3(1.0) - exp(-p_Color * p_Exposure);
	p_Color = pow(p_Color, vec3(1. / pushConstants.gamma));
	return p_Color;
}

float luminance(vec3 color)
{
    return dot(color, vec3(0.2126f, 0.7152f, 0.0722f));
}

vec3 convert_luminance(vec3 color_in, float luminance_out)
{
    float luminance_in = luminance(color_in);
    return color_in * (luminance_out / luminance_in);
}

vec3 reinhardExtendedLuminance(vec3 color, float max_white_l)
{
    float luminance_old = luminance(color);
    float numerator = luminance_old * (1.0f + (luminance_old / (max_white_l * max_white_l)));
    float luminance_new = numerator / (1.0f + luminance_old);
    return convert_luminance(color, luminance_new);
}

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = ACESInputMat * color;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = ACESOutputMat * color;

    // Clamp to [0, 1]
    color = clamp(color, 0.0f, 1.0f);

    return color;
}

// Approximated ACES
//vec3 ACEST2(vec3 x) {
//  const float a = 2.51;
//  const float b = 0.03;
//  const float c = 2.43;
//  const float d = 0.59;
//  const float e = 0.14;
//  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
//}
//
vec3 HablePartial(vec3 color)
{
	float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}
vec3 HableToneMapping(vec3 p_Color, float p_Exposure)
{
    vec3 curr = HablePartial(p_Color * p_Exposure);

    vec3 W = vec3(11.2f);
    vec3 white_scale = vec3(1.0f) / HablePartial(W);
    return curr * white_scale;	
}

vec3 LinearTosRGB(vec3 color)
{
    vec3 x = color * 12.92f;
    vec3 y = 1.055f * pow(clamp(color, 0.0f, 1.0f), vec3(1.0f / pushConstants.gamma)) - 0.055f;

    vec3 clr = color;
    clr.r = color.r < 0.0031308f ? x.r : y.r;
    clr.g = color.g < 0.0031308f ? x.g : y.g;
    clr.b = color.b < 0.0031308f ? x.b : y.b;

    return clr;
}

vec3 SRGBToLinear(vec3 color)
{
    vec3 x = color / 12.92f;
    vec3 y = pow(max((color + 0.055f) / 1.055f, 0.0f), vec3(pushConstants.gamma));

    vec3 clr = color;
    clr.r = color.r <= 0.04045f ? x.r : y.r;
    clr.g = color.g <= 0.04045f ? x.g : y.g;
    clr.b = color.b <= 0.04045f ? x.b : y.b;

    return clr;
}