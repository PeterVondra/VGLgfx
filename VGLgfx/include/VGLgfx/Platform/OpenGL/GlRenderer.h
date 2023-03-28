#pragma once

#include "GlWindow.h"
#include "GlFramebufferAttachment.h"

#include "../Definitions.h"

namespace vgl
{
	namespace gl
	{
		// DESCRIPTION
		class Renderer
		{
		public:
			Renderer();
			Renderer(Window* p_Window);
			Renderer(Window& p_Window);
			Renderer(Window& p_Window, float p_RenderResolutionScale);

			void beginRenderPass(RenderInfo& p_RenderInfo);
			// If not called, will use default frambuffer/swapchain
			void beginRenderPass(RenderInfo& p_RenderInfo, FramebufferAttachment& p_FramebufferAttachment);
			// Renders into the shadow map
			//void beginRenderPass(RenderInfo& p_RenderInfo, D_ShadowMap& p_ShadowMap);

			// If not called, will use default viewport data from current framebuffer
			void submit(Viewport& p_Viewport);

			/* 3D render pass submissions */
			void submit(D_ShadowMap& p_ShadowMap);
			//void submit(D_Light& p_LightSrc);
			void submit(S_Light& p_LightSrc);
			void submit(P_Light& p_LightSrc);
			// Use custom shader for next drawcall
			void submit(Shader& p_Shader);
			// Submits to push constant
			void submit(void* p_Data, const uint32_t p_Size);
			void submit(Image& p_Image, const uint32_t p_Binding);
			// Size() == swapchain image count
			void submit(std::vector<ImageAttachment>& p_Images, const uint32_t p_Binding);
			// Render only single material
			void submit(MeshData& p_MeshData, const uint32_t p_MTLIndex, Transform3D& p_Transform);
			void submit(MeshData& p_MeshData, Transform3D& p_Transform);
			void submit(Particle_Emitter& p_Particle_Emitter);
			void submit(EnvData& p_EnvironmentData);
			void submit(CubeMap& p_IrradianceMap, CubeMap& p_PrefilteredMap, Image& p_BRDFLut);
			void submit(Skybox& p_Skybox);
			void submit(Skybox& p_Skybox, AtmosphericScatteringInfo& p_Info);
			void blitImage(Image& p_Image);

			void endRenderPass();

			void render();

			Vector2i getRenderResolution() { return m_Viewport.m_Size * m_RenderResolutionScale; }

			Camera* getCurrentCameraPtr() { return m_Camera; }

			Matrix4f& getViewProjection() { return m_ViewProjection; }

		protected:
		};
	}
}