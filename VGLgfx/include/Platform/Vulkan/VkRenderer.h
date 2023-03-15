#pragma once

#include "VkWindow.h"
#include "VkFramebufferAttachment.h"
#include "VkImGui.h"
#include "VkCommandBuffer.h"

#include "../../VGL-3D/Mesh/Mesh.h"
#include "../../VGL-3D/SkyBox.h"
#include "../../Math/Camera.h"

namespace vgl
{
	class Application;

	namespace vk
	{
		#define MAX_FRAMES_IN_FLIGHT 2
		#define GET_CURRENT_IMAGE_INDEX(imgidx) (imgidx + 1) % MAX_FRAMES_IN_FLIGHT;

		// Directional shadow map
		struct DShadowMap
		{
			DShadowMap();
			~DShadowMap() {}

			void create(Vector2i p_Resolution, ImageFormat p_ImageFormat);

			Matrix4f m_View;
			Matrix4f m_Projection;
			Vector2i m_Resolution;

			float m_DepthBiasConstant = 1.25f;
			float m_DepthBiasSlope = 1.75f;

			void destroy();
			
			FramebufferAttachment m_Attachment;

		private:
			friend class Renderer;

			Context* m_ContextPtr;

			uint32_t m_CommandBufferIdx[MAX_FRAMES_IN_FLIGHT] = { 0,0 };
			std::vector<std::vector<CommandBuffer>> m_CommandBuffers; // Per swapchain image
		};
		
		// Point/Omni directional shadow map
		struct PShadowMap
		{
			PShadowMap();
			~PShadowMap() {}

			void create(Vector2i p_Resolution, ImageFormat p_ImageFormat);

			Vector2i m_Resolution;
			Vector3f* m_Position = nullptr;

			float m_DepthBiasConstant = 1.25f;
			float m_DepthBiasSlope = 1.75f;

			void destroy();

			FramebufferAttachment m_Attachment;
		private:
			Matrix4f m_View[6];
			Matrix4f m_Projection;

			Vector3f m_PrevPosition;

		private:
			friend class Renderer;

			Context* m_ContextPtr;

			Descriptor m_TransformDescriptor;

			uint32_t m_CommandBufferIdx[MAX_FRAMES_IN_FLIGHT] = { 0, 0 };
			std::vector<std::vector<CommandBuffer>> m_CommandBuffers; // Per swapchain image and per face
		};

		// Submits data to gpu for graphical rendering
		class Renderer
		{
			public:
				Renderer();
				Renderer(Window* p_Window);
				Renderer(Window& p_Window);
				Renderer(Window& p_Window, float p_RenderResolutionScale);

				void begin();
				void end();
				void beginRenderPass(RenderInfo& p_RenderInfo);
				// If not called, will use default frambuffer/swapchain
				void beginRenderPass(RenderInfo& p_RenderInfo, FramebufferAttachment& p_FramebufferAttachment);
				// Renders into the shadow map
				void beginRenderPass(RenderInfo& p_RenderInfo, DShadowMap& p_ShadowMap);
				void beginRenderPass(RenderInfo& p_RenderInfo, PShadowMap& p_ShadowMap);

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
				void submit(
					ComputePipeline& p_ComputePipeline,
					Descriptor& p_Descriptor,
					const Vector3i p_WorkgroupCount,
					PipelineStage p_SrcStage = PipelineStage::Bottom_Of_Pipe,
					PipelineStage p_DstStage = PipelineStage::Top_Of_Pipe
				);
				// Submits to push constant
				void submit(void* p_Data, const uint32_t p_Size);
				void submit(Image& p_Image, const uint32_t p_Binding);
				// Size() == swapchain image count
				void submit(std::vector<ImageAttachment>& p_Images, const uint32_t p_Binding);
				// Render only single material
				void submit(MeshData& p_MeshData, const uint32_t p_MTLIndex, Transform3D& p_Transform);
				void submit(MeshData& p_MeshData, Transform3D& p_Transform);
				/*void submit(Particle_Emitter& p_Particle_Emitter);
				//void submit(EnvData& p_EnvironmentData);
				//void submit(ImageCube& p_IrradianceMap, CubeMap& p_PrefilteredMap, Image& p_BRDFLut);
				/void submit(Skybox& p_Skybox);*/
				void submit(Skybox& p_Skybox, AtmosphericScatteringInfo& p_Info);
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

				DShadowMap* m_DShadowMapPtr = nullptr;
				PShadowMap* m_CurrentPShadowMapPtr = nullptr;

				AtmosphericScatteringInfo* m_AtmosphericScatteringInfoPtr = nullptr;

			private:
				void recordPrimaryCommandBuffers(uint32_t p_ImageIndex);
				// Directional Shadow map
				void recordMeshDataSMD(MeshData& p_MeshData, Transform3D& p_Transform);
				// Omni/Point Shadow map
				void recordMeshDataSMP(MeshData& p_MeshData, Transform3D& p_Transform);
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
				void setCmdViewport(CommandBuffer& p_CommandBuffer, Vector2i p_Size);

