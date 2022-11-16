#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PI 3.141592
#define EPSILON 0.0000001

// Bloom applied image
layout(binding = 0) uniform sampler2D imageHDR;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformBufferObject
{
	int autofocus;
	int	noise; //use noise instead of pattern for sample dithering
	int showFocus; //show debug focus point and focal range (red = focal point, green = focal range)
	float focalDepth;
	float focalLength;
	float fstop;
	float maxblur;
	vec2 focus; // Focus point

	float namount; //dither amount

	int samples;
	int rings;
	float CoC;//circle of confusion size in mm (35mm film = 0.03mm)
	float threshold; //highlight threshold;
	float gain; //highlight gain;

	float bias; //bokeh edge bias
	float fringe; //bokeh chromatic aberration/fringing
}ubo;

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

void main()
{
    //scene depth calculation
	
    float depth = texture(imageHDR,UV.xy).w;
	
	if (depthblur)
		depth = linearize(bdepth(UV.xy));

	//focal plane calculation
	
	float fDepth = ubo.focalDepth;
	
	if (ubo.autofocus > 0)
		fDepth = linearize(texture(imageHDR,ubo.focus).w);
	
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
		
    outColor = vec4(col, 1.0f);
}