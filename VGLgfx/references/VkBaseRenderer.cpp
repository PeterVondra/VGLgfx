#include "VkBaseRenderer.h"

namespace vgl
{
	namespace vk
	{
		BaseRenderer::BaseRenderer()
			: m_DefaultRenderPass(RenderPassType::Graphics),
			m_CurrentWindowSize({ 0, 0 })
		{
			m_ContextPtr = &ContextSingleton::getInstance();
			m_CurrentWindow = nullptr;

			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.signalSemaphoreCount = 1;

			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.swapchainCount = 1;
			presentInfo.pResults = nullptr;

			initDefaultShaders();
		}
		BaseRenderer::BaseRenderer(Window* p_Window)
			: m_DefaultRenderPass(RenderPassType::Graphics),
			m_CurrentWindowSize(p_Window->getWindowSize())
		{
			m_ContextPtr = &ContextSingleton::getInstance();
			bind(p_Window);

			m_CurrentWindowSize = p_Window->getWindowSize();

			m_CurrentWindow = p_Window;
			m_Camera = nullptr;

			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.signalSemaphoreCount = 1;

			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.swapchainCount = 1;
			presentInfo.pResults = nullptr;

			initDefaultShaders();
		}
		BaseRenderer::BaseRenderer(Window& p_Window)
			: m_DefaultRenderPass(RenderPassType::Graphics),
			m_CurrentWindowSize(p_Window.getWindowSize())//, m_ThreadPool(std::thread::hardware_concurrency())
			, imgui(&p_Window, p_Window.getWindowSize())
		{
			m_ContextPtr = &ContextSingleton::getInstance();
			bind(&p_Window);

			m_CurrentWindowSize = p_Window.getWindowSize();

			m_CurrentWindow = &p_Window;
			m_Camera = nullptr;

			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.signalSemaphoreCount = 1;

			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.swapchainCount = 1;
			presentInfo.pResults = nullptr;

			initDefaultShaders();
		}
		BaseRenderer::BaseRenderer(Window& p_Window, float p_RenderResolutionScale)
			: m_DefaultRenderPass(RenderPassType::Graphics), m_RenderResolutionScale(p_RenderResolutionScale),
			m_CurrentWindowSize(p_Window.getWindowSize())//, m_ThreadPool(std::thread::hardware_concurrency())
			, imgui(&p_Window, p_Window.getWindowSize()* m_RenderResolutionScale)
		{
			m_ContextPtr = &ContextSingleton::getInstance();

			m_Viewport.m_Size = p_Window.getWindowSize();

			bind(&p_Window);

			m_CurrentWindowSize = p_Window.getWindowSize();

			m_CurrentWindow = &p_Window;
			m_Camera = nullptr;

			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.signalSemaphoreCount = 1;

			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.swapchainCount = 1;
			presentInfo.pResults = nullptr;

			initDefaultShaders();
		}
		BaseRenderer::~BaseRenderer()
		{

		}

		void BaseRenderer::initDefaultShaders()
		{
			m_TextProjection = Matrix4f::orthoRH_ZO(0, m_CurrentWindow->getWindowSize().x, 0, m_CurrentWindow->getWindowSize().y, 0.0f, 1.0f);
		}

		void BaseRenderer::createPipelines(MeshData& p_MeshData, RenderInfo& p_RenderInfo, RenderPass& p_RenderPass)
		{
			PipelineInfo pipelineInfo;
			
			pipelineInfo.p_CullMode = p_RenderInfo.p_CullMode;
			pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
			pipelineInfo.p_PolygonMode = p_RenderInfo.p_PolygonMode;
			pipelineInfo.p_MSAASamples = 1;
			pipelineInfo.p_Viewport = Viewport(m_CurrentWindow->m_WindowSize, { 0, 0 });
			pipelineInfo.p_Scissor = Scissor(m_CurrentWindow->m_WindowSize, { 0, 0 });
			pipelineInfo.p_SampleRateShading = true;
			pipelineInfo.p_RenderPass = &p_RenderPass;
			pipelineInfo.p_AlphaBlending = false;// p_RenderInfo.p_AlphaBlending;

			pipelineInfo.p_AttributeDescription = p_MeshData.m_VertexArray.getAttributeDescriptions();
			pipelineInfo.p_BindingDescription = p_MeshData.m_VertexArray.getBindingDescription();

			for (int i = 0; i < p_MeshData.m_Materials.size(); i++) {
				bool c = false;

				for (int k = 0; k < shaderData.size(); k++) {
					if (p_MeshData.m_Materials[i].m_ShaderInfo == shaderData[k].info) {
						c = true;
						p_MeshData.m_Materials[i].m_PipelineIndex = k;
					}
				}

				if (c) continue;

				shaderData.emplace_back();

				Shader shader;
				shader.compile(ShaderPermutationsGenerator::generateShaderGBuffer(p_MeshData.m_Materials[i].m_ShaderInfo));
				shaderData[shaderData.size() - 1].shader = shader;

				pipelineInfo.p_Shader = &shaderData[shaderData.size() - 1].shader;

				pipelineInfo.p_DescriptorSetLayout = &p_MeshData.m_MtlUniforms[i].m_DescriptorSetLayout[0];

				shaderData[shaderData.size() - 1].pipeline.create(pipelineInfo);

				p_MeshData.m_Materials[i].m_PipelineIndex = shaderData.size() - 1;
			}
		}

