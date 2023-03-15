#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define HORIZONTAL 1
#define VERTICAL 2
#define PI 3.141592
#define EPSILON 0.0000001

layout(binding = 0) uniform sampler2D imageHDR;
layout(binding = 1) uniform sampler2D imageBloom;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstants {
	float deltaTime;
	float gamma; // 1.7
	float exposure; // 0.7
	float exposure_adapt_rate; // 0.001
    float FXAA_Span_Max;// = 16.0f;
    float FXAA_REDUCTION_MIN;// = 1.0f/128.0f;
    float FXAA_REDUCTION_BIAS;// = 1.0f/8.0f;
	float filmicScale;
	float filmicStrength;
} pushConstants;

layout(binding = 1) uniform UniformBufferObject
{
	int autofocus;
	int	noise; //use noise instead of pattern for sample dithering
	int showFocus; //show debug focus point and focal range (red = focal point, green = focal range)
	int vignetting; //use optical lens vignetting?
	float focalDepth;
	float focalLength;
	float fstop;
	float maxblur;
	vec2 focus; // Focus point

	float namount; //dither amount

	float vignout; //vignetting outer border
	float vignin; //vignetting inner border
	float vignfade; //f-stops till vignete fades

	int samples;
	int rings;
	float CoC;//circle of confusion size in mm (35mm film = 0.03mm)
	float threshold; //highlight threshold;
	float gain; //highlight gain;

	float bias; //bokeh edge bias
	float fringe; //bokeh chromatic aberration/fringing
}ubo;

layout(std140, set = 0, binding = 2) restrict buffer PrevFrameData
{
	float prevExposure;
	float prevDepth;
}sbo;

vec3 filmicgrain(vec4 color)
{
	float x = (UV.x + 4)*(UV.y+4)*sin(1.0f-pushConstants.deltaTime)*pushConstants.filmicScale;
	return vec3(mod((mod(x, 13) + 1) * (mod(x, 123) + 1), 0.01) - 0.005) * pushConstants.filmicStrength;
}

vec2 screenWH = textureSize(imageHDR, 0);
float near = 1.0f;
float far = 1000.0f;

bool manualdof = false;

float ndofstart = 1.0; //near dof blur start
float ndofdist = 10.0; //near dof blur falloff distance
float fdofstart = 1.0; //far dof blur start
float fdofdist = 10.0; //far dof blur falloff distance

bool depthblur = true ; //blur the depth buffer?
float dbsize = 1.25; //depthblursize

vec2 texel = vec2(1.0f, 1.0f) / screenWH;

float bdepth(vec2 coords) //blurring depth
{
	float d = 0.0;
	float kernel[9];
	vec2 offset[9];
	
	vec2 wh = vec2(texel.x, texel.y) * dbsize;
	
	offset[0] = vec2(-wh.x,-wh.y);
	offset[1] = vec2( 0.0, -wh.y);
	offset[2] = vec2( wh.x -wh.y);
	
	offset[3] = vec2(-wh.x,  0.0);
	offset[4] = vec2( 0.0,   0.0);
	offset[5] = vec2( wh.x,  0.0);
	
	offset[6] = vec2(-wh.x, wh.y);
	offset[7] = vec2( 0.0,  wh.y);
	offset[8] = vec2( wh.x, wh.y);
	
	kernel[0] = 1.0/16.0;   kernel[1] = 2.0/16.0;   kernel[2] = 1.0/16.0;
	kernel[3] = 2.0/16.0;   kernel[4] = 4.0/16.0;   kernel[5] = 2.0/16.0;
	kernel[6] = 1.0/16.0;   kernel[7] = 2.0/16.0;   kernel[8] = 1.0/16.0;
	
	
	for( int i=0; i<9; i++ )
	{
		float tmp = texture(imageHDR, coords + offset[i]).w;
		d += tmp * kernel[i];
	}
	
	return d;
}

