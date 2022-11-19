#pragma once

#include "Math/Vector.h"

namespace vgl
{
    // For HDR and FXAA
	struct HDRInfo {
		//bool autoExposure;
		float deltatime = 0.0f;

        alignas(4) float gamma = 2.2f;
		alignas(4) float exposure = 2.56f;
		alignas(4) float exposure_adapt_rate = 0.025;

		alignas(4) float filmicScale = 32;
		alignas(4) float filmicStrength = 10;

		alignas(4) int vignetting = 1; //use optical lens vignetting?
		alignas(4) float vignout = 0.5; //vignetting outer border
		alignas(4) float vignin = 0.0; //vignetting inner border
		alignas(4) float vignfade = 22.0; //f-stops till vignete fades
		alignas(4) float fstop = 7.7f;
	};

	struct FXAAInfo
	{
		alignas(4) float FXAA_Span_Max = 16.0f;
		alignas(4) float FXAA_REDUCTION_MIN = 1.0f / 128.0f;
		alignas(4) float FXAA_REDUCTION_BIAS = 1.0f / 8.0f;
	};

	// Atmosperic scattering
	struct AtmosphericScatteringInfo
	{
		alignas(4) Vector3f p_RayOrigin = { 0, 6372e3, 0 };					// ray origin
		alignas(16) Vector3f p_SunPos = { 0, 0.1, -1 };						// position of the sun
		alignas(4) float p_SunIntensity = 100.0;							// intensity of the sun
		alignas(4) float p_PlanetRadius = 6372e3;							// radius of the planet in meters
		alignas(4) float p_AtmosphereRadius = 6471e3;						// radius of the atmosphere in meters
		alignas(16) Vector3f p_RayleighSC = { 5.5e-6, 13.0e-6, 22.4e-6 };	// Rayleigh scattering coefficient
		alignas(4) float p_MieSC = 21e-6;									// Mie scattering coefficient
		alignas(4) float p_RayleighSHeight =  2e3;							// Rayleigh scale height
		alignas(4) float p_MieSHeight = 1.2e3;								// Mie scale height
		alignas(4) float p_MieDir =  0.858;									// Mie preferred scattering direction
		alignas(4) float p_Scale = 2.0;										// Mie preferred scattering direction
	};

	// Bloom effect
	struct BloomInfo
	{
		alignas(4) float lumaThreshold = 2.532f;
		alignas(4) float BlurScale = 3.0f;
		alignas(4) int samples = 16;
	};

	// Depth of field effect
	struct DOFInfo
	{
		int autofocus = 1;
		alignas(4) int noise = 1; //use noise instead of pattern for sample dithering
		alignas(4) int showFocus = 0; //show debug focus point and focal range (red = focal point, green = focal range)
		alignas(4) float focalDepth = 1.5;
		alignas(4) float focalLength = 2.79f;
		alignas(4) float fstop = 7.7f;
		alignas(4) float maxblur = 2.3f;
		alignas(8) Vector2f focus = { 0.5, 0.5 }; // Focus point

		alignas(4) float namount = 0.0001; //dither amount

		alignas(4) int samples = 4;
		alignas(4) int rings = 4;
		alignas(4) float CoC = 0.09;//circle of confusion size in mm (35mm film = 0.03mm)
		alignas(4) float threshold = 0.8; //highlight threshold;
		alignas(4) float gain = 0.1f; //highlight gain;
		
		alignas(4) float bias = 0.5f; //bokeh edge bias
		alignas(4) float fringe = 0.7f; //bokeh chromatic aberration/fringing
	};
}