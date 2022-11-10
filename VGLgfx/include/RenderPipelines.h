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

			// Geometry and material data / G-Buffer
			FramebufferAttachment m_GBuffer;
			// Lighting pass - no post-process
			FramebufferAttachment m_LightPassFramebuffer;
			// Post-Process framebuffer
			FramebufferAttachment m_HDRFramebuffer;

			HDRFXAAInfo m_HDRInfo; // HDR effects
			Shader m_HDRShader;
			Shader m_GBufferLightPassShader;
			Shader m_SSAOShader;
			Shader m_SSAOBlurShader;
			Shader m_SSRShader;

			// SSAO Framebuffer
			FramebufferAttachment m_SSAOFramebuffer;
			FramebufferAttachment m_SSAOBlurFramebuffer;

			std::vector<FramebufferAttachment> m_GBlurBloomFramebuffers;
			FramebufferAttachment& getGBloomFramebuffer(Vector2i p_Size);

			// SSR
			FramebufferAttachment m_SSRFramebuffer;
			struct SSR_DATA {
				float SSR_MaxDistance = 15.0f;
				// [0, 1]
				float SSR_Resolution = 0.3f;
				float SSR_Steps = 5;
				float SSR_Thickness = 0.5f;
			};
			SSR_DATA SSR_Data;

			// SSAO
			std::vector<Vector4f> SSAO_Kernel;
			std::vector<Vector4f> SSAO_Noise;
			Image SSAO_NoiseImage;
			uint32_t Sample_Count = 64;
			float SSAO_Bias = 2.5f;
			float SSAO_Radius = 40.0f;
			float SSAO_Intensity = 5.0f;

		private:
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