		void BaseRenderer::recordMeshCmdBuffer(MeshData& p_MeshData, FramebufferAttachment& p_FramebufferAttachment, CommandBuffer& p_CommandBuffer, const uint16_t p_Index)
		{
			p_CommandBuffer.cmdBegin(p_FramebufferAttachment.m_InheritanceInfo[p_Index]);
			setCmdDefautlViewport(p_CommandBuffer);

			for (int i = 0; i < p_MeshData.m_Materials.size(); i++)
			{
				if (!p_MeshData.m_Materials[i].config.render)
					continue;
				p_CommandBuffer.cmdBindPipeline(shaderData[p_MeshData.m_Materials[i].m_PipelineIndex].pipeline);
				p_CommandBuffer.cmdBindDescriptorSets(p_MeshData.m_MtlUniforms[i]);
				p_CommandBuffer.cmdBindVertexArray(p_MeshData.m_VertexArray);
				p_CommandBuffer.cmdDrawIndexed(p_MeshData.m_MaterialIndices[i].first, p_MeshData.m_MaterialIndices[i].second);
			}

			p_CommandBuffer.cmdEnd();
		}

		void BaseRenderer::setCmdDefautlViewport(CommandBuffer& p_CommandBuffer)
		{
			p_CommandBuffer.cmdSetViewport(
				Viewport({
					int(m_Viewport.m_Size.x * m_RenderResolutionScale), int(-m_Viewport.m_Size.y * m_RenderResolutionScale) },
					Vector2i(0, int(m_Viewport.m_Size.y * m_RenderResolutionScale)) + m_Viewport.m_Position
				)
			);
			p_CommandBuffer.cmdSetScissor(
				Scissor({
					int(m_Viewport.m_Size.x * m_RenderResolutionScale),
					int(m_Viewport.m_Size.y * m_RenderResolutionScale) }, { 0, 0 }
				)
			);
		}

		void BaseRenderer::bind(Window* p_Window)
		{
			if (m_CurrentWindow != p_Window && p_Window != nullptr)
			{
				createSyncObjects();

				vkDeviceWaitIdle(m_ContextPtr->m_Device);

				m_CurrentWindow = p_Window;

				createRenderPass();
				//createMainRenderPass();
#ifdef IMGUI_VK_IMPL
				imgui.init();
#endif
#ifndef IMGUI_VK_IMPL
				imgui.init(m_DefaultRenderPass);
#endif

				createDepthResources();

				m_CurrentWindow->m_Swapchain.initFramebuffers(m_DefaultRenderPass.m_RenderPass, m_DepthImageAttachment.m_ImageView, m_CurrentWindow->m_ColorImageView);

				for (size_t i = 0; i < m_CurrentWindow->m_Swapchain.m_SwapchainImages.size(); i++)
					m_PrimaryCommandBuffers.emplace_back(i, Level::Primary);

				presentInfo.pSwapchains = &m_CurrentWindow->m_Swapchain.m_Swapchain;

				m_ProjectionSize = m_CurrentWindow->getWindowSize().getVec2f();

				setAspectRatio(m_CurrentWindow->getWindowSize().getVec2f());
			}
		}

		void BaseRenderer::destroy()
		{

		}