vec3 colorDOF(vec2 coords,float blur) //processing the sample
{
	vec3 col = vec3(0.0);
	
	col.r = texture(imageHDR,coords + vec2(0.0,1.0)*texel*ubo.fringe*blur).r;
	col.g = texture(imageHDR,coords + vec2(-0.866,-0.5)*texel*ubo.fringe*blur).g;
	col.b = texture(imageHDR,coords + vec2(0.866,-0.5)*texel*ubo.fringe*blur).b;
	
	vec3 lumcoeff = vec3(0.299,0.587,0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum-ubo.threshold)*ubo.gain, 0.0);
	return col+mix(vec3(0.0),col,thresh*blur);
}

vec3 debugFocus(vec3 col, float blur, float depth)
{
	
	float edge = 0.002*depth; //distance based edge smoothing
	float m = clamp(smoothstep(0.0,edge,blur),0.0,1.0);
	float e = clamp(smoothstep(1.0-edge,1.0,blur),0.0,1.0);
	
	col = mix(col,vec3(1.0,0.5,0.0),(1.0-m)*0.6);
	col = mix(col,vec3(0.0,0.5,1.0),((1.0-e)-(1.0-m))*0.2);
	
	return col;
}

vec2 rand(vec2 coord) //generating noise/pattern texture for dithering
{
	float noiseX = ((fract(1.0-coord.s*(screenWH.x/2.0))*0.25)+(fract(coord.t*(screenWH.y/2.0))*0.75))*2.0-1.0;
	float noiseY = ((fract(1.0-coord.s*(screenWH.x/2.0))*0.75)+(fract(coord.t*(screenWH.y/2.0))*0.25))*2.0-1.0;
	
	if (ubo.noise > 0)
	{
		noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
		noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
	}
	return vec2(noiseX,noiseY);
}

float linearize(float depth)
{
	return -far * near / (depth * (far - near) - far);

}

float vignette()
{
	float dist = distance(UV.xy, vec2(0.5,0.5));
	dist = smoothstep(ubo.vignout+(ubo.fstop/ubo.vignfade), ubo.vignin+(ubo.fstop/ubo.vignfade), dist);
	return clamp(dist,0.0,1.0);
}

// Tonemapping
vec3 ACESFitted(vec3 color);
vec3 ACEST2(vec3 x);
vec3 LinearTosRGB(vec3 color);
vec3 SRGBToLinear(vec3 color);

