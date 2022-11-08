#pragma once

#include "VkWindow.h"
#include "VkFramebufferAttachment.h"
#include "VkImGui.h"
#include "VkCommandBuffer.h"

#include "../../VGL-3D/Mesh/Mesh.h"
#include "../../Math/Camera.h"

namespace vgl
{
	class Application;

	namespace vk
	{
		#define MAX_FRAMES_IN_FLIGHT 2
		#define GET_CURRENT_IMAGE_INDEX(imgidx) (imgidx + 1) % 2;

		struct D_ShadowMap
		{
			D_ShadowMap();
			~D_ShadowMap() {}

			FramebufferAttachment m_Attachment;

			Matrix4f m_View;
			Matrix4f m_Projection;
			Vector2i m_Resolution;
			Vector3f m_Direction;

			float m_DepthBiasConstant = 1.00005;
			float m_DepthBiasSlope = 1.0005;

			void destroy() {
				m_Attachment.destroy();

				for (auto& dsc : m_Descriptors)
					if (dsc.isValid())
						dsc.destroy();
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
			std::vector<DescriptorSetInfo> m_DescriptorAlbedoInfo;
			std::vector<Image*> m_AlbedoPtrs;
			std::vector<std::vector<CommandBuffer>> m_CommandBuffers; // Per swapchain image
		};

		// Submits data to gpu for graphical rendering
		class Renderer
		{
			public:
				Renderer();
				Renderer(Window* p_Window);
				Renderer(Window& p_Window);
				Renderer(Window& p_Window, float p_RenderResolutionScale);

				void begin(){
					m_ContextPtr->m_ImageIndex = m_ImageIndex;
					m_ContextPtr->m_CurrentFrame = m_CurrentFrame;
				}
				void end() {
					GPUSubmit();
				}
				void beginRenderPass(RenderInfo& p_RenderInfo);
				// If not called, will use default frambuffer/swapchain
				void beginRenderPass(RenderInfo& p_RenderInfo, FramebufferAttachment& p_FramebufferAttachment);
				// Renders into the shadow map
				void beginRenderPass(RenderInfo& p_RenderInfo, D_ShadowMap& p_ShadowMap);

				void submit(Camera& p_Camera);

				// If not called, will use default viewport data from current framebuffer
				void submit(Viewport& p_Viewport); 

				/* 3D render pass submissions */
				//void submit(D_ShadowMap& p_ShadowMap);
				//void submit(D_Light& p_LightSrc);
				//void submit(S_Light& p_LightSrc);
				//void submit(P_Light& p_LightSrc);
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
				//void submit(Particle_Emitter& p_Particle_Emitter);
				//void submit(EnvData& p_EnvironmentData);
				//void submit(ImageCube& p_IrradianceMap, CubeMap& p_PrefilteredMap, Image& p_BRDFLut);
				//void submit(Skybox& p_Skybox);
				//void submit(Skybox& p_Skybox, AtmosphericScatteringInfo& p_Info);
				void blitImage(Image& p_Image);

				void endRenderPass();

				void render();
				//Vector2i getRenderResolution() { return m_Viewport.m_Size; }

				const int32_t getImageIndex() { return m_ImageIndex; }

				Camera* getCurrentCameraPtr() { return m_Camera; }

				Matrix4f& getViewProjection(){ return m_ViewProjection; }
				Matrix4f& getMVP(){ return m_MVP; }
				Vector2f& getOrthoProjection(){ return m_OrthoProjectionSize; }
				
				Viewport& getViewport(){ return m_Viewport; }
				Scissor& getScissor(){ return m_Scissor; }

			private:
				Camera* m_Camera;
				Matrix4f m_ViewProjection; // View * Projection
				Matrix4f m_MVP; // Model * View * Projection

				Matrix4f m_FontProjection;
				Vector2f m_OrthoProjectionSize;

				Viewport m_Viewport;
				Scissor m_Scissor;

				static RenderInfo m_DefaultRenderInfo;

				D_ShadowMap* m_DShadowMapPtr = nullptr;

			private:
				void recordPrimaryCommandBuffers(uint32_t p_ImageIndex);
				void shadowMapRecordMeshData(MeshData& p_MeshData, Transform3D& p_Transform);
				// Record a commandbuffer for draw commands with a mesh
				void recordMeshData(MeshData& p_MeshData, Transform3D& p_Transform);
				void recordMeshDataSkybox(MeshData& p_MeshData, Transform3D& p_Transform);