				// p_Ptr is arbitrary data set beforehand 
				void blitImageFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);
				// Render into current framebuffer
				void recordPCmdFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);
				// Render into directional/spot shadow map framebuffer
				void recordPCmdSMAPFunDS(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);
				void recordPCmdSMAPFunP(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);

				struct ComputeDispatchInfo
				{
					ComputePipeline* p_ComputePipeline;
					Descriptor* p_Descriptor;
					Vector3i p_WorkgroupCount;

					PipelineStage p_SrcStage = PipelineStage::Fragment_Shader;
					PipelineStage p_DstStage = PipelineStage::Compute_Shader;
				};

				void recordPCmdComputeFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);

			private:
				friend class vgl::Application;

				ImGuiContext m_ImGuiContext;
				GraphicsContext* m_GraphicsContextPtr = nullptr;
			private:
				// Main commandbuffers used for rendering, these are used when calling the submit() functions
				std::vector<std::vector<CommandBuffer>> m_CommandBuffers;
				uint32_t m_CommandBufferIdx[MAX_FRAMES_IN_FLIGHT] = {0, 0};

				std::vector<VkCommandBufferInheritanceInfo> m_InheritanceInfo;

				RenderInfo m_RenderInfo;
				Shader* m_ShaderPtr = nullptr;
				//EnvData* m_EnvDataPtr = nullptr;
				//AtmosphericScatteringInfo* m_AtmosphericScatteringInfoPtr = nullptr;

				FramebufferAttachment* m_FramebufferAttachmentPtr = nullptr;

				//D_ShadowMap* m_DShadowMapPtr = nullptr;

				DescriptorSetInfo m_DescriptorSetInfo;
				Descriptor m_Descriptor;

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
				std::vector<PCMDP> m_RecordPrimaryComputeCmdBufferFunPtrs;
			
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

		/*struct ComputeCommands
		{
			ComputeCommands() : m_ContextPtr(&ContextSingleton::getInstance()) {}

			void submit(
				Image& p_Image,
				PipelineStage p_SrcStage,
				PipelineStage p_DstStage
			){
				static ImageMemoryBarrierInfo info = { &p_Image, p_SrcStage, p_DstStage };
				m_ComputeCommandsQueue.emplace_back(&info, &ComputeCommands::CCmdImageMemoryBarrierFun);
			}

			void CCmdImageMemoryBarrierFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex)
			{
				ImageMemoryBarrierInfo& info = *(ImageMemoryBarrierInfo*)p_Ptr;
				if (info.p_Image == nullptr) return;
				
				VkImageMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.image = info.p_Image->m_VkImageHandle;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;
				barrier.subresourceRange.levelCount = info.p_Image->m_MipLevels;
				barrier.subresourceRange.baseMipLevel = 0;

				vkCmdPipelineBarrier(*m_CommandBufferPtr, (VkPipelineStageFlags)info.p_SrcStage, (VkPipelineStageFlags)info.p_DstStage,
					0, 0, nullptr, 0, nullptr, 1,
					&barrier
				);
			}

			void bufferMemoryBarrier(VkBuffer& p_Buffer, PipelineStage p_SrcStage, PipelineStage p_DstStage)
			{
				VkBufferMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				barrier.buffer = p_Buffer;
				//barrier.dstAccessMask = VK_ACCESS_STAG

			}
			void executionBarrier(PipelineStage p_SrcStage, PipelineStage p_DstStage)
			{
				vkCmdPipelineBarrier(*m_CommandBufferPtr, (VkPipelineStageFlags)p_SrcStage, (VkPipelineStageFlags)p_DstStage, VK_DEPENDENCY_BY_REGION_BIT, 1, )
			}
			void dispatch()
			{

			}

		private:
			Context* m_ContextPtr = nullptr;

			VkCommandBuffer* m_CommandBufferPtr;

			std::vector<VkImageMemoryBarrier> m_ImageMemoryBarriers;
			std::vector<VkBufferMemoryBarrier> m_BufferMemoryBarriers;

			struct ImageMemoryBarrierInfo
			{
				Image* p_Image = nullptr;
				PipelineStage p_SrcStage;
				PipelineStage p_DstStage;
			};

			// Data passed from command submission
			struct CCMDFP {
				CCMDFP(void* p_Ptr, void(ComputeCommands::* p_CCMDFun)(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex))
					: ptr(p_Ptr), ccmdFun(p_CCMDFun) {}

				void* ptr; // Data
				void(ComputeCommands::* ccmdFun)(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex);
			};

			std::vector<CCMDFP> m_ComputeCommandsQueue;
		};*/
	}
}