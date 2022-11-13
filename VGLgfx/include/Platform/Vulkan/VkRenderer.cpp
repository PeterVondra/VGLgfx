#include "VkRenderer.h"
#include "VkGraphics_Internal.h"

namespace vgl
{
	namespace vk
	{
		struct Lambda {
			template<typename Tret, typename T>
			static Tret lambda_ptr_exec(void* data) {
				return (Tret)(*(T*)fn<T>())(data);
			}

			template<typename FunT, typename Tret = void, typename Tfp = Tret(FunT::*)(void*), typename T>
			static Tfp ptr(T& t) {
				fn<T>(&t);
				return (Tfp)lambda_ptr_exec<Tret, T>;
			}

			template<typename T>
			static void* fn(void* new_fn = nullptr) {
				static void* fn;
				if (new_fn != nullptr)
					fn = new_fn;
				return fn;
			}
		};

		DShadowMap::DShadowMap() : m_ContextPtr(&ContextSingleton::getInstance()), m_CommandBuffers(m_ContextPtr->m_SwapchainImageCount), m_CommandBufferIdx(0), m_Resolution(0.0f) {
			for (uint32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				CommandBuffer::allocate(Level::Secondary, 500, m_CommandBuffers[i]);

			m_Attachment.m_FramebufferAttachmentInfo.p_RenderPipelineInfo.p_RenderPass = &GraphicsContextSingleton::getInstance().getDShadowMapRenderPass();
		}
		void DShadowMap::create(Vector2i p_Resolution, ImageFormat p_ImageFormat)
		{
			// Create shadow map framebuffer
			m_Attachment.m_FramebufferAttachmentInfo.p_Size = p_Resolution;
			m_Attachment.addAttachment(p_Resolution, p_ImageFormat, vgl::Layout::DepthR);
			m_Attachment.create();

			m_Resolution = p_Resolution;
		}
		void DShadowMap::destroy()
		{
			m_Attachment.destroy();
			for (auto& cmds : m_CommandBuffers)
				for (auto& cmd : cmds)
					cmd.destroy();
		}
		
		PShadowMap::PShadowMap() : m_ContextPtr(&ContextSingleton::getInstance()), m_CommandBuffers(m_ContextPtr->m_SwapchainImageCount), m_CommandBufferIdx(0), m_Resolution(0.0f) {
			for (uint32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				CommandBuffer::allocate(Level::Secondary, 500, m_CommandBuffers[i]);

			m_Attachment.m_FramebufferAttachmentInfo.p_RenderPipelineInfo.p_RenderPass = &GraphicsContextSingleton::getInstance().getPShadowMapRenderPass();
		}
		void PShadowMap::create(Vector2i p_Resolution, ImageFormat p_ImageFormat)
		{
			// Create shadow map framebuffer
			m_Attachment.m_FramebufferAttachmentInfo.p_Size = p_Resolution;
			m_Attachment.addAttachment(p_Resolution, p_ImageFormat, vgl::Layout::ShaderR).p_ViewType = AttachmentViewType::ImageCube;
			m_Attachment.addAttachment(p_Resolution, ImageFormat::D32SF, vgl::Layout::Depth).p_ViewType = AttachmentViewType::ImageCube;
			m_Attachment.create();

			m_Resolution = p_Resolution;

			VGL_INTERNAL_ASSERT_WARNING(m_Position != nullptr, "[vk::PShadowMap]Position was nullptr, positon will be set to origin");

			if (m_Position == nullptr)
				m_PrevPosition = Vector3f(0.0f);
			else m_PrevPosition = { m_Position->x, m_Position->y, m_Position->z };

			m_Projection = Matrix4f::perspectiveRH_ZO(90.0f, p_Resolution.x / p_Resolution.y, 1.0f, 1000000.0f);

			// Prepare view matrices
			m_View[0] = Matrix4f::lookAtRH(m_PrevPosition, m_PrevPosition + Vector3f(1.0f,	0.0f,	0.0f), {0.0f, -1.0f,  0.0f})	*m_Projection;
			m_View[1] = Matrix4f::lookAtRH(m_PrevPosition, m_PrevPosition + Vector3f(-1.0f, 0.0f,	0.0f), {0.0f, -1.0f,  0.0f})	*m_Projection;
			m_View[2] = Matrix4f::lookAtRH(m_PrevPosition, m_PrevPosition + Vector3f(0.0f,	1.0f,	0.0f), {0.0f,  0.0f,  1.0f})	*m_Projection;
			m_View[3] = Matrix4f::lookAtRH(m_PrevPosition, m_PrevPosition + Vector3f(0.0f, -1.0f,	0.0f), {0.0f,  0.0f, -1.0f})	*m_Projection;
			m_View[4] = Matrix4f::lookAtRH(m_PrevPosition, m_PrevPosition + Vector3f(0.0f,	0.0f,	1.0f), {0.0f, -1.0f,  0.0f})	*m_Projection;
			m_View[5] = Matrix4f::lookAtRH(m_PrevPosition, m_PrevPosition + Vector3f(0.0f,	0.0f,  -1.0f), {0.0f, -1.0f,  0.0f})	*m_Projection;

			DescriptorSetInfo info = {};
			info.p_VertexUniformBuffer = UniformBuffer(sizeof(Matrix4f) * 6, 0);
			m_TransformDescriptor.create(info);
			m_TransformDescriptor.copy(ShaderStage::VertexBit, m_View, sizeof(Matrix4f) * 6, 0);
		}
		void PShadowMap::destroy()
		{
			m_Attachment.destroy();
			for (auto& cmds : m_CommandBuffers)
				for (auto& cmd : cmds)
					cmd.destroy();
		}

		Renderer::Renderer()
			: m_ContextPtr(&ContextSingleton::getInstance()),
			m_GraphicsContextPtr(&GraphicsContextSingleton::getInstance()),
			m_DefaultRenderPass(RenderPassType::Graphics),
			m_PostSwapchainRenderPass(RenderPassType::Graphics),
			m_ImGuiContext(nullptr)
		{ 
			m_CommandBuffers.resize(m_ContextPtr->m_SwapchainImageCount);
			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				for (int32_t j = 0; j < 1000; j++)
					m_CommandBuffers[i].emplace_back(i, Level::Secondary);
			for (size_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
					m_PrimaryCommandBuffers.emplace_back(i, Level::Primary);

			m_SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			m_SubmitInfo.waitSemaphoreCount = 1;
			m_SubmitInfo.pWaitDstStageMask = waitStages;
			m_SubmitInfo.commandBufferCount = 1;
			m_SubmitInfo.signalSemaphoreCount = 1;

			m_PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			m_PresentInfo.waitSemaphoreCount = 1;
			m_PresentInfo.swapchainCount = 1;
			m_PresentInfo.pResults = nullptr;

			m_Viewport.m_Size = m_WindowPtr->getWindowSize();

			createSyncObjects();
			m_ContextPtr->m_InFlightFencesPtr = &m_InFlightFences;

			createDefaultRenderPass();
			createDepthImageAttachment();
			m_WindowPtr->m_Swapchain.initFramebuffers(m_DefaultRenderPass.m_RenderPass, m_DepthImageAttachment.getImage().m_ImageView, m_WindowPtr->m_ColorImageView);
			m_PresentInfo.pSwapchains = &m_WindowPtr->m_Swapchain.m_Swapchain;

			createPostSwapchainRenderPass();
			m_ImGuiContext.init(m_PostSwapchainRenderPass);

			m_GraphicsContextPtr->init();
		}
		Renderer::Renderer(Window* p_Window) 
			: m_ContextPtr(&ContextSingleton::getInstance()),
			m_GraphicsContextPtr(&GraphicsContextSingleton::getInstance()),
			m_DefaultRenderPass(RenderPassType::Graphics),
			m_PostSwapchainRenderPass(RenderPassType::Graphics),
			m_ImGuiContext(p_Window)
		{
			m_CommandBuffers.resize(m_ContextPtr->m_SwapchainImageCount);
			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				for (int32_t j = 0; j < 1000; j++)
					m_CommandBuffers[i].emplace_back(i, Level::Secondary);
			for (size_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
					m_PrimaryCommandBuffers.emplace_back(i, Level::Primary);
			
			m_WindowPtr = p_Window;

			m_SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			m_SubmitInfo.waitSemaphoreCount = 1;
			m_SubmitInfo.pWaitDstStageMask = waitStages;
			m_SubmitInfo.commandBufferCount = 1;
			m_SubmitInfo.signalSemaphoreCount = 1;

			m_PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			m_PresentInfo.waitSemaphoreCount = 1;
			m_PresentInfo.swapchainCount = 1;
			m_PresentInfo.pResults = nullptr;

			m_Viewport.m_Size = m_WindowPtr->getWindowSize();

			createSyncObjects();
			m_ContextPtr->m_InFlightFencesPtr = &m_InFlightFences;

			createDefaultRenderPass();
			createDepthImageAttachment();
			m_WindowPtr->m_Swapchain.initFramebuffers(m_DefaultRenderPass.m_RenderPass, m_DepthImageAttachment.getImage().m_ImageView, m_WindowPtr->m_ColorImageView);
			m_PresentInfo.pSwapchains = &m_WindowPtr->m_Swapchain.m_Swapchain;

			createPostSwapchainRenderPass();
			m_ImGuiContext.init(m_PostSwapchainRenderPass);

			m_GraphicsContextPtr->init();
		}
		Renderer::Renderer(Window& p_Window) 
			: m_ContextPtr(&ContextSingleton::getInstance()),
			m_GraphicsContextPtr(&GraphicsContextSingleton::getInstance()),
			m_DefaultRenderPass(RenderPassType::Graphics),
			m_PostSwapchainRenderPass(RenderPassType::Graphics),
			m_ImGuiContext(&p_Window)
		{
			m_CommandBuffers.resize(m_ContextPtr->m_SwapchainImageCount);
			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				for (int32_t j = 0; j < 1000; j++)
					m_CommandBuffers[i].emplace_back(i, Level::Secondary);
			for (size_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
					m_PrimaryCommandBuffers.emplace_back(i, Level::Primary);

			m_WindowPtr = &p_Window;

			m_SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			m_SubmitInfo.waitSemaphoreCount = 1;
			m_SubmitInfo.pWaitDstStageMask = waitStages;
			m_SubmitInfo.commandBufferCount = 1;
			m_SubmitInfo.signalSemaphoreCount = 1;

			m_PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			m_PresentInfo.waitSemaphoreCount = 1;
			m_PresentInfo.swapchainCount = 1;
			m_PresentInfo.pResults = nullptr;

			m_Viewport.m_Size = m_WindowPtr->getWindowSize();

			createSyncObjects();
			m_ContextPtr->m_InFlightFencesPtr = &m_InFlightFences;

			createDefaultRenderPass();
			createDepthImageAttachment();
			m_WindowPtr->m_Swapchain.initFramebuffers(m_DefaultRenderPass.m_RenderPass, m_DepthImageAttachment.getImage().m_ImageView, m_WindowPtr->m_ColorImageView);
			m_PresentInfo.pSwapchains = &m_WindowPtr->m_Swapchain.m_Swapchain;

			createPostSwapchainRenderPass();
			m_ImGuiContext.init(m_PostSwapchainRenderPass);

			m_GraphicsContextPtr->init();
		}
		Renderer::Renderer(Window& p_Window, float p_RenderResolutionScale) 
			: m_ContextPtr(&ContextSingleton::getInstance()),
			m_GraphicsContextPtr(&GraphicsContextSingleton::getInstance()),
			m_DefaultRenderPass(RenderPassType::Graphics),
			m_PostSwapchainRenderPass(RenderPassType::Graphics),
			m_ImGuiContext(&p_Window)
		{ 
			m_CommandBuffers.resize(m_ContextPtr->m_SwapchainImageCount);
			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				for (int32_t j = 0; j < 1000; j++)
					m_CommandBuffers[i].emplace_back(i, Level::Secondary);

			for (size_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
					m_PrimaryCommandBuffers.emplace_back(i, Level::Primary);

			m_WindowPtr = &p_Window;

			m_SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			m_SubmitInfo.waitSemaphoreCount = 1;
			m_SubmitInfo.pWaitDstStageMask = waitStages;
			m_SubmitInfo.commandBufferCount = 1;
			m_SubmitInfo.signalSemaphoreCount = 1;

			m_PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			m_PresentInfo.waitSemaphoreCount = 1;
			m_PresentInfo.swapchainCount = 1;
			m_PresentInfo.pResults = nullptr;

			m_Viewport.m_Size = m_WindowPtr->getWindowSize();

			createSyncObjects();
			m_ContextPtr->m_InFlightFencesPtr = &m_InFlightFences;

			createDefaultRenderPass();
			createDepthImageAttachment();
			m_WindowPtr->m_Swapchain.initFramebuffers(m_DefaultRenderPass.m_RenderPass, m_DepthImageAttachment.getImage().m_ImageView, m_WindowPtr->m_ColorImageView);
			m_PresentInfo.pSwapchains = &m_WindowPtr->m_Swapchain.m_Swapchain;

			createPostSwapchainRenderPass();
			m_ImGuiContext.init(m_PostSwapchainRenderPass);

			m_GraphicsContextPtr->init();
		}

		void Renderer::beginRenderPass(RenderInfo& p_RenderInfo)
		{
			m_RenderInfo = p_RenderInfo;

			m_RecordMeshDataFun = &Renderer::recordMeshData;
		}
		void Renderer::beginRenderPass(RenderInfo& p_RenderInfo, FramebufferAttachment& p_FramebufferAttachment)
		{
			m_FramebufferAttachmentPtr = &p_FramebufferAttachment;
			m_RenderInfo = p_RenderInfo;

			m_RecordMeshDataFun = &Renderer::recordMeshData;

			m_RecordPrimaryCmdBuffersFunPtrs.emplace_back(&p_FramebufferAttachment, &Renderer::recordPCmdFun);
		}
		void Renderer::beginRenderPass(RenderInfo& p_RenderInfo, DShadowMap& p_ShadowMap)
		{
			p_ShadowMap.m_CommandBufferIdx = 0;

			m_RenderInfo = p_RenderInfo;

			m_FramebufferAttachmentPtr = &p_ShadowMap.m_Attachment;
			m_DShadowMapPtr = &p_ShadowMap;

			m_RecordMeshDataFun = &Renderer::recordMeshDataSMD;

			m_RecordPrimaryCmdBuffersFunPtrs.emplace_back(&p_ShadowMap, &Renderer::recordPCmdSMAPFunDS);
		}

		void Renderer::beginRenderPass(RenderInfo& p_RenderInfo, PShadowMap& p_ShadowMap)
		{
			p_ShadowMap.m_CommandBufferIdx = 0;

			m_RenderInfo = p_RenderInfo;

			m_FramebufferAttachmentPtr = &p_ShadowMap.m_Attachment;
			m_CurrentPShadowMapPtr = &p_ShadowMap;

			m_RecordMeshDataFun = &Renderer::recordMeshDataSMP;

			m_RecordPrimaryCmdBuffersFunPtrs.emplace_back(&p_ShadowMap, &Renderer::recordPCmdSMAPFunP);
		}

		void Renderer::submit(Camera& p_Camera)
		{
			m_Camera = &p_Camera;
			m_Camera->setOrthographicMatrix(0, m_OrthoProjectionSize.x, 0, m_OrthoProjectionSize.y, 0.0f, 1.0f);
			m_FontProjection = Matrix4f::orthoRH_ZO(0, m_WindowPtr->getWindowSize().x, 0, m_WindowPtr->getWindowSize().y, 0.0f, 1.0f);
			m_ViewProjection = m_Camera->getViewMatrix() * m_Camera->getPerspectiveMatrix();
		}
		
		void Renderer::submit(Viewport& p_Viewport)
		{

		}
		
		void Renderer::submit(Shader& p_Shader)
		{

		}
		void Renderer::submit(void* p_Data, const uint32_t p_Size)
		{
		}
		void Renderer::submit(Image& p_Image, const uint32_t p_Binding)
		{

		}
		void Renderer::submit(std::vector<ImageAttachment>& p_Images, const uint32_t p_Binding)
		{
			for (int32_t i = 0; i < p_Images.size(); i++)
				m_DescriptorSetInfo.p_ImageDescriptors.emplace_back(&p_Images[i].getImage(), 0, i);
		}
		void Renderer::submit(MeshData& p_MeshData, const uint32_t p_MTLIndex, Transform3D& p_Transform)
		{

		}
		void Renderer::submit(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			(this->*(m_RecordMeshDataFun))(p_MeshData, p_Transform);
		}
		
		void Renderer::submit(Skybox& p_Skybox, AtmosphericScatteringInfo& p_Info)
		{
			m_AtmosphericScatteringInfoPtr = &p_Info;
			if(p_Skybox.m_CubeMap != nullptr)
				if (p_Skybox.m_CubeMap->isValid())
					m_AtmosphericScatteringInfoPtr = nullptr;
			if(m_AtmosphericScatteringInfoPtr) m_AtmosphericScatteringInfoPtr->p_RayOrigin = m_Camera->getPosition();

			recordMeshDataSkybox(p_Skybox, p_Skybox);
		}
		void Renderer::blitImage(Image& p_Image)
		{
			m_RecordPrimaryCmdBuffersFunPtrs.emplace_back(&p_Image, &Renderer::blitImageFun);
		}
		void Renderer::endRenderPass()
		{
			
		}
		void Renderer::render()
		{
		}
		// Directional Shadow map
		void Renderer::recordMeshDataSMD(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			VGL_INTERNAL_ASSERT_WARNING(m_DShadowMapPtr != nullptr, "[vk::Renderer]No shadow map bound");
			if (m_DShadowMapPtr == nullptr) return;

			for (uint32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
				CommandBuffer& cmd = m_DShadowMapPtr->m_CommandBuffers[i][m_DShadowMapPtr->m_CommandBufferIdx];

				// Shadow map rendering
				cmd.cmdBegin(m_DShadowMapPtr->m_Attachment.m_InheritanceInfo[i]);

				cmd.cmdSetViewport(
					Viewport({
						int(m_DShadowMapPtr->m_Resolution.x), int(-m_DShadowMapPtr->m_Resolution.y) },
						Vector2i(0, m_DShadowMapPtr->m_Resolution.y)
						)
				);
				cmd.cmdSetScissor(
					Scissor({
						int(m_DShadowMapPtr->m_Resolution.x),
						int(m_DShadowMapPtr->m_Resolution.y) }, { 0, 0 }
						)
				);

				Matrix4f data = p_Transform.model * m_DShadowMapPtr->m_View * m_DShadowMapPtr->m_Projection;

				vkCmdSetDepthBias
				(
					cmd.m_CommandBuffer,
					m_DShadowMapPtr->m_DepthBiasConstant,
					0.0f,
					m_DShadowMapPtr->m_DepthBiasSlope
				);

				uint32_t albedo_concurrent = 0;
				uint32_t default_concurrent = 0;
				cmd.cmdBindVertexArray(p_MeshData.m_VertexArray);
				uint32_t prevIndex = 0;
				for (int j = 0; j < p_MeshData.m_Materials.size(); j++) {
					// Should material be rendered?
					if (!p_MeshData.m_Materials[j].config.render) {
						cmd.cmdDrawIndexed(prevIndex, p_MeshData.m_SubMeshIndices[j].first);
						prevIndex = p_MeshData.m_SubMeshIndices[j].second;
					}
					else if (p_MeshData.m_Materials[j].m_AlbedoMap.isValid()) {
						if (albedo_concurrent == 0) { // --> If pipeline is already bound, skip this step
							cmd.cmdBindPipeline(m_GraphicsContextPtr->getDShadowMapAlbedoPipeline());
							vkCmdPushConstants(cmd.vkHandle(), m_GraphicsContextPtr->getDShadowMapAlbedoPipeline().m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Matrix4f), &data);
						}
						vkCmdBindDescriptorSets(
							cmd.vkHandle(),
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							m_GraphicsContextPtr->getDShadowMapAlbedoPipeline().m_PipelineLayout,
							0, 1,
							&p_MeshData.m_Materials[j].m_AlbedoMap.m_DescriptorSet,
							0, nullptr
						);

						cmd.cmdDrawIndexed(prevIndex, p_MeshData.m_SubMeshIndices[j].second);
						prevIndex = p_MeshData.m_SubMeshIndices[j].second;
						albedo_concurrent++;
						default_concurrent = 0;
					}
					else {
						if (default_concurrent == 0) { // --> If pipeline is already bound, skip this step
							cmd.cmdBindPipeline(m_GraphicsContextPtr->getDShadowMapPipeline());
							vkCmdPushConstants(cmd.vkHandle(), m_GraphicsContextPtr->getDShadowMapPipeline().m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Matrix4f), &data);
						}
						cmd.cmdDrawIndexed(prevIndex, p_MeshData.indices.size());
						albedo_concurrent = 0;
						default_concurrent++;
					}
				}
				cmd.cmdEnd();
			}
			m_DShadowMapPtr->m_CommandBufferIdx++;
		}

		void Renderer::recordMeshDataSMP(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			VGL_INTERNAL_ASSERT_WARNING(m_CurrentPShadowMapPtr != nullptr, "[vk::Renderer]No Omni-shadow map bound");
			if (m_CurrentPShadowMapPtr == nullptr) return;

			struct PSMT {
				Matrix4f model;
				// Light position
				Vector3f position;
			};

			for (uint32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
				CommandBuffer& cmd = m_CurrentPShadowMapPtr->m_CommandBuffers[i][m_CurrentPShadowMapPtr->m_CommandBufferIdx];

				// Shadow map rendering
				cmd.cmdBegin(m_CurrentPShadowMapPtr->m_Attachment.m_InheritanceInfo[i]);

				cmd.cmdSetViewport(
					Viewport({
						int(m_CurrentPShadowMapPtr->m_Resolution.x), int(m_CurrentPShadowMapPtr->m_Resolution.y) },
						Vector2i(0, 0)
						)
				);
				cmd.cmdSetScissor(
					Scissor({
						int(m_CurrentPShadowMapPtr->m_Resolution.x),
						int(m_CurrentPShadowMapPtr->m_Resolution.y) }, { 0, 0 }
						)
				);

				// If light position is changed, update view matrices
				if (*m_CurrentPShadowMapPtr->m_Position != m_CurrentPShadowMapPtr->m_PrevPosition) {
					m_CurrentPShadowMapPtr->m_PrevPosition = *m_CurrentPShadowMapPtr->m_Position;
#define CPSMPTR m_CurrentPShadowMapPtr
					// Prepare view matrices
					CPSMPTR->m_View[0] = Matrix4f::lookAtRH(
						CPSMPTR->m_PrevPosition, 
						CPSMPTR->m_PrevPosition + Vector3f(1.0f,  0.0f,  0.0f),
						Vector3f(0.0f, -1.0f,  0.0f)
					)*CPSMPTR->m_Projection;

					CPSMPTR->m_View[1] = Matrix4f::lookAtRH(
						CPSMPTR->m_PrevPosition,
						CPSMPTR->m_PrevPosition + Vector3f(-1.0f, 0.0f,  0.0f),
						Vector3f(0.0f, -1.0f,  0.0f)
					)*CPSMPTR->m_Projection;

					CPSMPTR->m_View[2] = Matrix4f::lookAtRH(
						CPSMPTR->m_PrevPosition,
						CPSMPTR->m_PrevPosition + Vector3f(0.0f,  1.0f,  0.0f),
						Vector3f(0.0f,  0.0f,  1.0f)
					)*CPSMPTR->m_Projection;

					CPSMPTR->m_View[3] = Matrix4f::lookAtRH(
						CPSMPTR->m_PrevPosition,
						CPSMPTR->m_PrevPosition + Vector3f(0.0f, -1.0f,  0.0f),
						Vector3f(0.0f,  0.0f, -1.0f)
					)*CPSMPTR->m_Projection;

					CPSMPTR->m_View[4] = Matrix4f::lookAtRH(
						CPSMPTR->m_PrevPosition,
						CPSMPTR->m_PrevPosition + Vector3f(0.0f,  0.0f,  1.0f),
						Vector3f(0.0f, -1.0f,  0.0f)
					)*CPSMPTR->m_Projection;

					CPSMPTR->m_View[5] = Matrix4f::lookAtRH(
						CPSMPTR->m_PrevPosition,
						CPSMPTR->m_PrevPosition + Vector3f(0.0f,  0.0f, -1.0f),
						Vector3f(0.0f, -1.0f,  0.0f)
					)*CPSMPTR->m_Projection;
#undef CPSMPTR
				}

				PSMT data = {};
				data.model = p_Transform.model;
				data.position = m_CurrentPShadowMapPtr->m_PrevPosition;

				//vkCmdSetDepthBias
				//(
				//	cmd.m_CommandBuffer,
				//	m_CurrentPShadowMapPtr->m_DepthBiasConstant,
				//	0.0f,
				//	m_CurrentPShadowMapPtr->m_DepthBiasSlope
				//);

				vkCmdBindDescriptorSets(
					cmd.vkHandle(),
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_GraphicsContextPtr->getPShadowMapAlbedoPipeline().m_PipelineLayout,
					0, 1,
					&m_CurrentPShadowMapPtr->m_TransformDescriptor.m_DescriptorSets[i].m_DescriptorSet,
					0, nullptr
				);

				uint32_t albedo_concurrent = 0;
				uint32_t default_concurrent = 0;
				cmd.cmdBindVertexArray(p_MeshData.m_VertexArray);
				uint32_t prevIndex = 0;
				for (int j = 0; j < p_MeshData.m_Materials.size(); j++) {
					// Should material be rendered?
					if (!p_MeshData.m_Materials[j].config.render) {
						cmd.cmdDrawIndexed(prevIndex, p_MeshData.m_SubMeshIndices[j].first);
						prevIndex = p_MeshData.m_SubMeshIndices[j].second;
					}
					else if (p_MeshData.m_Materials[j].m_AlbedoMap.isValid()) {
						if (albedo_concurrent == 0) { // --> If pipeline is already bound, skip this step
							cmd.cmdBindPipeline(m_GraphicsContextPtr->getPShadowMapAlbedoPipeline());
							vkCmdPushConstants(cmd.vkHandle(), m_GraphicsContextPtr->getPShadowMapAlbedoPipeline().m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PSMT), &data);
						}
						vkCmdBindDescriptorSets(
							cmd.vkHandle(),
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							m_GraphicsContextPtr->getPShadowMapAlbedoPipeline().m_PipelineLayout,
							1, 1,
							&p_MeshData.m_Materials[j].m_AlbedoMap.m_DescriptorSet,
							0, nullptr
						);

						cmd.cmdDrawIndexed(prevIndex, p_MeshData.m_SubMeshIndices[j].second);
						prevIndex = p_MeshData.m_SubMeshIndices[j].second;
						albedo_concurrent++;
						default_concurrent = 0;
					}
					else {
						if (default_concurrent == 0) { // --> If pipeline is already bound, skip this step
							cmd.cmdBindPipeline(m_GraphicsContextPtr->getPShadowMapPipeline());
							vkCmdPushConstants(cmd.vkHandle(), m_GraphicsContextPtr->getPShadowMapPipeline().m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PSMT), &data);
						}
						cmd.cmdDrawIndexed(prevIndex, p_MeshData.indices.size());
						albedo_concurrent = 0;
						default_concurrent++;
					}
				}
				cmd.cmdEnd();
			}
			m_CurrentPShadowMapPtr->m_CommandBufferIdx++;
		}
		
		void Renderer::createPipelines(MeshData& p_MeshData, RenderInfo& p_RenderInfo, RenderPass& p_RenderPass)
		{
			g_PipelineInfo pipelineInfo;
			
			pipelineInfo.p_CullMode = p_RenderInfo.p_CullMode;
			pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
			pipelineInfo.p_PolygonMode = p_RenderInfo.p_PolygonMode;
			pipelineInfo.p_MSAASamples = 1;
			pipelineInfo.p_Viewport = Viewport(m_WindowPtr->m_WindowSize, { 0, 0 });
			pipelineInfo.p_Scissor = Scissor(m_WindowPtr->m_WindowSize, { 0, 0 });
			pipelineInfo.p_SampleRateShading = true;
			pipelineInfo.p_RenderPass = &p_RenderPass;
			pipelineInfo.p_AlphaBlending = false;// p_RenderInfo.p_AlphaBlending;

			pipelineInfo.p_AttributeDescription = p_MeshData.m_VertexArray.getAttributeDescriptions();
			pipelineInfo.p_BindingDescription = p_MeshData.m_VertexArray.getBindingDescription();

			for (int i = 0; i < p_MeshData.m_Materials.size(); i++) {
				bool c = false;

				for (int k = 0; k < m_ShaderData.size(); k++) {
					if (p_MeshData.m_Materials[i].m_ShaderInfo == m_ShaderData[k].p_Info) {
						c = true;
						p_MeshData.m_Materials[i].m_PipelineIndex = k;
					}
				}

				if (c) continue;

				m_ShaderData.emplace_back();

				Shader shader;
				shader.compile(ShaderPermutationsGenerator::generateGBufferShader(p_MeshData.m_Materials[i].m_ShaderInfo, ShaderType::Deferred));
				m_ShaderData[m_ShaderData.size() - 1].p_Shader = shader;

				pipelineInfo.p_Shader = &m_ShaderData[m_ShaderData.size() - 1].p_Shader;

				pipelineInfo.p_DescriptorSetLayouts = { p_MeshData.m_MTLDescriptors[i].m_DescriptorSetLayout };
				
				m_ShaderData[m_ShaderData.size() - 1].p_Pipeline.create(pipelineInfo);

				p_MeshData.m_Materials[i].m_PipelineIndex = m_ShaderData.size() - 1;
			}
		}

		void Renderer::recordMeshData(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			waitForFences();

			if (p_MeshData.m_MTLRecreateFlag || p_MeshData.m_RecreateFlag) {
				if (p_MeshData.m_RecreateFlag) {
					for (auto& mtl : p_MeshData.m_MTLDescriptors)
						if (mtl.isValid()) {
							m_ContextPtr->deviceWaitIdle();
							mtl.destroy();
						}
				}

				if (!p_MeshData.m_RecreateFlag || p_MeshData.m_MTLRecreateFlag)
					p_MeshData.initMaterials();

				createPipelines(p_MeshData, m_RenderInfo, m_FramebufferAttachmentPtr->m_RenderPass);

				p_MeshData.m_RecreateFlag = false;
				p_MeshData.m_MTLRecreateFlag = false;
			}

			for (int16_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				recordMeshCmdBuffer(p_MeshData, *m_FramebufferAttachmentPtr, m_CommandBuffers[i][m_CommandBufferIdx], i);

			float lumaThreshold = 2.532f;

			for (int i = 0; i < p_MeshData.m_Materials.size(); i++)
			{
				if (!p_MeshData.m_Materials[i].config.render)
					continue;

				//if (!(p_MeshData.m_Materials[i].m_PrevConfig == p_MeshData.m_Materials[i].config)) {
					p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::FragmentBit, Vector4f(
						p_MeshData.m_Materials[i].config.m_Albedo.r,
						p_MeshData.m_Materials[i].config.m_Albedo.g,
						p_MeshData.m_Materials[i].config.m_Albedo.b,
						p_MeshData.m_Materials[i].config.m_PDMDepth), 0);
					p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::FragmentBit,
						Vector4f(
							p_MeshData.m_Materials[i].config.m_Metallic,
							p_MeshData.m_Materials[i].config.m_Roughness,
							p_MeshData.m_Materials[i].config.m_Ambient, lumaThreshold
						), 4 * sizeof(float)
					);
				//}

				p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::FragmentBit, Vector3f(3, 3, 3), 12 * sizeof(float));
				for(int k = 0; k < p_MeshData.m_Materials.size(); k++)
					for(int j = 0; j < p_MeshData.m_Materials[k].m_ShaderInfo.p_PointLights.size(); j++)
						p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::FragmentBit, p_MeshData.m_Materials[k].m_ShaderInfo.p_PointLights[j], 16 * sizeof(float) + sizeof(P_Light)*j);

				// MVP Matrix
				p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::VertexBit, p_Transform.model * m_ViewProjection, 0);
				// Model Matrix
				p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::VertexBit, p_Transform.model, sizeof(Matrix4f));
				// View Matrix
				p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::VertexBit, m_Camera->getViewMatrix(), 2*sizeof(Matrix4f));
				// Camera position
				p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::VertexBit, m_Camera->getPosition(), 3*sizeof(Matrix4f));

				p_MeshData.m_Materials[i].m_PrevConfig = p_MeshData.m_Materials[i].config;
			}

			m_CommandBufferIdx++;
		}

		void Renderer::recordMeshCmdBuffer(MeshData& p_MeshData, FramebufferAttachment& p_FramebufferAttachment, CommandBuffer& p_CommandBuffer, const uint32_t p_ImageIndex)
		{
			p_CommandBuffer.cmdBegin(p_FramebufferAttachment.m_InheritanceInfo[p_ImageIndex]);
			setCmdViewport(p_CommandBuffer, p_FramebufferAttachment.m_FramebufferAttachmentInfo.p_Size);

			for (int i = 0; i < p_MeshData.m_Materials.size(); i++)
			{
				if (!p_MeshData.m_Materials[i].config.render)
					continue;

				p_CommandBuffer.cmdBindPipeline(m_ShaderData[p_MeshData.m_Materials[i].m_PipelineIndex].p_Pipeline);
				p_CommandBuffer.cmdBindDescriptorSets(p_MeshData.m_MTLDescriptors[i], p_ImageIndex);
				p_CommandBuffer.cmdBindVertexArray(p_MeshData.m_VertexArray);
				p_CommandBuffer.cmdDrawIndexed(p_MeshData.m_SubMeshIndices[i].first, p_MeshData.m_SubMeshIndices[i].second);
			}

			p_CommandBuffer.cmdEnd();
		}

		void Renderer::recordMeshDataSkybox(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			waitForFences();

			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
				m_CommandBuffers[i][m_CommandBufferIdx].cmdBegin(m_FramebufferAttachmentPtr->m_InheritanceInfo[i]);
				setCmdViewport(m_CommandBuffers[i][m_CommandBufferIdx], m_FramebufferAttachmentPtr->m_FramebufferAttachmentInfo.p_Size);
			
				if(m_AtmosphericScatteringInfoPtr)
					m_CommandBuffers[i][m_CommandBufferIdx].cmdBindPipeline(m_GraphicsContextPtr->getAtmosphericScatteringPipeline());
				else 
					m_CommandBuffers[i][m_CommandBufferIdx].cmdBindPipeline(m_GraphicsContextPtr->getSkyBoxPipeline());

				m_CommandBuffers[i][m_CommandBufferIdx].cmdBindDescriptorSets(p_MeshData.m_MTLDescriptors[0], 0);
			
				if(m_AtmosphericScatteringInfoPtr)
					vkCmdPushConstants(
						m_CommandBuffers[i][m_CommandBufferIdx].m_CommandBuffer,
						m_GraphicsContextPtr->getAtmosphericScatteringPipeline().m_PipelineLayout,
						getShaderStageVkH(ShaderStage::FragmentBit),
						0, sizeof(AtmosphericScatteringInfo), m_AtmosphericScatteringInfoPtr
					);

				m_CommandBuffers[i][m_CommandBufferIdx].cmdBindVertexArray(p_MeshData.m_VertexArray);
				m_CommandBuffers[i][m_CommandBufferIdx].cmdDrawIndexed(p_MeshData.m_SubMeshIndices[0].first, p_MeshData.m_SubMeshIndices[0].second);
			
				m_CommandBuffers[i][m_CommandBufferIdx].cmdEnd();

			}
			p_MeshData.m_MTLDescriptors[0].copy(ShaderStage::VertexBit, p_Transform.model * m_ViewProjection, 0);
			
			m_CommandBufferIdx++;
		}
		
		void Renderer::setCmdDefaultViewport(CommandBuffer& p_CommandBuffer)
		{
			p_CommandBuffer.cmdSetViewport(
				Viewport({
					int(m_Viewport.m_Size.x), int(-m_Viewport.m_Size.y) },
					Vector2i(0, int(m_Viewport.m_Size.y)) + m_Viewport.m_Position
				)
			);
			p_CommandBuffer.cmdSetScissor(
				Scissor({
					int(m_Viewport.m_Size.x),
					int(m_Viewport.m_Size.y) }, { 0, 0 }
				)
			);
		}

		void Renderer::setCmdViewport(CommandBuffer& p_CommandBuffer, Vector2i p_Size)
		{
			p_CommandBuffer.cmdSetViewport(
				Viewport({
					int(p_Size.x), int(-p_Size.y) },
					Vector2i(0, int(p_Size.y))
					)
			);
			p_CommandBuffer.cmdSetScissor(
				Scissor({
					int(p_Size.x),
					int(p_Size.y) }, { 0, 0 }
					)
			);
		}

		void Renderer::blitImageFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex)
		{
			Image* p_Image = (Image*)p_Ptr;

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = p_Image->m_VkImageHandle;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = 1;

			int mipWidth = p_Image->m_Size.x;
			int mipHeight = p_Image->m_Size.y;

			VkImageLayout layout = p_Image->m_CurrentLayout;

			m_ContextPtr->setImageLayout(
				p_PrimaryCommandBuffer.m_CommandBuffer,
				p_Image->m_VkImageHandle,
				VK_IMAGE_ASPECT_COLOR_BIT,
				p_Image->m_CurrentLayout,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			);
			

			for (int i = 1; i < p_Image->m_MipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i;
				m_ContextPtr->setImageLayout(p_PrimaryCommandBuffer.m_CommandBuffer, p_Image->m_VkImageHandle, p_Image->m_CurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, barrier.subresourceRange);

				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(
					p_PrimaryCommandBuffer.m_CommandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0, 0, nullptr, 0, nullptr, 1,
					&barrier
				);

				VkImageBlit blit = {};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;

				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				vkCmdBlitImage(
					p_PrimaryCommandBuffer.m_CommandBuffer,
					p_Image->m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					p_Image->m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit, VK_FILTER_LINEAR
				);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(
					p_PrimaryCommandBuffer.m_CommandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0, 0, nullptr, 0, nullptr, 1, 
					&barrier
				);

				if (mipWidth > 1) mipWidth /= 2;

				if (mipHeight > 1) mipHeight /= 2;
			}
			
			p_Image->m_CurrentLayout = p_Image->m_FinalLayout;

			barrier.subresourceRange.baseMipLevel = p_Image->m_MipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				p_PrimaryCommandBuffer.m_CommandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, nullptr, 0, nullptr, 1,
				&barrier
			);
		}
		
		void Renderer::recordPrimaryCommandBuffers(uint32_t p_ImageIndex)
		{
			waitForFences();
			m_PrimaryCommandBuffers[p_ImageIndex].cmdBegin();

			// Record all submitted renderpasses
			for (auto& cmdFun : m_RecordPrimaryCmdBuffersFunPtrs)
				(this->*cmdFun.pcmdRecFun)(cmdFun.ptr, m_PrimaryCommandBuffers[p_ImageIndex], p_ImageIndex);

			if (p_ImageIndex == m_ContextPtr->m_SwapchainImageCount - 1) {
				m_RecordPrimaryCmdBuffersFunPtrs.clear();
				m_CommandBufferIdx = 0;
			}

			// Default
			m_PrimaryCommandBuffers[p_ImageIndex].cmdBeginRenderPass(
				m_DefaultRenderPass,
				SubpassContents::Secondary, 
				m_WindowPtr->m_Swapchain.m_SwapchainFramebuffers[p_ImageIndex], 
				m_Viewport.m_Size
			);
			m_PrimaryCommandBuffers[p_ImageIndex].cmdEndRenderPass();

#ifdef VGL_IMGUI_VK_IMPL
			m_PrimaryCommandBuffers[p_ImageIndex].cmdBeginRenderPass(
				*m_ImGuiContext.m_RenderPass, 
				SubpassContents::Inline, 
				m_ImGuiContext.m_Framebuffers[p_ImageIndex], 
				m_Viewport.m_Size
			);
			//m_PrimaryCommandBuffers[p_ImageIndex].cmdExecuteCommands(m_ImGuiContext.m_CommandBuffer);
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_PrimaryCommandBuffers[p_ImageIndex].vkHandle());
			m_PrimaryCommandBuffers[p_ImageIndex].cmdEndRenderPass();
