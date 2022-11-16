#pragma once

#include "Math/Vector.h"

namespace vgl
{
    // For HDR and FXAA
	struct HDRFXAAInfo {
		//bool autoExposure;
		float deltatime = 0.0f;

        float gamma = 2.2f;
		float exposure = 1.525f;
		float exposure_adapt_rate = 0.001;

		float FXAA_Span_Max = 16.0f;
		float FXAA_REDUCTION_MIN = 1.0f / 128.0f;
		float FXAA_REDUCTION_BIAS = 1.0f / 8.0f;

		float filmicScale = 32;
		float filmicStrength = 10;

		int vignetting = 1; //use optical lens vignetting?
		float vignout = 0.5; //vignetting outer border
		float vignin = 0.0; //vignetting inner border
		float vignfade = 22.0; //f-stops till vignete fades
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
		float lumaThreshold = 2.532f;
		float BlurScale = 3.0f;
		int samples = 16;
	};

	// Depth of field effect
	struct DOFInfo
	{
		int autofocus = 1;
		int noise = 1; //use noise instead of pattern for sample dithering
		int showFocus = 0; //show debug focus point and focal range (red = focal point, green = focal range)
		float focalDepth = 1.5;
		float focalLength = 2.79f;
		float fstop = 7.7f;
		float maxblur = 2.3f;
		Vector2f focus = { 0.5, 0.5 }; // Focus point

		float namount = 0.0001; //dither amount

		int samples = 4;
		int rings = 4;
		float CoC = 0.09;//circle of confusion size in mm (35mm film = 0.03mm)
		float threshold = 0.8; //highlight threshold;
		float gain = 0.1f; //highlight gain;

		float bias = 0.5f; //bokeh edge bias
		float fringe = 0.7f; //bokeh chromatic aberration/fringing
	};
}