				void recordMeshCmdBuffer(
					MeshData& p_MeshData,
					FramebufferAttachment& p_FramebufferAttachment,
					CommandBuffer& p_CommandBuffer,
					const uint32_t p_ImageIndex
				);
				void createPipelines(MeshData& p_MeshData, RenderInfo& p_RenderInfo, RenderPass& p_RenderPass);

				void setCmdDefaultViewport(CommandBuffer& p_CommandBuffer);

				// p_Ptr is arbitrary data set beforehand 
				void blitImageFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);
				void primaryRecFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);
				void primaryShadowRecFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);

			private:
				friend class vgl::Application;

				ImGuiContext m_ImGuiContext;
				GraphicsContext* m_GraphicsContextPtr = nullptr;
			private:
				// Main commandbuffers used for rendering, these are used when calling the submit() functions
				std::vector<std::vector<CommandBuffer>> m_CommandBuffers;
				uint32_t m_CommandBufferIdx = 0;

				std::vector<VkCommandBufferInheritanceInfo> m_InheritanceInfo;

				RenderInfo m_RenderInfo;
				Shader* m_ShaderPtr = nullptr;
				//EnvData* m_EnvDataPtr = nullptr;
				//AtmosphericScatteringInfo* m_AtmosphericScatteringInfoPtr = nullptr;

				FramebufferAttachment* m_FramebufferAttachmentPtr = nullptr;

				//D_ShadowMap* m_DShadowMapPtr = nullptr;

				DescriptorSetInfo m_DescriptorSetInfo;
				DescriptorSetManager m_DescriptorSetManager;

				// Used for caching pipelines
				struct ShaderData
				{
					g_Pipeline p_Pipeline;
					Shader p_Shader;
					ShaderInfo p_Info;
				};

				std::vector<ShaderData> m_ShaderData; // TODO: Use std::map instead

			private:
				// Relevant Vulkan devices, data and utility functions
				Context* m_ContextPtr;

				// Data passed from render submissions (via function ptrs)
				struct PCMDP {
					PCMDP(void* p_Ptr, void(Renderer::* p_PCMDFun)(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex))
						: ptr(p_Ptr), pcmdRecFun(p_PCMDFun) {}
					void* ptr;
					void(Renderer::*pcmdRecFun)(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);
				};

				void (Renderer::*m_RecordMeshDataFun)(MeshData& p_MeshData, Transform3D& p_Transform);

				std::vector<PCMDP> m_RecordPrimaryCmdBuffersFunPtrs;
			
			// Internal vulkan functions for renderer 
			private:
				void	GPUSubmit(); // Submits commandbuffers to gpu queue

				void	destroySwapChain();
				void	recreateSwapChain(); // Recreate the swapchain, e.g. when window framebuffer is resized
				void	createSyncObjects();

				void	createDefaultRenderPass(); // Default renderpass, mainly used when presenting to the screen buffer
				void	createPostSwapchainRenderPass(); // Used when presenting to the screen buffer after swapchain has already been rendered to
				void	createDepthImageAttachment();

				void 	waitForFences(); // Stall to wait for the fences to sync with the gpu
				
			private:
				Window* m_WindowPtr;

				ImageAttachment m_DepthImageAttachment;

				RenderPass m_PostSwapchainRenderPass;
				RenderPass m_DefaultRenderPass;
				VkCommandBufferInheritanceInfo m_DefaultInheritanceInfo;

				std::vector<CommandBuffer> m_PrimaryCommandBuffers;

				VkResult m_CreateFunResult; // Result of vulkan functions, e.g. vkCreate...() functions 
				VkSubmitInfo m_SubmitInfo = {};
				VkPresentInfoKHR m_PresentInfo = {};
				VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

				// Objects for syncing the gpu/cpu submissions
				std::vector<VkSemaphore>	m_ImageAvailableSemaphore;
				std::vector<VkSemaphore>	m_RenderFinishedSemaphore;
				std::vector<VkFence>		m_InFlightFences;
				std::vector<VkFence>		m_ImagesInFlight;

				uint32_t	m_CurrentFrame = 0; // Current frame/image that is written by the gpu
				uint32_t 	m_ImageIndex = 0; // Available frame/image that is not currently written by the gpu
		};
	}
}