void main()
{
	float time_c = (1-exp(-pushConstants.deltaTime*pushConstants.exposure_adapt_rate));

    //scene depth calculation
	
    float depth = texture(imageHDR,UV.xy).w;
	
	if (depthblur)
		depth = linearize(bdepth(UV.xy));

	//focal plane calculation
	
	float fDepth = ubo.focalDepth;
	
	if (ubo.autofocus > 0)
		fDepth = linearize(texture(imageHDR,ubo.focus).w);

	sbo.prevDepth = fDepth;
	
	//dof blur factor calculation
	
	float blur = 0.0;
	
	if (manualdof)
	{
		float a = depth-fDepth; //focal plane
		float b = (a-fdofstart)/fdofdist; //far DoF
		float c = (-a-ndofstart)/ndofdist; //near Dof
		blur = (a>0.0)?b:c;
	}
	
	else
	{
		float f = ubo.focalLength; //focal length in mm
		float d = fDepth*1000.0; //focal plane in mm
		float o = depth*1000.0; //depth in mm
		
		float a = (o*f)/(o-f);
		float b = (d*f)/(d-f);
		float c = (d-f)/(d*ubo.fstop*ubo.CoC);
		
		blur = abs(a-b)*c;
	}
	
	blur = clamp(blur,0.0,1.0);
	
	// calculation of pattern for ditering
	vec2 noise = rand(UV.xy)*ubo.namount*blur;
	
	// getting blur x and y step factor
	
	float w = (1.0f/screenWH.x)*blur*ubo.maxblur+noise.x;
	float h = (1.0f/screenWH.y)*blur*ubo.maxblur+noise.y;
	
	// calculation of final color
	
	vec3 col = vec3(0.0);
	
	if(blur >= 0.03) // optimization
	{
		col = texture(imageHDR, UV.xy).rgb;
		float s = 1.0;
		int ringsamples;
		
		for (int i = 1; i <= ubo.rings; i += 1)
		{
			ringsamples = i * ubo.samples;
			
			for (int j = 0 ; j < ringsamples ; j += 1)
			{
				float step = PI*2.0 / float(ringsamples);
				float pw = (cos(float(j)*step)*float(i));
				float ph = (sin(float(j)*step)*float(i));
				float p = 1.0;
				col += colorDOF(UV.xy + vec2(pw*w,ph*h),blur)*mix(1.0,(float(i))/(float(ubo.rings)),ubo.bias)*p;
				s += 1.0*mix(1.0,(float(i))/(float(ubo.rings)),ubo.bias)*p;
				
			}
		}
		col /= s; //divide by sample count
	}
	
	if (ubo.showFocus > 0)
		col = debugFocus(col, blur, depth);
		
    vec3 color = col;

	const float gamma = pushConstants.gamma;
	float exposure = pushConstants.exposure;
    //vec3 bloomColor = texture(imageBloom, UV).rgb;
    
	if(blur < 0.03){
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
    
		color = result2;//64;
		if(lumaResult2 < lumaMin || lumaResult2 > lumaMax)
		    color = result1;//64;
    }
	
   // color += bloomColor;
	//if (vignetting)
	//	color *= vignette();
    
    // tone mapping
	vec3 avgColor = textureLod(imageHDR, vec2(0.5f, 0.5f), 1000.0).rgb + textureLod(imageHDR, vec2(0.5f, 0.5f), 9.0).rgb/2;
	float avgLuma = dot(avgColor/1.5, vec3(0.2125, 0.7154, 0.0721));

	//avgLuma = sbo.prevAvgLuma + (avgLuma- sbo.prevAvgLuma)*time_c;

	float S = 100;
	float K = 12.5;
	float Q = 0.65;
	float EV100 = log2(avgLuma*(S/K));
	
	exposure = (pushConstants.exposure)/((78/(Q*S))*exp2(EV100));
	
	exposure = abs(sbo.prevExposure + (exposure - sbo.prevExposure)*time_c);
	
	exposure = max(0.7, exposure);

	//exposure = pushConstants.exposure * mix(sbo.prevExposure, 0.5/avgLuma, 0.02);
	
	sbo.prevExposure = exposure;

    vec3 result = vec3(1.0) - exp(-color * exposure);
	
    // also gamma correct while we're at it       
    result = pow(LinearTosRGB(ACESFitted(result)), vec3(1.0 / gamma));
    outColor = vec4(color, 1.0);// * ditherPattern;

	if(ubo.vignetting > 0)
		outColor = vec4(result * vignette(), 1.0);

	outColor += vec4(filmicgrain(outColor), 1.0f);
}

vec3 linearToneMapping(vec3 p_Color, p_Exposure)
{
	color = vec3(1.0) - exp(-color * exposure);
	color = pow(color, vec3(1. / ubo.gamma));
	return color;
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
vec3 ACEST2(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

HablePartial(vec3 color)
{
	float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}
HableToneMapping(vec3 color, float p_Exposure)
{
    vec3 curr = HablePartial(v * p_Exposure);

    vec3 W = vec3(11.2f);
    vec3 white_scale = vec3(1.0f) / HablePartial(W);
    return curr * white_scale;	
}

vec3 LinearTosRGB(vec3 color)
{
    vec3 x = color * 12.92f;
    vec3 y = 1.055f * pow(clamp(color, 0.0f, 1.0f), vec3(1.0f / ubo.gamma)) - 0.055f;

    vec3 clr = color;
    clr.r = color.r < 0.0031308f ? x.r : y.r;
    clr.g = color.g < 0.0031308f ? x.g : y.g;
    clr.b = color.b < 0.0031308f ? x.b : y.b;

    return clr;
}

vec3 SRGBToLinear(vec3 color)
{
    vec3 x = color / 12.92f;
    vec3 y = pow(max((color + 0.055f) / 1.055f, 0.0f), vec3(ubo.gamma));

    vec3 clr = color;
    clr.r = color.r <= 0.04045f ? x.r : y.r;
    clr.g = color.g <= 0.04045f ? x.g : y.g;
    clr.b = color.b <= 0.04045f ? x.b : y.b;

    return clr;
}