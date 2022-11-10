#pragma once

#define NOMINMAX

#include<iostream>
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include "../../../lib/imgui/imgui.h"
#include "../../../lib/imgui/examples/imgui_impl_glfw.h"

#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderFramebuffer.h"
#include "../Shader/VulkanShader.h"
#include "../Buffers/VulkanCommandBuffer.h"
#include "../Core/VulkanWindow.h"
#include "../VkImGui.h"

#include "../../../Exzec-3D/Mesh/Mesh.h"
#include "../../../Exzec-3D/SkyBox.h"
#include "../../../Exzec-2D/Circle.h"
#include "../../../Exzec-Core/Utils/Logger.h"
#include "../../../Exzec-Core/Math/Camera.h"
#include "../../../Exzec-Core/Math/Quaternion.h"
#include "../../../Exzec-Core/Utils/ThreadPool.h"
#include "VulkanRenderInfo.h"

namespace exz
{
	namespace vk
	{
		class Renderer
		{
			public:
				Renderer();
				Renderer(Window* p_Window);
				Renderer(Window& p_Window);
				Renderer(Window& p_Window, Vector2i p_RenderResolution);
				~Renderer();

				CubeMap generateAtmosphericScatteringCubeMap(Vector2i p_Size);
				CubeMap getCubeMapFromHDR(Image& p_HDRImage, Vector2i p_Size);
				CubeMap generateIrradianceMap(CubeMap& p_CubeMapImage, Vector2i p_Size);
				CubeMap generatePreFilteredMaps(CubeMap& p_CubeMapImage, Vector2i p_InitialSize);
				Image	generateBRDFLut(Vector2f p_Size);

				Image& getFramebufferImage() { return m_MainImage.getImage(); }
				Image& getShadowMapImage() { return m_ShadowMapRenderFramebuffer.m_ImageAttachment.getImage(); }
				Image& getBloomMapImage() { return m_GaussBlurFramebuffers[1].m_ImageAttachment.getImage(); }

				void refreshRenderResolution(const Vector2i& p_Size);

				void setAspectRatio(Vector2f p_Size);
				void setCamera(Camera* p_Camera);
				void setCamera(Camera& p_Camera);
				void beginScene();
				void beginScene(CubeMap& p_IrradianceMap, CubeMap& p_PreFilteredMap, Image& p_BRDFLut);

				void submit(MeshData& p_MeshData, Matrix4f& p_ModelMatrix, RenderInfo& p_RenderInfo = defaultRenderInfo);
				void submit(MeshData& p_MeshData, const uint32_t p_MaterialID, Matrix4f& p_ModelMatrix, RenderInfo& p_RenderInfo = defaultRenderInfo);

				void submit(Skybox& p_SkyBox, bool renderAtmosphere = false);
				void submit(Skybox* p_SkyBox, bool renderAtmosphere = false);

				void submit(TextBox* p_TextBox);
				void submit(TextBox* p_TextBox, Scissor p_Scissor);
				void submit(TextBox* p_TextBox, Viewport p_Scissor);
				void submit(TextBox& p_TextBox);

				void submit(Rectangle* p_Rectangle);
				void submit(Rectangle& p_Rectangle);
				
				void submit(Circle* p_Circle);
				void submit(Circle& p_Circle);

				void endScene();

				void waitForFences();
				void destroy();

				void setBlurScale() {
					blurScale[0] = { BlurScale / m_GaussBlurFramebuffers[0].m_Info.p_Size.x, 0 };
					blurScale[1] = { 0, BlurScale / m_GaussBlurFramebuffers[1].m_Info.p_Size.y };
				}
				void setRenderResolution(Vector2i p_Resolution) {
					m_RenderResolution = p_Resolution;
				}

				void updateImGuiDrawData() {
					imgui.updateBuffers();
					imgui.genCmdBuffers();
				}

			public:
				HDRFXAAInfo hdrInfo;
				AtmosphericScatteringInfo atmosphericScatteringInfo;
				float lumaThreshold = 2.532f;
				float BlurScale = 3.0f;
				Vector2i m_ViewportPos = { 0, 0 };
				EnvData m_EnvData;

			protected:
			private:
				Pipeline m_SkyBoxPipeline;
				Pipeline m_AtmosphericScatteringPipeline;
				Pipeline m_NT3DPipeline;
				Pipeline m_3DDTPipeline;
				Pipeline m_3DPipeline;
				Pipeline m_3DSPMPipeline;
				Pipeline m_3DPBRPipeline;
				Pipeline m_2DCirclePipeline;
				Pipeline m_2DShapePipeline;
				Pipeline m_TexturedQuadPipeline;
				Pipeline m_2DTextBoxPipeline;