		void BaseRenderer::updateImGuiDrawData()
		{
			imgui.updateBuffers();
#ifdef IMGUI_VK_IMPL
			imgui.genCmdBuffers();
#endif
#ifndef IMGUI_VK_IMPL
			imgui.genCmdBuffers(m_DefaultInheritanceInfo);
#endif
		}

		void BaseRenderer::setViewport(Viewport p_Viewport)
		{
		}

		void BaseRenderer::setAspectRatio(Vector2f p_Size)
		{
			m_ProjectionSize = p_Size;
		}

		void BaseRenderer::beginScene()
		{
			
		}

		void BaseRenderer::setCamera(Camera& p_Camera)
		{
			setCamera(&p_Camera);
		}
		void BaseRenderer::setCamera(Camera* p_Camera)
		{
			m_Camera = p_Camera;
			m_Camera->setPerspectiveMatrix(m_Camera->getAspectRatio());
			m_Camera->setOrthographicMatrix(0, m_ProjectionSize.x, 0, m_ProjectionSize.y, 0.0f, 1.0f);
			m_TextProjection = Matrix4f::orthoRH_ZO(0, m_CurrentWindow->getWindowSize().x, 0, m_CurrentWindow->getWindowSize().y, 0.0f, 1.0f);
			m_ViewProjection = m_Camera->getViewMatrix() * m_Camera->getPerspectiveMatrix();
		}

		void BaseRenderer::endScene()
		{
			submitCommands();
		}

