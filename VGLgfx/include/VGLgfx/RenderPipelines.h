#pragma once

#include "VGL_Internal.h"
#include "Scene.h"
#include "DefaultECSComponents.h"
//#include "../VGL-3D/Particles.h"
#include "Platform/Definitions.h"
#include "GFXDefinitions.h"
#include "Scene.h"

namespace vgl
{
	class RenderPipeline_Deferred
	{
		public:
			RenderPipeline_Deferred();
			~RenderPipeline_Deferred();

			void setup(Renderer* p_Renderer, Window* p_Window, Scene* p_Scene);
			void render(RenderInfo& p_RenderInfo);

			float volumetric_light_mie_scattering = 1;
			DOFInfo m_DOFInfo = {}; // Depth of Field
			HDRInfo m_HDRInfo; // HDR effects
			FXAAInfo m_FXAAInfo; // Anti aliasing

			// Geometry and material data / G-Buffer
			FramebufferAttachment m_GBuffer;
			// Lighting pass - no post-process
			FramebufferAttachment m_LightPassFramebuffer;
			// Post-Process framebuffers
			FramebufferAttachment m_DOFFramebuffer;
			FramebufferAttachment m_FXAAFramebuffer;
			FramebufferAttachment m_HDRFramebuffer;
			FramebufferAttachment m_VoxelFramebuffer;
			
			uint32_t m_DownsampleMipLevels = 1;
			std::vector<FramebufferAttachment> m_DownsamplingFramebuffers;
			uint32_t m_UpsampleMipLevels = 1;
			std::vector<FramebufferAttachment> m_UpsamplingFramebuffers;

			FramebufferAttachment m_PrefilterFramebufferAttachment;
			FramebufferAttachment m_BloomFramebufferAttachment;
			
			// For upsampling (Bloom)
			float m_FilterRadius = 0.001f;
			float m_BloomThreshold = 0.004f;
			Vector3f m_BloomKnee = { 1.0f, 0.5f, 0.01f };
			struct BloomMod {
				float intensity = 1.54f;
				float contribution = 0.025f;
			};
			BloomMod m_BloomMod;

			Shader m_HDRShader;
			Shader m_GBufferLightPassShader;
			Shader m_SSAOShader;
			Shader m_SSAOBlurShader;
			Shader m_DOFShader;
			Shader m_FXAAShader;
			Shader m_SSRShader;
			Shader m_DownsamplingShader;
			Shader m_UpsamplingShader;
			Shader m_PrefilterShader;
			Shader m_BloomShader;
			Shader m_VoxelShader;

			// SSAO Framebuffer
			FramebufferAttachment m_SSAOFramebuffer;
			FramebufferAttachment m_SSAOBlurFramebuffer;

			// SSR
			FramebufferAttachment m_SSRFramebuffer;
			struct SSR_DATA {
				float SSR_StepLength = -0.1f;
				float SSR_MaxSteps = 50;
				float SSR_BinarySearchSteps = 5;
				float SSR_ReflectionSpecularFalloffExponent = 3.0f;
			};
			SSR_DATA SSR_Data;

			// SSAO
			std::vector<Vector4f> SSAO_Kernel;
			std::vector<Vector4f> SSAO_Noise;
			Image SSAO_NoiseImage;
			uint32_t Sample_Count = 64;
			float SSAO_Bias = 2.5f;
			float SSAO_Radius = 50.0f;
			float SSAO_Intensity = 10.0f;

		private:
			void setupGBuffer();
			void setupSSAOBuffers();
			void setupLightPassBuffer();
			void setupSSRBuffer();
			void setupPostProcessBuffers();
			void setupBloomBuffers();
			void setupVoxelBuffers();

			void transferGBufferData();
			void transferSSAOData();
			void transferSSRData();
			void transferLightPassData();
			void transferPostProcessData();
			void transferBloomData();

			Renderer* m_RendererPtr = nullptr;
			Window* m_WindowPtr = nullptr;
			Scene* m_ScenePtr = nullptr;
	};

	class RenderPipeline_DeferredTiled
	{
		public:
			RenderPipeline_DeferredTiled();
			~RenderPipeline_DeferredTiled();

			void setup(Renderer& p_Renderer, Scene& p_Scene);
			void render(RenderInfo& p_RenderInfo);

		private:
	};
}