				Shader m_SkyBoxShader;
				Shader m_AtmosphericScatteringShader;
				Shader m_NT3DShader;
				Shader m_3DDTShader;
				Shader m_2DCircleShader;
				Shader m_2DShapeShader;
				Shader m_TexturedQuadShader;
				Shader m_2DTextBoxShader;
				Shader m_ShadowDepthMapShader;
				Shader m_GaussanBlurShader;

				UniformManager m_AtmosphericScatteringUM;

			private:
				ImGuiContext imgui;

				std::vector<CommandBuffer> m_TxtCommandBuffers;
				int m_TxtCommandBufferCount = 0;
				Matrix4f txtProj;

				// For Hdr and Bloom rendering
				RenderFramebufferInfo colorRenderFramebufferInfo; // Uses VK_FORMAT_R32G32B32A32_SFLOAT
				RenderFramebuffer colorRenderFramebuffer; // Uses VK_FORMAT_R32G32B32A32_SFLOAT

				UniformManager m_TexturedQaudUniformManager;

				Vector2f		blurScale[2];
				RenderFramebufferInfo m_GaussBlurFramebufferInfos[2];
				RenderFramebuffer m_GaussBlurFramebuffers[2];
				UniformManager	m_GaussBlurUniformMngr[2];

				std::vector<std::pair<Pipeline, ShaderInfo>> pipelines;
				std::vector<Shader> shaders;

				// Shadow mapping
				std::vector<UniformManager>		m_SUBM;
				Matrix4f						m_SMMVP;
				Vector2i						m_ShadowMapSize;
				RenderFramebufferInfo			m_ShadowMapRenderFramebufferInfo;
				RenderFramebuffer				m_ShadowMapRenderFramebuffer;
				float							depthBiasSlope = 1.75f;
				float							depthBiasConstant = 1.25f;
				Vector3f						m_LightDir = Vector3f(0.33, 1, 0.33);
				Vector3f						m_OldLightDir = Vector3f(0.33, 1, 0.33);

				// Current camera
				Camera* m_Camera;
				Matrix4f view;
				Matrix4f projection;
				Matrix4f mat;
				Matrix4f mvp;
				Matrix4f recMvp;

				Image* brdf;
				CubeMap* irradiance;
				CubeMap* preFiltered;

				vk::VertexArray m_RecVao;
				vk::IndexBuffer m_RecIndices;
				vk::VertexBuffer m_RecVertices;
				std::vector<Vector2f> m_RecRawVertices;
				std::vector<unsigned int> m_RecRawIndices;

			private:
				uint32_t m_CurrentShadowMapCmdBuffer = 0;
				uint32_t m_CurrentColorCmdBuffer = 0;
				std::vector<CommandBuffer> m_CommandBuffers;
				//std::vector<CommandBuffer> m_UnrecordedCommandBuffers;
				std::vector<CommandBuffer> m_ShadowMapCommandBuffers;
				std::vector<CommandBuffer> m_ColorCommandBuffers;

			private:

				void	createGaussBlurFramebuffer();

				void	initPipelines(MeshData& p_MeshData, RenderInfo& p_RenderInfo);
				void	renderTexturedQuadHdr();
				void	recordMeshCmdBuffer(MeshData& p_MeshData, Matrix4f& p_ModelMatrix);
				void	recordMeshCmdBuffer(MeshData& p_MeshData, const uint32_t p_MaterialID, Matrix4f& p_ModelMatrix);

				void	initShaders();

			private:

				static RenderInfo defaultRenderInfo;

				bool renderSame = true;

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

				unsigned int m_CurrentCmdBuffer;
				std::vector<CommandBuffer> m_PrimaryCommandBuffers;

				RenderPass m_DefaultRenderPass;
				VkCommandBufferInheritanceInfo m_DefaultInheritanceInfo;

				Utils::ThreadPool m_ThreadPool;

				Context* m_ContextPtr;
				Window* m_CurrentWindow;
				Vector2i m_CurrentWindowSize;

				Vector2f p_ProjectionSize;
				Vector2i m_RenderResolution;

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
				void	recordPrimaryCmdBuffer(VkFramebuffer& p_FrameBuffer, const int p_Index);

				void	createRenderPass();
				void	createDepthResources();

				void	cleanUpSwapChain();
				void	recreateSwapChain();
				void	createSyncObjects();
		};
	}
}