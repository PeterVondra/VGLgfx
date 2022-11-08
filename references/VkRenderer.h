#pragma once

#include "VkBaseRenderer.h"
#include "VkGraphics_Internal.h"
#include "../../VGL-3D/Particles.h"

namespace vgl
{
	namespace vk
	{
		struct D_ShadowMap
		{
			D_ShadowMap() : m_ContextPtr(&ContextSingleton::getInstance()), m_CommandBuffers(m_ContextPtr->m_SwapchainImageCount), m_CommandBufferIdx(0) {
				for (uint32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
					for (uint32_t j = 0; j < 500; j++){
						m_CommandBuffers[i].emplace_back(i, Level::Secondary);
						m_Descriptors.emplace_back();
						m_DescriptorInfo.emplace_back(DescriptorSetInfo({ UniformBuffer(sizeof(Matrix4f), 0) }));
					}
				}

				m_DescriptorAlbedoInfo.resize(1000);
			}
			~D_ShadowMap() {}

			FramebufferAttachment m_Attachment;

			Matrix4f m_View;
			Matrix4f m_Projection;
			Vector2i m_Resolution;
			Vector3f m_Direction;

			float m_DepthBiasConstant;
			float m_DepthBiasSlope;

			void destroy() {
				m_Attachment.destroy();

				for (auto& dsc : m_Descriptors)
					if (dsc.isComplete())
						dsc.destroy();
				m_DescriptorInfo.clear();
				m_DescriptorAlbedoInfo.clear();
				for (auto& cmds : m_CommandBuffers)
					for (auto& cmd : cmds)
						cmd.destroy();
			}

			private:
				friend class Renderer;

				Context* m_ContextPtr;

				uint32_t m_CommandBufferIdx;
				std::vector<DescriptorSetManager> m_Descriptors;
				std::vector<DescriptorSetInfo> m_DescriptorInfo;
				std::vector<DescriptorSetInfo> m_DescriptorAlbedoInfo;
				std::vector<Image*> m_AlbedoPtrs;
				std::vector<std::vector<CommandBuffer>> m_CommandBuffers; // Per swapchain image
		};	
		struct P_ShadowMap
		{
			P_ShadowMap() : m_ContextPtr(&ContextSingleton::getInstance()), m_CommandBuffers(m_ContextPtr->m_SwapchainImageCount), m_CommandBufferIdx(0) {
				for (uint32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
					for (uint32_t j = 0; j < 500; j++){
						m_CommandBuffers[i].emplace_back(i, Level::Secondary);
						m_Descriptors.emplace_back();
						m_DescriptorInfo.emplace_back(DescriptorSetInfo({ UniformBuffer(sizeof(Matrix4f), 0) }));
						m_DescriptorInfo.emplace_back(DescriptorSetInfo({ UniformBuffer(sizeof(Matrix4f), 0) }));
					}
				}
			}
			~P_ShadowMap() {}

			FramebufferAttachment m_Attachment;

			Matrix4f m_View;
			Matrix4f m_Projection;
			Vector2i m_Resolution;
			Vector3f m_Direction;

			float m_DepthBiasConstant;
			float m_DepthBiasSlope;

			void destroy() {
				m_Attachment.destroy();

				for (auto& dsc : m_Descriptors)
					if (dsc.isComplete())
						dsc.destroy();
				m_DescriptorInfo.clear();
				for (auto& cmds : m_CommandBuffers)
					for (auto& cmd : cmds)
						cmd.destroy();
			}

			private:
				friend class Renderer;

				Context* m_ContextPtr;

				uint32_t m_CommandBufferIdx;
				std::vector<DescriptorSetManager> m_Descriptors;
				std::vector<DescriptorSetInfo> m_DescriptorInfo;
				std::vector<std::vector<CommandBuffer>> m_CommandBuffers; // Per swapchain image
		};	
		struct S_ShadowMap
		{
			S_ShadowMap() : m_ContextPtr(&ContextSingleton::getInstance()), m_CommandBuffers(m_ContextPtr->m_SwapchainImageCount), m_CommandBufferIdx(0) {
				for (uint32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
					for (uint32_t j = 0; j < 500; j++){
						m_CommandBuffers[i].emplace_back(i, Level::Secondary);
						m_Descriptors.emplace_back();
						m_DescriptorInfo.emplace_back(DescriptorSetInfo({ UniformBuffer(sizeof(Matrix4f), 0) }));
					}
				}
			}
			~S_ShadowMap() {}

			FramebufferAttachment m_Attachment;

			Matrix4f m_View;
			Matrix4f m_Projection;
			Vector2i m_Resolution;
			Vector3f m_Direction;
			Vector3f m_Position;

			float m_DepthBiasConstant;
			float m_DepthBiasSlope;

			private:
				friend class Renderer;

				Context* m_ContextPtr;

				uint32_t m_CommandBufferIdx;
				std::vector<DescriptorSetManager> m_Descriptors;
				std::vector<DescriptorSetInfo> m_DescriptorInfo;
				std::vector<std::vector<CommandBuffer>> m_CommandBuffers; // Per swapchain image
		};

		// DESCRIPTION
		class Renderer : public BaseRenderer
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
				void beginRenderPass(RenderInfo& p_RenderInfo, D_ShadowMap& p_ShadowMap);

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

				const int32_t getImageIndex() { return m_ImageIndex; }

				Camera* getCurrentCameraPtr() { return m_Camera; }

				Matrix4f& getViewProjection(){ return m_ViewProjection; }

			protected:
			private:
				void f_PrimaryCmdRec(CommandBuffer& p_PrimaryCommandBuffer, uint32_t p_ImageIndex)override;
				void shadowMapRecordMeshData(MeshData& p_MeshData, Transform3D& p_Transform);
				void recordMeshData(MeshData& p_MeshData, Transform3D& p_Transform);
				void recordMeshDataSkybox(MeshData& p_MeshData, Transform3D& p_Transform);

				void blitImageFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);
				void primaryRecFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);
				void primaryShadowRecFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);

			private:
				std::vector<std::vector<CommandBuffer>> m_CommandBuffers;
				uint32_t m_CommandBufferIdx = 0;

				RenderInfo m_RenderInfo;
				Shader* m_ShaderPtr = nullptr;
				EnvData* m_EnvDataPtr = nullptr;
				AtmosphericScatteringInfo* m_AtmosphericScatteringInfoPtr = nullptr;

				FramebufferAttachment* m_FramebufferAttachmentPtr = nullptr;

				D_ShadowMap* m_DShadowMapPtr = nullptr;

				DescriptorSetInfo m_DescriptorSetInfo;
				DescriptorSetManager m_DescriptorSetManager;

			private:
				struct PCMDP {
					PCMDP(void* p_Ptr, void(Renderer::* p_PCMDFun)(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex))
						: ptr(p_Ptr), pcmdRecFun(p_PCMDFun) {}
					void* ptr;
					void(Renderer::*pcmdRecFun)(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);
				};

				void (Renderer::*m_RecordMeshDataFun)(MeshData& p_MeshData, Transform3D& p_Transform);

				std::vector<PCMDP> m_RecordPrimaryCmdBuffersFunPtrs;

		};
	}
}