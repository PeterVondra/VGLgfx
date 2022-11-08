#include "Application/RenderAPI.h"
#include "Scene.h"
#include "../DefaultECSComponents.h"
#include "../VGL-3D/Particles.h"

namespace vgl
{
	class RenderPipeline_Forward
	{
		public:
			RenderPipeline_Forward();
			~RenderPipeline_Forward();

			void setup(Renderer* p_Renderer, vk::Window* p_Window, Scene* p_Scene);
			void render(vk::RenderInfo& p_RenderInfo);

			// Lighting pass - no post-process
			vk::FramebufferAttachment m_LightPassFramebuffer;

			// Post-Process framebuffer
			DOFInfo m_DOFInfo; // Depth of Field
			HDRFXAAInfo m_HDRInfo;
			vk::Shader m_HDRShader;
			vk::FramebufferAttachment m_HDRFramebuffer;

		private:
			Renderer* m_RendererPtr = nullptr;
			vk::Window* m_WindowPtr = nullptr;
			Scene* m_ScenePtr = nullptr;
	};

	class RenderPipeline_ForwardPlus
	{
		public:
			RenderPipeline_ForwardPlus();
			~RenderPipeline_ForwardPlus();

			void setup(Renderer& p_Renderer, Scene& p_Scene);
			void render(vk::RenderInfo& p_RenderInfo);

		private:
	};

	class RenderPipeline_Deferred
	{
		public:
			RenderPipeline_Deferred();
			~RenderPipeline_Deferred();

			void setup(Renderer* p_Renderer, vk::Window* p_Window, Scene* p_Scene);
			void render(vk::RenderInfo& p_RenderInfo);

			float volumetric_light_mie_scattering = 1;
			DOFInfo m_DOFInfo; // Depth of Field

			// Geometry and material data / G-Buffer
			vk::FramebufferAttachment m_GBuffer;
			// Lighting pass - no post-process
			vk::FramebufferAttachment m_LightPassFramebuffer;
			// Post-Process framebuffer
			vk::FramebufferAttachment m_HDRFramebuffer;

			HDRFXAAInfo m_HDRInfo; // HDR effects
			vk::Shader m_HDRShader;
			vk::Shader m_GBufferLightPassShader;
			vk::Shader m_SSAOShader;
			vk::Shader m_SSAOBlurShader;
			vk::Shader m_SSRShader;

			// SSAO Framebuffer
			vk::FramebufferAttachment m_SSAOFramebuffer;
			vk::FramebufferAttachment m_SSAOBlurFramebuffer;

			std::vector<vk::FramebufferAttachment> m_GBlurBloomFramebuffers;
			vk::FramebufferAttachment& getGBloomFramebuffer(Vector2i p_Size);

			// SSR
			vk::FramebufferAttachment m_SSRFramebuffer;
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
			vk::Image SSAO_NoiseImage;
			uint32_t Sample_Count = 64;
			float SSAO_Bias = 2.5f;
			float SSAO_Radius = 40.0f;
			float SSAO_Intensity = 5.0f;

		private:
			Renderer* m_RendererPtr = nullptr;
			vk::Window* m_WindowPtr = nullptr;
			Scene* m_ScenePtr = nullptr;
	};

	class RenderPipeline_DeferredTiled
	{
		public:
			RenderPipeline_DeferredTiled();
			~RenderPipeline_DeferredTiled();

			void setup(Renderer& p_Renderer, Scene& p_Scene);
			void render(vk::RenderInfo& p_RenderInfo);

		private:
	};
}