#endif
			m_PrimaryCommandBuffers[p_ImageIndex].cmdEnd();
		}

		void Renderer::recordPCmdFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex)
		{
			FramebufferAttachment* framebufferPtr = (FramebufferAttachment*)p_Ptr;

			if (framebufferPtr) {
				if (framebufferPtr->m_FramebufferAttachmentInfo.p_RenderPipelineInfo.p_CreateGraphicsPipeline)
					framebufferPtr->recordCmdBuffer(p_PrimaryCommandBuffer, p_ImageIndex);
				else{

					framebufferPtr->cmdBeginRenderPass(p_PrimaryCommandBuffer, SubpassContents::Secondary, p_ImageIndex);
					p_PrimaryCommandBuffer.cmdExecuteCommands(m_CommandBuffers[p_ImageIndex], m_CommandBufferIdx);
					framebufferPtr->cmdEndRenderPass();
				}
			}
		}

		void Renderer::recordPCmdSMAPFunDS(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex)
		{
			DShadowMap* shadowPtr = (DShadowMap*)p_Ptr;

			if (shadowPtr)
				if (!shadowPtr->m_CommandBuffers.empty()) {
					shadowPtr->m_Attachment.cmdBeginRenderPass(p_PrimaryCommandBuffer, SubpassContents::Secondary, p_ImageIndex);
					p_PrimaryCommandBuffer.cmdExecuteCommands(shadowPtr->m_CommandBuffers[p_ImageIndex], shadowPtr->m_CommandBufferIdx);
					shadowPtr->m_Attachment.cmdEndRenderPass();
				}
		}
		void Renderer::recordPCmdSMAPFunP(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex)
		{
			PShadowMap* shadowPtr = (PShadowMap*)p_Ptr;

			if (shadowPtr)
				if (!shadowPtr->m_CommandBuffers[0].empty()) {
					shadowPtr->m_Attachment.cmdBeginRenderPass(p_PrimaryCommandBuffer, SubpassContents::Secondary, p_ImageIndex);
					p_PrimaryCommandBuffer.cmdExecuteCommands(shadowPtr->m_CommandBuffers[p_ImageIndex], shadowPtr->m_CommandBufferIdx);
					shadowPtr->m_Attachment.cmdEndRenderPass();
				}
		}

		void Renderer::GPUSubmit()
		{
			//float start = Utils::Logger::getTimePoint();
#ifndef VGL_IMGUI_VK_IMPL
			m_ImGuiContext.updateBuffers();
			m_ImGuiContext.genCmdBuffers();
#endif
#ifdef VGL_IMGUI_VK_IMPL
			//m_ImGuiContext.genCmdBuffers();
			m_ImGuiContext.updateViewports();
#endif

			for (size_t i = 0; i < m_PrimaryCommandBuffers.size(); i++) recordPrimaryCommandBuffers(i);

			waitForFences();
			VkResult result = vkAcquireNextImageKHR(m_ContextPtr->m_Device, m_WindowPtr->m_Swapchain.m_Swapchain, UINT64_MAX,
				m_ImageAvailableSemaphore[m_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex);

			//Check if a previous frame is using this image (i.e. there is its fence to wait on)
			if (m_ImagesInFlight[m_ImageIndex] != VK_NULL_HANDLE)
				vkWaitForFences(m_ContextPtr->m_Device, 1, &m_ImagesInFlight[m_ImageIndex], VK_TRUE, UINT64_MAX);

			m_ImagesInFlight[m_ImageIndex] = m_InFlightFences[m_CurrentFrame];

			// If swapchain needs recreation
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
				recreateSwapChain();
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
				VGL_INTERNAL_ERROR("[vk::Renderer]Failed to acquire swap chain image");

			m_SubmitInfo.pWaitSemaphores = &m_ImageAvailableSemaphore[m_CurrentFrame];
			m_SubmitInfo.pCommandBuffers = &m_PrimaryCommandBuffers[m_ImageIndex].m_CommandBuffer;
			m_SubmitInfo.pSignalSemaphores = &m_RenderFinishedSemaphore[m_CurrentFrame];

			vkResetFences(m_ContextPtr->m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

			// Submit the commands to the graphics queue
			if (vkQueueSubmit(m_ContextPtr->m_GraphicsQueue, 1, &m_SubmitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
				VGL_INTERNAL_ERROR("[vk::Renderer]Failed to submit command buffer");

			m_PresentInfo.pWaitSemaphores = &m_RenderFinishedSemaphore[m_CurrentFrame];
			m_PresentInfo.pImageIndices = &m_ImageIndex;

			result = vkQueuePresentKHR(m_ContextPtr->m_PresentQueue, &m_PresentInfo);

			// If swapchain needs recreation
			if (result == VK_ERROR_OUT_OF_DATE_KHR || m_WindowPtr->windowResized())
				recreateSwapChain();

			else if (result != VK_SUCCESS)
				VGL_INTERNAL_ERROR("[vk::Renderer]Failed to present swap chain image");

			//waitForFences();
			//std::cout << 1000 * (Utils::Logger::getTimePoint() - start) << std::endl;

			m_CurrentFrame = GET_CURRENT_IMAGE_INDEX(m_CurrentFrame);

			m_ContextPtr->m_ImageIndex = m_ImageIndex;
			m_ContextPtr->m_CurrentFrame = m_CurrentFrame;

			vkWaitForFences(m_ContextPtr->m_Device, 1, &m_ImagesInFlight[m_ImageIndex], VK_TRUE, UINT64_MAX);
		}

		void Renderer::createDefaultRenderPass()
		{
			AttachmentInfo colorAttachment = {};
			colorAttachment.p_AttachmentType = AttachmentType::Color;
			colorAttachment.p_Format = m_WindowPtr->m_Swapchain.m_SwapchainImageFormat;
			colorAttachment.p_SampleCount = m_WindowPtr->m_MSAASamples;
			colorAttachment.p_LoadOp = LoadOp::Clear;
			colorAttachment.p_StoreOp = StoreOp::Store;
			colorAttachment.p_StencilLoadOp = LoadOp::Null;
			colorAttachment.p_StencilStoreOp = StoreOp::Null;
			colorAttachment.p_InitialLayout = Layout::Undefined;
			colorAttachment.p_FinalLayout = Layout::Color;

			m_DefaultRenderPass.addAttachment(colorAttachment);

			AttachmentInfo depthAttachment = {};
			depthAttachment.p_AttachmentType = AttachmentType::Depth;
			depthAttachment.p_Format = m_ContextPtr->findDepthFormat();
			depthAttachment.p_SampleCount = m_WindowPtr->m_MSAASamples;
			depthAttachment.p_LoadOp = LoadOp::Clear;
			depthAttachment.p_StoreOp = StoreOp::Null;
			depthAttachment.p_StencilLoadOp = LoadOp::Null;
			depthAttachment.p_StencilStoreOp = StoreOp::Null;
			depthAttachment.p_InitialLayout = Layout::Undefined;
			depthAttachment.p_FinalLayout = Layout::Depth;

			m_DefaultRenderPass.addAttachment(depthAttachment);

			AttachmentInfo resolveAttachment = {};
			resolveAttachment.p_AttachmentType = AttachmentType::Resolve;
			resolveAttachment.p_Format = m_WindowPtr->m_Swapchain.m_SwapchainImageFormat;
			resolveAttachment.p_SampleCount = 1;
			resolveAttachment.p_LoadOp = LoadOp::Null;
			resolveAttachment.p_StoreOp = StoreOp::Store;
			resolveAttachment.p_StencilLoadOp = LoadOp::Null;
			resolveAttachment.p_StencilStoreOp = StoreOp::Null;
			resolveAttachment.p_InitialLayout = Layout::Undefined;
			resolveAttachment.p_FinalLayout = Layout::Present;
#ifdef VGL_IMGUI_VK_IMPL
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

			m_DefaultInheritanceInfo = {};
			m_DefaultInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			m_DefaultInheritanceInfo.renderPass = m_DefaultRenderPass.m_RenderPass;
			m_DefaultInheritanceInfo.framebuffer = VK_NULL_HANDLE;
			m_DefaultInheritanceInfo.subpass = 0;
		}

		void Renderer::createPostSwapchainRenderPass()
		{
			AttachmentInfo colorAttachment = {};
			colorAttachment.p_AttachmentType = AttachmentType::Color;
			colorAttachment.p_Format = m_WindowPtr->m_Swapchain.m_SwapchainImageFormat;
			colorAttachment.p_SampleCount = VK_SAMPLE_COUNT_1_BIT;// m_WindowPtr->m_MSAASamples;
			colorAttachment.p_LoadOp = LoadOp::Load;
			colorAttachment.p_StoreOp = StoreOp::Store;
			colorAttachment.p_StencilLoadOp = LoadOp::Null;
			colorAttachment.p_StencilStoreOp = StoreOp::Null;
			colorAttachment.p_InitialLayout = Layout::Color;
			colorAttachment.p_FinalLayout = Layout::Present;

			m_PostSwapchainRenderPass.addAttachment(colorAttachment);

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;// VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			m_PostSwapchainRenderPass.m_Dependencies.push_back(dependency);

			m_PostSwapchainRenderPass.create();
		}

		// For screen buffer
		void Renderer::createDepthImageAttachment()
		{
			ImageAttachmentInfo info;
			info.p_Size = m_WindowPtr->m_WindowSize;
			info.p_AttachmentInfo = &m_DefaultRenderPass.m_AttachmentInfo[1];
			info.p_CreateSampler = false;

			m_DepthImageAttachment.create(info);
		}

		void Renderer::destroySwapChain()
		{
			m_WindowPtr->m_Swapchain.destroy();

			m_WindowPtr->destroyMSAAColorImage();

			m_DepthImageAttachment.destroy();
		}
		void Renderer::recreateSwapChain()
		{
			waitForFences();

			if (m_WindowPtr->getWindowSize().x <= 1)
			{
				glfwWaitEvents();
				m_WindowPtr->setWindowSize(2, m_WindowPtr->getWindowSize().y);
			}
			if (m_WindowPtr->getWindowSize().y <= 1)
			{
				glfwWaitEvents();
				m_WindowPtr->setWindowSize(m_WindowPtr->getWindowSize().x, 2);
			}

			m_Viewport.m_Size = m_WindowPtr->getWindowSize();

			destroySwapChain();

			m_WindowPtr->m_Swapchain.initSwapchain(m_WindowPtr->getWindowSize());

			m_WindowPtr->createColorResources();
			createDepthImageAttachment();

			m_WindowPtr->m_Swapchain.initFramebuffers(m_DefaultRenderPass.m_RenderPass, m_DepthImageAttachment.m_ImageView, m_WindowPtr->m_ColorImageView);
			m_PresentInfo.pSwapchains = &m_WindowPtr->m_Swapchain.m_Swapchain;

			m_ContextPtr->m_SwapchainImageCount = m_WindowPtr->m_Swapchain.m_SwapchainFramebuffers.size();

			m_WindowPtr->m_DeltaTime = 0;
			m_WindowPtr->m_PreviousTime = Utils::Logger::getTimePoint();
#ifdef VGL_IMGUI_VK_IMPL
			m_ImGuiContext.recreateSwapchain();
#endif

		}

		void Renderer::waitForFences()
		{
			vkWaitForFences(m_ContextPtr->m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		}

		void Renderer::createSyncObjects()
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
					vkCreateFence(m_ContextPtr->m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS
				){
#ifdef USING_LOGGING
					VGL_LOG_MSG("Failed to create synchronization objects for frame", "Render Sync Objects", Utils::Severity::Error);
#endif
				}
			}
		}
	}
}
