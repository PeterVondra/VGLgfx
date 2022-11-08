#pragma once

#include "VkContext.h"
#include "VkWindow.h"
#include "VkFramebuffer.h"
#include "VkFramebufferAttachment.h"
#include "../../VGL-3D/Mesh/Mesh.h"
#include "../../Math/Camera.h"
#include "VkImage.h"
#include "VkDescriptorSetManager.h"
#include "VkRenderInfo.h"
#include "../../Utils/Cache.h"
#include "../../Utils/ThreadPool.h"

namespace vgl
{
	namespace vk
	{
		struct UberShaderData
		{
			Pipeline pipeline;
			Shader shader;
			ShaderInfo info;
		};
		struct GaussBlurFramebufferData
		{
			Vector2f blurScale[2];
			FramebufferAttachmentInfo gaussBlurFramebufferInfos[2];
			FramebufferAttachment gaussBlurFramebuffers[2]; // Index = 1 = rendered image
			DescriptorSetManager gaussBlurUniformMngr[2];

			void updateData(const Vector2i& p_Size, float p_BlurScale)
			{
				blurScale[0] = { p_BlurScale / p_Size.x, 0 };
				blurScale[1] = { 0, p_BlurScale / p_Size.y };
			}
		};

		struct MeshRenderInfo
		{
			Pipeline* pipeline;
			FramebufferAttachment framebuffer;
		};

		class BaseRenderer
		{
		public:
			BaseRenderer();
			BaseRenderer(Window* p_Window);
			BaseRenderer(Window& p_Window);
			BaseRenderer(Window& p_Window, Vector2i p_RenderResolution);
			~BaseRenderer();

			CubeMap generateAtmosphericScatteringCubeMap(Vector2i p_Size, AtmosphericScatteringInfo& p_Info = atmosphericScatteringInfo);
			CubeMap getCubeMapFromHDR(Image& p_HDRImage, Vector2i p_Size);
			CubeMap generateIrradianceMap(CubeMap& p_CubeMapImage, Vector2i p_Size);
			CubeMap generatePreFilteredMaps(CubeMap& p_CubeMapImage, Vector2i p_InitialSize);
			Image	generateBRDFLut(Vector2f p_Size);

			void refreshRenderResolution(const Vector2i& p_Size);

			void setAspectRatio(Vector2f p_Size);
			void setCamera(Camera* p_Camera);
			void setCamera(Camera& p_Camera);

			Image& getMainImage() { return m_MainImage.getImage(); };

			void beginScene();
			void endScene();

			void waitForFences();
			void destroy();

			void updateImGuiDrawData();
		public:
			/// General config values
			HDRFXAAInfo hdrInfo;
			static AtmosphericScatteringInfo atmosphericScatteringInfo;
			float lumaThreshold = 2.532f;
			float BlurScale = 3.0f;
			Vector2i m_ViewportPos = { 0, 0 };
			EnvData m_EnvData;

			float depthBiasSlope = 1.75f;
			float depthBiasConstant = 1.25f;

		protected:
			Camera* m_Camera;
			Matrix4f m_ViewProjection;
			ImGuiContext imgui;
			void initDefaultShaders();

			Matrix4f mvp;

			static RenderInfo defaultRenderInfo;

			// For the main image that will present the finsihed rendered image
			DescriptorSetManager m_MainImageUniformManager;

			std::vector<UberShaderData> shaderData;

			// Current camera
			Camera* m_Camera;

			vk::VertexArray m_RecVao;
			vk::IndexBuffer m_RecIndices;
			vk::VertexBuffer m_RecVertices;
			std::vector<Vector2f> m_RecRawVertices;
			std::vector<unsigned int> m_RecRawIndices;

			Vector2i m_RenderResolution;

		protected:
			Pipeline m_SkyBoxPipeline;
			Pipeline m_AtmosphericScatteringPipeline;
			Pipeline m_2DCirclePipeline;
			Pipeline m_2DShapePipeline;
			Pipeline m_TexturedQuadPipeline;
			Pipeline m_2DTextBoxPipeline;
			PipelineInfo m_ShadowDepthMapPipelineInfo;
			PipelineInfo m_GaussanBlurPipelineInfo;

			Shader m_SkyBoxShader;
			Shader m_AtmosphericScatteringShader;
			Shader m_2DCircleShader;
			Shader m_2DShapeShader;
			Shader m_TexturedQuadShader;
			Shader m_2DTextBoxShader;
			Shader m_ShadowDepthMapShader;
			Shader m_GaussanBlurShader;

			DescriptorSetManager m_AtmosphericScatteringUM;

		private:
			ImGuiContext imgui; // ImGui context for rendering

			Matrix4f txtProj;
			int m_TxtCommandBufferCount = 0;
			std::vector<CommandBuffer> m_TxtCommandBuffers; // Text will be rendered

		private:
			uint32_t m_CurrentMainCmdBufIdx = 0;
			std::vector<CommandBuffer> m_MainCommandBuffers; // General command buffers
			//std::vector<CommandBuffer> m_UnrecordedCommandBuffers;

		private:
			void	renderMainImage();
			void	initDefaultShaders();

		private:
			Vector3f m_ClearValue;
			VkClearValue m_clearColor;
			ImageAttachment m_DepthImageAttachment;

			VkResult result;
			VkSubmitInfo submitInfo = {};
			VkPresentInfoKHR presentInfo = {};
			VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

			std::vector<VkSemaphore>	imageAvailableSemaphore;
			std::vector<VkSemaphore>	renderFinishedSemaphore;
			std::vector<VkFence>		inFlightFences;
			std::vector<VkFence>		imagesInFlight;

			size_t		current_frame = 0;
			bool		framebufferResized = false;
			int			m_CurrentFrameBufferImage = 0;
			const int	MAX_FRAMES_IN_FLIGHT = 2;
			uint32_t	imageIndex = 0;

			bool m_RecordCmdBuffers = true;

			std::vector<CommandBuffer> m_PrimaryCommandBuffers;

			RenderPass m_DefaultRenderPass;
			VkCommandBufferInheritanceInfo m_DefaultInheritanceInfo;

			Utils::ThreadPool m_ThreadPool;

			Context* m_ContextPtr;
			Window* m_CurrentWindow;
			Vector2i m_CurrentWindowSize;

			Vector2f m_ProjectionSize;

			ImageAttachment m_MainImage;
			Framebuffer m_MainFramebuffer;
			VkCommandBufferInheritanceInfo m_MainInheritanceInfo;
			RenderPass m_MainRenderPass;

		private:

			void	createMainResources();
			void	createMainRenderPass();

			void	bind(Window* p_Window);
			void	submitCommands();
			void	submitCommandBuffer(CommandBuffer& p_CmdBuffer);
			void	recordPrimaryCmdBuffer(VkFramebuffer& p_FrameBuffer, const uint32_t p_Index);

			void	createRenderPass();
			void	createDepthResources();

			void	cleanUpSwapChain();
			void	recreateSwapChain();
			void	createSyncObjects();
		};
	}
}