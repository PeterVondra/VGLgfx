#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define HORIZONTAL 1
#define VERTICAL 2
#define PI 3.141592
#define EPSILON 0.0000001

layout(binding = 0) uniform sampler2D imageHDR;
//layout(binding = 1) uniform sampler2D imageBloom;

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
float far = 10.0f;

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
	//vec2 uv = UV.xy;
   	//
    //uv *=  1.0 - uv.yx;   //vec2(1.0)- uv.yx; -> 1.-u.yx; Thanks FabriceNeyret !
    //
    //float vig = uv.x*uv.y * 20.0; // multiply with sth for intensity
    //
    //vig = pow(vig, 0.5); // change pow for modifying the extend of the  vignette
	//return vig;
	float dist = distance(UV.xy, vec2(0.5,0.5));
	dist = smoothstep(ubo.vignout+(ubo.fstop/ubo.vignfade), ubo.vignin+(ubo.fstop/ubo.vignfade), dist);
	return clamp(dist,0.0,1.0);
}

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
    result = pow(result, vec3(1.0 / gamma));
    outColor = vec4(col, 1.0);// * ditherPattern;
	if(ubo.vignetting > 0)
		outColor = vec4(result * vignette(), 1.0);

	outColor += vec4(filmicgrain(outColor), 1.0f);
}