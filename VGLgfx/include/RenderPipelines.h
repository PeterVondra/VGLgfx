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

			Shader m_HDRShader;
			Shader m_GBufferLightPassShader;
			Shader m_SSAOShader;
			Shader m_SSAOBlurShader;
			Shader m_DOFShader;
			Shader m_FXAAShader;
			Shader m_SSRShader;

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

			void transferGBufferData();
			void transferSSAOData();
			void transferSSRData();
			void transferLightPassData();
			void transferPostProcessData();

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