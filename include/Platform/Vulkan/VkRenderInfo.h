#pragma once

#include "VkRenderPass.h"
#include "VkFramebufferAttachment.h"
#include "VkShader.h"
#include "VkCommandBuffer.h"
#include "VkWindow.h"

namespace vgl
{
	// For HDR and FXAA
	struct HDRFXAAInfo {
		//bool autoExposure;
		float gamma = 0.920f;
		float exposure = 1.605f;
		float FXAA_Span_Max = 16.0f;
		float FXAA_REDUCTION_MIN = 1.0f / 128.0f;
		float FXAA_REDUCTION_BIAS = 1.0f / 8.0f;
	};

	// Atmosperic scattering
	struct AtmosphericScatteringInfo
	{
		alignas(16) Vector3f p_RayOrigin = { 0, 6372e3, 0 };			// ray origin
		alignas(16) Vector3f p_SunPos = { 0, 0.1, -1 };					// position of the sun
		alignas(4) float p_SunIntensity = 22.0;							// intensity of the sun
		alignas(4) float p_PlanetRadius = 6372e3;						// radius of the planet in meters
		alignas(4) float p_AtmosphereRadius = 6471e3;					// radius of the atmosphere in meters
		alignas(16) Vector3f p_RayleighSC = { 5.5e-6, 13.0e-6, 22.4e-6 }; // Rayleigh scattering coefficient
		alignas(4) float p_MieSC = 21e-6;								// Mie scattering coefficient
		alignas(4) float p_RayleighSHeight = 2e3;						// Rayleigh scale height
		alignas(4) float p_MieSHeight = 1.2e3;							// Mie scale height
		alignas(4) float p_MieDir = 0.858;								// Mie preferred scattering direction
		alignas(4) float p_Scale = 2.0;									// Mie preferred scattering direction
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
		bool autofocus = true;
		float focalDepth = 1.5;
		float focalLength = 1.0f;
		float fstop = 6.0f;
		float maxblur = 2.3f;
		Vector2f focus = { 0.5, 0.5 }; // Focus point

		bool showFocus = false; //show debug focus point and focal range (red = focal point, green = focal range)
		bool vignetting = true; //use optical lens vignetting?
		float vignout = 0.5; //vignetting outer border
		float vignin = 0.0; //vignetting inner border
		float vignfade = 22.0; //f-stops till vignete fades

		int samples = 4;
		int rings = 4;
		float CoC = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)
		float threshold = 0.8; //highlight threshold;
		float gain = 0.1f; //highlight gain;
		bool noise = true; //use noise instead of pattern for sample dithering
	};

	namespace vk
	{
		struct RenderInfo
		{
			bool p_AlphaBlending = false;
			CullMode p_CullMode = CullMode::BackBit;
			PolygonMode p_PolygonMode = PolygonMode::Fill;
			IATopoogy p_IATopology = IATopoogy::TriList;
		};
	}
}