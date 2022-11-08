#pragma once

#include "VkImage.h"
#include "VkVertexArray.h"
#include "VkPipeline.h"
#include "../../VGL-Core/Math/Transform.h"
#include "VkFramebufferAttachment.h"
#include <vector>

namespace vgl
{
	// For HDR and FXAA
	struct HDRFXAAInfo {
		//bool autoExposure;
		float gamma = 1.185f;
		float exposure = 1.855f;
		float exposure_adapt_rate = 0.007;
		float FXAA_Span_Max = 16.0f;
		float FXAA_REDUCTION_MIN = 1.0f / 128.0f;
		float FXAA_REDUCTION_BIAS = 1.0f / 8.0f;
		float deltaTime = 0.0f;
		float filmicStrength = 10.0f;
		float filmicScale = 10.0f;
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
		int32_t samples = 16;
	};

	// Depth of field effect
	struct DOFInfo
	{
		int32_t autofocus = true;
		int32_t	 noise = true; //use noise instead of pattern for sample dithering
		int32_t showFocus = false; //show debug focus point and focal range (red = focal point, green = focal range)
		int32_t vignetting = true; //use optical lens vignetting?
		float focalDepth = 1.5;
		float focalLength = 1.0f;
		float fstop = 6.0f;
		float maxblur = 2.3f;
		Vector2f focus = { 0.5, 0.5 }; // Focus point

		float namount = 0.0001; //dither amount

		float vignout = 0.5; //vignetting outer border
		float vignin = 0.0; //vignetting inner border
		float vignfade = 22.0; //f-stops till vignete fades

		int32_t samples = 4;
		int32_t rings = 4;
		float CoC = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)
		float threshold = 0.8; //highlight threshold;
		float gain = 0.1f; //highlight gain;

		float bias = 0.5; //bokeh edge bias
		float fringe = 0.7; //bokeh chromatic aberration/fringing
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

		class GraphicsContext
		{
			public:

				static void init();

				static void createSkyboxPipelines(RenderPass& p_RenderPass);
				// Pass GBuffer RenderPass
				static void createShapesPipelines(RenderPass& p_RenderPass);

				inline static Shader& getDShadowMapShader() { static Shader m_DShadowMapShader; return m_DShadowMapShader; }
				inline static Shader& getDShadowMapAlbedoShader() { static Shader m_DShadowMapAlbedoShader; return m_DShadowMapAlbedoShader; }

				static CubeMap	generateAtmosphericScatteringCubeMap(Vector2i p_Size, AtmosphericScatteringInfo& p_Info);
				static CubeMap	getCubeMapFromHDR(Image& p_HDRImage, Vector2i p_Size);
				static CubeMap	generateIrradianceMap(CubeMap& p_CubeMapImage, Vector2i p_Size);
				static CubeMap	generatePreFilteredMaps(CubeMap& p_CubeMapImage, Vector2i p_InitialSize);
				//static Image	generateBRDFLut(Vector2f p_Size);

				static const VertexArray& getRecVao();
				static VertexBuffer& getRecVertexBuffer();
				static IndexBuffer& getRecIndexBuffer();

			private:
				friend class Renderer;

				static Pipeline m_TextBoxPipeline;
				static Pipeline m_Shape2DPipeline;
				static Pipeline m_Circle2DPipeline;
				static Pipeline m_ParticlePipeline;
				
				static Shader m_TextBoxShader;
				static Shader m_Shape2DShader;
				static Shader m_Circle2DShader;
				static Shader m_ParticleShader;
				
				inline static Pipeline& getAtmosphericScatteringPipeline() { static Pipeline m_AtmosphericScatteringPipeline; return m_AtmosphericScatteringPipeline; }
				inline static Pipeline& getSkyBoxPipeline() { static Pipeline m_SkyBoxPipeline; return m_SkyBoxPipeline; }
				inline static Shader& getSkyBoxShader() { static Shader m_SkyBoxShader; return m_SkyBoxShader; }
				//static Pipeline& getAtmosphericScatteringPipelineC16SF() { static Pipeline m_AtmosphericScatteringPipeline; return m_AtmosphericScatteringPipeline; }
				//static Pipeline& getAtmosphericScatteringPipelineC16SFD() { static Pipeline m_AtmosphericScatteringPipeline; return m_AtmosphericScatteringPipeline; }


				inline static Shader& getAtmosphericScatteringShader() { static Shader m_AtmosphericScatteringShader; return m_AtmosphericScatteringShader; }
				//inline static Pipeline& getSkyBoxPipeline() { static Pipeline m_SkyBoxPipeline; return m_SkyBoxPipeline; }
				//inline static Shader& getSkyBoxShader() { static Shader m_SkyBoxShader; return m_SkyBoxShader; }

				inline static Pipeline& getDShadowMapPipeline() { static Pipeline m_DShadowMapPipeline; return m_DShadowMapPipeline; }
				inline static Pipeline& getDShadowMapAlbedoPipeline() { static Pipeline m_DShadowMapAlbedoPipeline; return m_DShadowMapAlbedoPipeline; }
				inline static RenderPass& getDShadowMapRenderPass() { static RenderPass m_DShadowMapRenderPass(RenderPassType::Graphics); return m_DShadowMapRenderPass; }
				inline static VkDescriptorSetLayout& getDShadowMapDescriptorSetLayout() {
					static VkDescriptorSetLayout m_DShadowMapDescriptorSetLayout = 
						ContextSingleton::getInstance().m_DescriptorSetLayoutCache.createDescriptorSetLayout({ {0} });
					return m_DShadowMapDescriptorSetLayout;
				}
				inline static VkDescriptorSetLayout& getDShadowMapAlbedoDescriptorSetLayout() {
					static VkDescriptorSetLayout m_DShadowMapAlbedoDescriptorSetLayout =
						ContextSingleton::getInstance().m_DescriptorSetLayoutCache.createDescriptorSetLayout({ 0 }, {}, { {1} });
					return m_DShadowMapAlbedoDescriptorSetLayout;
				}

				static ::std::vector<Vector2f> m_RecRawVertices;
				static VertexBuffer m_RecVertices;

				static ::std::vector<uint32_t> m_RecRawIndices;
				static IndexBuffer m_RecIndices;

				static BufferLayout m_RecLayout;

				static VertexArray m_RecVao;
		};
	}
}