		void BaseRenderer::submitCommands()
		{
			//m_ThreadPool.wait();

			//float start = Utils::Logger::getTimePoint();
			
			updateImGuiDrawData();

			for (size_t i = 0; i < m_PrimaryCommandBuffers.size(); i++)
				recordPrimaryCmdBuffer(m_CurrentWindow->m_Swapchain.m_SwapchainFramebuffers[i], i);

			waitForFences();

			VkResult result = vkAcquireNextImageKHR(m_ContextPtr->m_Device, m_CurrentWindow->m_Swapchain.m_Swapchain, UINT64_MAX,
				m_ImageAvailableSemaphore[m_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex);

			//Check if a previous frame is using this image (i.e. there is its fence to wait on)
			if (m_ImagesInFlight[m_ImageIndex] != VK_NULL_HANDLE)
				vkWaitForFences(m_ContextPtr->m_Device, 1, &m_ImagesInFlight[m_ImageIndex], VK_TRUE, UINT64_MAX);

			m_ImagesInFlight[m_ImageIndex] = m_InFlightFences[m_CurrentFrame];

			// If swapchain needs recreation
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
				recreateSwapChain();
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
				Utils::Logger::logMSG("Failed to acquire swap chain image", "vkAcquireNextImageKHR", Utils::Severity::Error);

			submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphore[m_CurrentFrame];
			submitInfo.pCommandBuffers = &m_PrimaryCommandBuffers[m_ImageIndex].m_CommandBuffer;
			submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphore[m_CurrentFrame];

			vkResetFences(m_ContextPtr->m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

			// Submit the commands
			if (vkQueueSubmit(m_ContextPtr->m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
				Utils::Logger::logMSG("Failed to submit command buffer", "vkQueueSubmit", Utils::Severity::Error);

			presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphore[m_CurrentFrame];
			presentInfo.pImageIndices = &m_ImageIndex;

			result = vkQueuePresentKHR(m_ContextPtr->m_PresentQueue, &presentInfo);

			// If swapchain needs recreation
			if (result == VK_ERROR_OUT_OF_DATE_KHR || m_CurrentWindow->windowResized())
				recreateSwapChain();

			else if (result != VK_SUCCESS)
				Utils::Logger::logMSG("Failed to present swap chain image", "vkQueuePresentKHR", Utils::Severity::Error);

			//waitForFences();
			//std::cout << 1000 * (Utils::Logger::getTimePoint() - start) << std::endl;

			m_CurrentFrame = (m_CurrentFrame + 1) % 2;

			vkWaitForFences(m_ContextPtr->m_Device, 1, &m_ImagesInFlight[m_ImageIndex], VK_TRUE, UINT64_MAX);
			for (size_t i = 0; i < m_PrimaryCommandBuffers.size(); i++)
				vkResetCommandPool(m_ContextPtr->m_Device, m_ContextPtr->m_CommandPools[i], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		}

		void BaseRenderer::recordPrimaryCmdBuffer(VkFramebuffer& p_FrameBuffer, const uint32_t p_Index)
		{
			m_PrimaryCommandBuffers[p_Index].cmdBegin();

			f_PrimaryCmdRec(m_PrimaryCommandBuffers[p_Index], p_Index);

			// Default
			m_PrimaryCommandBuffers[p_Index].cmdBeginRenderPass(m_DefaultRenderPass, SubpassContents::Secondary, p_FrameBuffer, m_CurrentWindowSize);
			m_PrimaryCommandBuffers[p_Index].cmdEndRenderPass();

#ifdef IMGUI_VK_IMPL
			m_PrimaryCommandBuffers[p_Index].cmdBeginRenderPass(imgui.m_RenderPass, SubpassContents::Secondary, imgui.m_Framebuffers[p_Index], m_CurrentWindowSize);
			m_PrimaryCommandBuffers[p_Index].cmdExecuteCommands(imgui.m_CommandBuffer);
			m_PrimaryCommandBuffers[p_Index].cmdEndRenderPass();
#endif

#ifndef IMGUI_VK_IMPL
			m_PrimaryCommandBuffers[p_Index].cmdBeginRenderPass(m_DefaultRenderPass, SubpassContents::Secondary, p_FrameBuffer, m_CurrentWindowSize);
			m_PrimaryCommandBuffers[p_Index].cmdExecuteCommands(imgui.commandBuffer);
			m_PrimaryCommandBuffers[p_Index].cmdEndRenderPass();
#endif
			m_PrimaryCommandBuffers[p_Index].cmdEnd();
		}

		void BaseRenderer::waitForFences()
		{
			//vkWaitForFences(m_ContextPtr->m_Device, 1, &m_ImagesInFlight[m_ImageIndex], VK_TRUE, UINT64_MAX);
			vkWaitForFences(m_ContextPtr->m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		}

		void BaseRenderer::submitCommandBuffer(CommandBuffer& p_CmdBuffer)
		{
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &p_CmdBuffer.m_CommandBuffer;

			vkQueueSubmit(m_ContextPtr->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(m_ContextPtr->m_GraphicsQueue);
		}

		void BaseRenderer::createRenderPass()
		{
			AttachmentInfo colorAttachment;
			colorAttachment.p_AttachmentType = AttachmentType::Color;
			colorAttachment.p_Format = m_CurrentWindow->m_Swapchain.m_SwapchainImageFormat;
			colorAttachment.p_SampleCount = m_CurrentWindow->m_MSAASamples;
			colorAttachment.p_LoadOp = LoadOp::Clear;
			colorAttachment.p_StoreOp = StoreOp::Store;
			colorAttachment.p_StencilLoadOp = LoadOp::Null;
			colorAttachment.p_StencilStoreOp = StoreOp::Null;
			colorAttachment.p_InitialLayout = Layout::Undefined;
			colorAttachment.p_FinalLayout = Layout::Color;

			m_DefaultRenderPass.addAttachment(colorAttachment);

			AttachmentInfo depthAttachment;
			depthAttachment.p_AttachmentType = AttachmentType::Depth;
			depthAttachment.p_Format = m_ContextPtr->findDepthFormat();
			depthAttachment.p_SampleCount = m_CurrentWindow->m_MSAASamples;
			depthAttachment.p_LoadOp = LoadOp::Clear;
			depthAttachment.p_StoreOp = StoreOp::Null;
			depthAttachment.p_StencilLoadOp = LoadOp::Null;
			depthAttachment.p_StencilStoreOp = StoreOp::Null;
			depthAttachment.p_InitialLayout = Layout::Undefined;
			depthAttachment.p_FinalLayout = Layout::Depth;

			m_DefaultRenderPass.addAttachment(depthAttachment);

			AttachmentInfo resolveAttachment;
			resolveAttachment.p_AttachmentType = AttachmentType::Resolve;
			resolveAttachment.p_Format = m_CurrentWindow->m_Swapchain.m_SwapchainImageFormat;
			resolveAttachment.p_SampleCount = 1;
			resolveAttachment.p_LoadOp = LoadOp::Null;
			resolveAttachment.p_StoreOp = StoreOp::Store;
			resolveAttachment.p_StencilLoadOp = LoadOp::Null;
			resolveAttachment.p_StencilStoreOp = StoreOp::Null;
			resolveAttachment.p_InitialLayout = Layout::Undefined;
			resolveAttachment.p_FinalLayout = Layout::Present;
#ifdef IMGUI_VK_IMPL
			resolveAttachment.p_FinalLayout = Layout::Color;
#endif

			m_DefaultRenderPass.addAttachment(resolveAttachment);

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			m_DefaultRenderPass.m_Dependencies.push_back(dependency);

			dependency.srcSubpass = 0;
			dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			m_DefaultRenderPass.m_Dependencies.push_back(dependency);

			m_DefaultRenderPass.create();

			m_DefaultInheritanceInfo.resize(m_ContextPtr->m_SwapchainImageCount);
			m_DefaultInheritanceInfo[0] = {};
			m_DefaultInheritanceInfo[0].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			m_DefaultInheritanceInfo[0].renderPass = m_DefaultRenderPass.m_RenderPass;
			m_DefaultInheritanceInfo[0].framebuffer = VK_NULL_HANDLE;
			m_DefaultInheritanceInfo[0].subpass = 0;
			m_DefaultInheritanceInfo[1] = m_DefaultInheritanceInfo[0];
		}

		void BaseRenderer::createDepthResources()
		{
			ImageAttachmentInfo info;
			info.p_Size = m_CurrentWindow->m_WindowSize;
			info.p_AttachmentInfo = &m_DefaultRenderPass.m_AttachmentInfos[1];

			m_DepthImageAttachment.create(info);
		}

		void BaseRenderer::createSyncObjects()
		{
			m_ImageAvailableSemaphore.resize(m_ContextPtr->m_SwapchainImageCount);
			m_RenderFinishedSemaphore.resize(m_ContextPtr->m_SwapchainImageCount);
			m_InFlightFences.resize(m_ContextPtr->m_SwapchainImageCount);
			m_ImagesInFlight.resize(m_ContextPtr->m_SwapchainImageCount, VK_NULL_HANDLE);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.pNext = nullptr;
			semaphoreInfo.flags = 0;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < 2; i++)
			{
				if (vkCreateSemaphore(m_ContextPtr->m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore[i]) != VK_SUCCESS ||
					vkCreateSemaphore(m_ContextPtr->m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore[i]) != VK_SUCCESS ||
					vkCreateFence(m_ContextPtr->m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
					Utils::Logger::logMSG("Failed to create synchronization objects for frame", "Render Sync Objects", Utils::Severity::Error);
			}
		}

		void BaseRenderer::recreateSwapChain()
		{
			m_TextProjection = Matrix4f::orthoRH_ZO(0, m_CurrentWindow->getWindowSize().x, 0, m_CurrentWindow->getWindowSize().y, 0.0f, 1.0f);

			waitForFences();

			if (m_CurrentWindow->getWindowSize().x <= 1)
			{
				glfwWaitEvents();
				m_CurrentWindow->setWindowSize(2, m_CurrentWindow->getWindowSize().y);
			}
			if (m_CurrentWindow->getWindowSize().y <= 1)
			{
				glfwWaitEvents();
				m_CurrentWindow->setWindowSize(m_CurrentWindow->getWindowSize().x, 2);
			}

			m_CurrentWindowSize = m_CurrentWindow->getWindowSize();

			cleanUpSwapChain();

			m_CurrentWindow->m_Swapchain.initSwapchain(m_CurrentWindowSize);

			createRenderPass();
			m_CurrentWindow->createColorResources();
			createDepthResources();

			m_CurrentWindow->m_Swapchain.initFramebuffers(m_DefaultRenderPass.m_RenderPass, m_DepthImageAttachment.m_ImageView, m_CurrentWindow->m_ColorImageView);

			m_CurrentWindow->m_DeltaTime = 0;
			m_CurrentWindow->m_PreviousTime = Utils::Logger::getTimePoint();
#ifdef IMGUI_VK_IMPL
			imgui.swapchainRecreate();
#endif
		}

		void BaseRenderer::cleanUpSwapChain()
		{
			m_CurrentWindow->m_Swapchain.destroy();

			m_DefaultRenderPass.destroy();

			m_CurrentWindow->destroyMSAAColorImage();

			m_DepthImageAttachment.destroy();
		}
	}
}