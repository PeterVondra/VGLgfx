#include "VkRenderer.h"

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

		Renderer::Renderer() : m_ContextPtr(&ContextSingleton::getInstance()), m_DefaultRenderPass(RenderPassType::Graphics), m_ImGuiContext(nullptr)
		{ 
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
		}
		Renderer::Renderer(Window* p_Window) 
			: m_ContextPtr(&ContextSingleton::getInstance()), m_DefaultRenderPass(RenderPassType::Graphics), m_ImGuiContext(p_Window)
		{
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

			m_ImGuiContext.init(m_DefaultRenderPass);
		}
		Renderer::Renderer(Window& p_Window) 
			: m_ContextPtr(&ContextSingleton::getInstance()), m_DefaultRenderPass(RenderPassType::Graphics), m_ImGuiContext(&p_Window)
		{
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

			m_ImGuiContext.init(m_DefaultRenderPass);
		}
		Renderer::Renderer(Window& p_Window, float p_RenderResolutionScale) 
			: m_ContextPtr(&ContextSingleton::getInstance()), m_DefaultRenderPass(RenderPassType::Graphics), m_ImGuiContext(&p_Window)
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

			m_ImGuiContext.init(m_DefaultRenderPass);
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

			m_RecordPrimaryCmdBuffersFunPtrs.emplace_back(&p_FramebufferAttachment, &Renderer::primaryRecFun);
		}
		/*void Renderer::beginRenderPass(RenderInfo& p_RenderInfo, D_ShadowMap& p_ShadowMap)
		{
			p_ShadowMap.m_CommandBufferIdx = 0;

			m_RenderInfo = p_RenderInfo;

			m_FramebufferAttachmentPtr = &p_ShadowMap.m_Attachment;
			m_DShadowMapPtr = &p_ShadowMap;

			m_RecordMeshDataFun = &Renderer::shadowMapRecordMeshData;

			m_RecordPrimaryCmdBuffersFunPtrs.emplace_back(&p_ShadowMap, &Renderer::primaryShadowRecFun);
		}*/

		void Renderer::submit(Camera& p_Camera)
		{
			m_Camera = &p_Camera;
			m_Camera->setPerspectiveMatrix(m_Camera->getAspectRatio());
			m_Camera->setOrthographicMatrix(0, m_OrthoProjectionSize.x, 0, m_OrthoProjectionSize.y, 0.0f, 1.0f);
			m_FontProjection = Matrix4f::orthoRH_ZO(0, m_WindowPtr->getWindowSize().x, 0, m_WindowPtr->getWindowSize().y, 0.0f, 1.0f);
			m_ViewProjection = m_Camera->getViewMatrix() * m_Camera->getPerspectiveMatrix();
		}
		
		void Renderer::submit(Viewport& p_Viewport)
		{

		}
		/*void Renderer::submit(D_ShadowMap& p_ShadowMap)
		{
			m_DShadowMapPtr = &p_ShadowMap;
			m_EnvData.p_DShadowMap.resize(m_ContextPtr->m_SwapchainImageCount);
			for(int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				for(int32_t j = 0; j < p_ShadowMap.m_Attachment.m_ImageAttachments[i].size(); j++)
					if(p_ShadowMap.m_Attachment.m_ImageAttachments[i][j].info.p_AttachmentInfo->p_AttachmentType == AttachmentType::Depth)
						m_EnvData.p_DShadowMap[i] = &p_ShadowMap.m_Attachment.m_ImageAttachments[i][j].getImage();
		}
		void Renderer::submit(D_Light& p_LightSrc)
		{
		}
		void Renderer::submit(S_Light& p_LightSrc)
		{
		}
		void Renderer::submit(P_Light& p_LightSrc)
		{
		}*/
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
				m_DescriptorSetInfo.p_ImageDescriptors.emplace_back(i, &p_Images[i].getImage(), 0);
		}
		void Renderer::submit(MeshData& p_MeshData, const uint32_t p_MTLIndex, Transform3D& p_Transform)
		{

		}
		void Renderer::submit(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			(this->*(m_RecordMeshDataFun))(p_MeshData, p_Transform);
		}
		/*void Renderer::submit(Particle_Emitter& p_Particle_Emitter)
		{
			if (!p_Particle_Emitter.m_Vao.m_IsComplete)
				p_Particle_Emitter.m_Vao.fill({ &GraphicsContext::getRecVertexBuffer(), &p_Particle_Emitter.m_InstanceDataVBuf }, GraphicsContext::getRecIndexBuffer());

			for (int16_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
				CommandBuffer& cmd = m_CommandBuffers[i][m_CommandBufferIdx];

				cmd.cmdBegin(m_FramebufferAttachmentPtr->m_InheritanceInfo[0]);

				cmd.cmdSetViewport(
					Viewport({
						int(m_FramebufferAttachmentPtr->m_FramebufferAttachmentInfo.p_Size.x), int(-m_FramebufferAttachmentPtr->m_FramebufferAttachmentInfo.p_Size.y) },
						Vector2i(0, m_FramebufferAttachmentPtr->m_FramebufferAttachmentInfo.p_Size.y)
						)
				);
				cmd.cmdSetScissor(
					Scissor({
						int(m_FramebufferAttachmentPtr->m_FramebufferAttachmentInfo.p_Size.x),
						int(m_FramebufferAttachmentPtr->m_FramebufferAttachmentInfo.p_Size.y) }, { 0, 0 }
						)
				);

				cmd.cmdBindPipeline(GraphicsContext::m_ParticlePipeline);
				
				vkCmdPushConstants(
					cmd.m_CommandBuffer,
					GraphicsContext::m_ParticlePipeline.m_PipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT,
					0,
					sizeof(Matrix4f),
					&m_ViewProjection
				);

				cmd.cmdBindVertexArray(p_Particle_Emitter.m_Vao);
				cmd.cmdDrawIndexedInstanced(0, p_Particle_Emitter.m_Vao.getIndexCount(), p_Particle_Emitter.m_Particles.size());

				cmd.cmdEnd();
			}

			m_CommandBufferIdx++;
		}*/
		/*void Renderer::submit(EnvData& p_EnvironmentData)
		{
			m_EnvData.p_BRDFLut = p_EnvironmentData.p_BRDFLut;
			m_EnvData.p_IrradianceMap = p_EnvironmentData.p_IrradianceMap;
			m_EnvData.p_PrefilteredMap = p_EnvironmentData.p_PrefilteredMap;
		}*/
		/*void Renderer::submit(CubeMap& p_IrradianceMap, CubeMap& p_PrefilteredMap, Image& p_BRDFLut)
		{
			m_EnvData.p_BRDFLut = &p_BRDFLut;
			m_EnvData.p_IrradianceMap = &p_IrradianceMap;
			m_EnvData.p_PrefilteredMap = &p_PrefilteredMap;
		}
		void Renderer::submit(Skybox& p_Skybox)
		{
			recordMeshDataSkybox(p_Skybox, p_Skybox);
		}
		void Renderer::submit(Skybox& p_Skybox, AtmosphericScatteringInfo& p_Info)
		{
			m_AtmosphericScatteringInfoPtr = &p_Info;
			if(p_Skybox.m_CubeMap != nullptr)
				if (p_Skybox.m_CubeMap->isComplete())
					m_AtmosphericScatteringInfoPtr = nullptr;
			//m_AtmosphericScatteringInfoPtr->p_RayOrigin = m_Camera->getPosition();

			recordMeshDataSkybox(p_Skybox, p_Skybox);
		}*/
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
		/*void Renderer::shadowMapRecordMeshData(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			DescriptorSetManager& descriptor = m_DShadowMapPtr->m_Descriptors[m_DShadowMapPtr->m_CommandBufferIdx];

			if (!descriptor.isValid())
				descriptor.create(m_DShadowMapPtr->m_DescriptorInfo[m_DShadowMapPtr->m_CommandBufferIdx]);

			descriptor.copy(ShaderStage::VertexBit, p_Transform.model * m_DShadowMapPtr->m_View * m_DShadowMapPtr->m_Projection, 0);

			for (uint32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
				CommandBuffer& cmd = m_DShadowMapPtr->m_CommandBuffers[i][m_DShadowMapPtr->m_CommandBufferIdx];

				// Shadow map rendering
				cmd.cmdBegin(m_DShadowMapPtr->m_Attachment.m_InheritanceInfo[0]);

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

				cmd.cmdBindPipeline(GraphicsContext::getDShadowMapPipeline());
				cmd.cmdBindDescriptorSets(m_DShadowMapPtr->m_Descriptors[m_DShadowMapPtr->m_CommandBufferIdx]);

				vkCmdSetDepthBias
				(
					cmd.m_CommandBuffer,
					m_DShadowMapPtr->m_DepthBiasConstant,
					0.00f,
					m_DShadowMapPtr->m_DepthBiasSlope
				);

				cmd.cmdBindVertexArray(p_MeshData.m_VertexArray);
				uint32_t prevIndex = 0;
				for (int i = 0; i < p_MeshData.m_Materials.size(); i++) {
					// Should material be rendered?
					if (!p_MeshData.m_Materials[i].config.render) {
						cmd.cmdDrawIndexed(prevIndex, p_MeshData.m_MaterialIndices[i].first);
						prevIndex = p_MeshData.m_MaterialIndices[i].second;
					}
				}

				cmd.cmdDrawIndexed(prevIndex, p_MeshData.indices.size());

				cmd.cmdEnd();
			}

			m_DShadowMapPtr->m_CommandBufferIdx++;
		}
		*/
		
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

				pipelineInfo.p_DescriptorSetLayout = p_MeshData.m_MTLDescriptors[i].m_DescriptorSetLayout;
				
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

				if (!p_MeshData.m_MTLValid || p_MeshData.m_RecreateFlag || p_MeshData.m_MTLRecreateFlag)
					p_MeshData.initMaterials(m_EnvData);

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

				//if(!m_EnvData.p_DShadowMap.empty())
				//	if (m_EnvData.p_DShadowMap[0]->isComplete()) {
				//		p_MeshData.m_MtlUniforms[i].copy(ShaderStage::FragmentBit, m_DShadowMapPtr->m_Direction, 8 * sizeof(float));
				//		p_MeshData.m_MtlUniforms[i].copy(ShaderStage::VertexBit, p_Transform.model * (m_DShadowMapPtr->m_View * m_DShadowMapPtr->m_Projection), MeshUBOAlignment::viewPosition_offset);
				//	}

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
				p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::VertexBit, p_Transform.model * m_ViewProjection, MeshUBOAlignment::mvp_offset);
				// Model Matrix
				p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::VertexBit, p_Transform.model, MeshUBOAlignment::model_offset);

				// Camera position
				p_MeshData.m_MTLDescriptors[i].copy(ShaderStage::VertexBit, m_Camera->getPosition(), MeshUBOAlignment::viewPosition_offset + sizeof(Matrix4f));

				p_MeshData.m_Materials[i].m_PrevConfig = p_MeshData.m_Materials[i].config;
			}

			m_CommandBufferIdx++;
		}

		void Renderer::recordMeshCmdBuffer(MeshData& p_MeshData, FramebufferAttachment& p_FramebufferAttachment, CommandBuffer& p_CommandBuffer, const uint32_t p_ImageIndex)
		{
			p_CommandBuffer.cmdBegin(p_FramebufferAttachment.m_InheritanceInfo[p_ImageIndex]);
			setCmdDefaultViewport(p_CommandBuffer);

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

		/*void Renderer::recordMeshDataSkybox(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
				m_CommandBuffers[i][m_CommandBufferIdx].cmdBegin(m_FramebufferAttachmentPtr->m_InheritanceInfo[0]);
				setCmdDefaultViewport(m_CommandBuffers[i][m_CommandBufferIdx]);
			
				if(m_AtmosphericScatteringInfoPtr)
					m_CommandBuffers[i][m_CommandBufferIdx].cmdBindPipeline(GraphicsContext::getAtmosphericScatteringPipeline());
				else 
					m_CommandBuffers[i][m_CommandBufferIdx].cmdBindPipeline(GraphicsContext::getSkyBoxPipeline());

				m_CommandBuffers[i][m_CommandBufferIdx].cmdBindDescriptorSets(p_MeshData.m_MtlDescriptors[0], m_ImageIndex);
			
				if(m_AtmosphericScatteringInfoPtr)
					vkCmdPushConstants(
						m_CommandBuffers[i][m_CommandBufferIdx].m_CommandBuffer,
						GraphicsContext::getAtmosphericScatteringPipeline().m_PipelineLayout,
						getShaderStageVkH(ShaderStage::FragmentBit),
						0, sizeof(AtmosphericScatteringInfo), m_AtmosphericScatteringInfoPtr
					);

				m_CommandBuffers[i][m_CommandBufferIdx].cmdBindVertexArray(p_MeshData.m_VertexArray);
				m_CommandBuffers[i][m_CommandBufferIdx].cmdDrawIndexed(p_MeshData.m_MaterialIndices[0].first, p_MeshData.m_MaterialIndices[0].second);
			
				m_CommandBuffers[i][m_CommandBufferIdx].cmdEnd();
			
				Vector3f viewPos = m_Camera->getPosition();
			
				m_Camera->setPosition({ 0, 0, 0 });
				// MVP Matrix
				p_MeshData.m_MtlUniforms[0].copy(ShaderStage::VertexBit, p_Transform.model * m_Camera->getViewMatrix() * m_Camera->getPerspectiveMatrix(), 0);
				m_Camera->setPosition(viewPos);
				// Model Matrix
				p_MeshData.m_MtlUniforms[0].copy(ShaderStage::VertexBit, p_Transform.model, sizeof(Matrix4f));
			}
			
			m_CommandBufferIdx++;
		}*/
		/*void Renderer::primaryShadowRecFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex)
		{
			D_ShadowMap* shadowPtr = (D_ShadowMap*)p_Ptr;

			if (shadowPtr)
				if (!shadowPtr->m_CommandBuffers.empty()) {
					shadowPtr->m_Attachment.cmdBeginRenderPass(p_PrimaryCommandBuffer, SubpassContents::Secondary);
					p_PrimaryCommandBuffer.cmdExecuteCommands(shadowPtr->m_CommandBuffers[p_ImageIndex], shadowPtr->m_CommandBufferIdx);
					shadowPtr->m_Attachment.cmdEndRenderPass();
				}
		}*/
		
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

			m_ContextPtr->setImageLayout(
				p_PrimaryCommandBuffer.m_CommandBuffer,
				p_Image->m_VkImageHandle,
				VK_IMAGE_ASPECT_COLOR_BIT,
				p_Image->m_CurrentLayout,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			);
			
			p_Image->m_CurrentLayout = p_Image->m_FinalLayout;

			for (int i = 1; i < p_Image->m_MipLevels; i++)
			{
				//m_ContextPtr->setImageLayout(p_PrimaryCommandBuffer.m_CommandBuffer, p_Image->m_Image, VK_IMAGE_ASPECT_COLOR_BIT, p_Image->m_InitialLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

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
			m_PrimaryCommandBuffers[p_ImageIndex].cmdBegin();

			//if(!m_DescriptorSetManager.isComplete())
			//	m_DescriptorSetManager.create(m_DescriptorSetInfo);
			//m_DescriptorSetInfo.clearDescriptors();

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
				m_WindowPtr->getWindowSize()
			);
			m_PrimaryCommandBuffers[p_ImageIndex].cmdEndRenderPass();

#ifdef IMGUI_VK_IMPL
			m_PrimaryCommandBuffers[p_ImageIndex].cmdBeginRenderPass(
				*m_ImGuiContext.m_RenderPass, 
				SubpassContents::Secondary, 
				m_WindowPtr->m_Swapchain.m_SwapchainFramebuffers[p_ImageIndex], 
				m_WindowPtr->getWindowSize()
			);
			//m_PrimaryCommandBuffers[p_ImageIndex].cmdExecuteCommands(m_ImGuiContext.m_CommandBuffer);
			// Record Imgui Draw Data and draw funcs into command buffer
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_PrimaryCommandBuffers[p_ImageIndex].vkHandle());

			m_PrimaryCommandBuffers[p_ImageIndex].cmdEndRenderPass();
#endif
			m_PrimaryCommandBuffers[p_ImageIndex].cmdEnd();
		}
		void Renderer::primaryRecFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex)
		{
			FramebufferAttachment* framebufferPtr = (FramebufferAttachment*)p_Ptr;

			m_ContextPtr->m_ImageIndex = p_ImageIndex;

			if (framebufferPtr) {
				if (framebufferPtr->m_FramebufferAttachmentInfo.p_RenderPipelineInfo.p_CreateGraphicsPipeline)
					framebufferPtr->recordCmdBuffer(p_PrimaryCommandBuffer, p_ImageIndex);
				else if (!m_CommandBuffers.empty()) {
					framebufferPtr->cmdBeginRenderPass(p_PrimaryCommandBuffer, SubpassContents::Secondary, p_ImageIndex);
					p_PrimaryCommandBuffer.cmdExecuteCommands(m_CommandBuffers[m_ImageIndex], m_CommandBufferIdx);
					framebufferPtr->cmdEndRenderPass();
				}
			}
		}

		void Renderer::updateImGuiDrawData()
		{
			//#ifdef IMGUI_VK_IMPL
			//	m_ImGuiContext.genCmdBuffers();
			//#elif undefined IMGUI_VK_IMPL
			//	m_ImGuiContext.updateBuffers();
			//	m_ImGuiContext.genCmdBuffers(m_DefaultInheritanceInfo);
			//#endif
		}

		void Renderer::GPUSubmit()
		{
			//m_ThreadPool.wait();

			//float start = Utils::Logger::getTimePoint();
			
			//updateImGuiDrawData();

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
				Utils::Logger::logMSG("Failed to acquire swap chain image", "vkAcquireNextImageKHR", Utils::Severity::Error);

			m_SubmitInfo.pWaitSemaphores = &m_ImageAvailableSemaphore[m_CurrentFrame];
			m_SubmitInfo.pCommandBuffers = &m_PrimaryCommandBuffers[m_ImageIndex].m_CommandBuffer;
			m_SubmitInfo.pSignalSemaphores = &m_RenderFinishedSemaphore[m_CurrentFrame];

			vkResetFences(m_ContextPtr->m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

			// Submit the commands to the graphics queue
			if (vkQueueSubmit(m_ContextPtr->m_GraphicsQueue, 1, &m_SubmitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
				Utils::Logger::logMSG("Failed to submit command buffer", "vkQueueSubmit", Utils::Severity::Error);

			m_PresentInfo.pWaitSemaphores = &m_RenderFinishedSemaphore[m_CurrentFrame];
			m_PresentInfo.pImageIndices = &m_ImageIndex;

			result = vkQueuePresentKHR(m_ContextPtr->m_PresentQueue, &m_PresentInfo);

			// If swapchain needs recreation
			if (result == VK_ERROR_OUT_OF_DATE_KHR || m_WindowPtr->windowResized())
				recreateSwapChain();

			else if (result != VK_SUCCESS)
				Utils::Logger::logMSG("Failed to present swap chain image", "vkQueuePresentKHR", Utils::Severity::Error);

			//waitForFences();
			//std::cout << 1000 * (Utils::Logger::getTimePoint() - start) << std::endl;

			m_CurrentFrame = GET_CURRENT_IMAGE_INDEX(m_CurrentFrame);

			m_ContextPtr->m_ImageIndex = m_ImageIndex;

			vkWaitForFences(m_ContextPtr->m_Device, 1, &m_ImagesInFlight[m_ImageIndex], VK_TRUE, UINT64_MAX);
			for (size_t i = 0; i < m_PrimaryCommandBuffers.size(); i++)
				vkResetCommandPool(m_ContextPtr->m_Device, m_ContextPtr->m_DefaultCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		}

		void Renderer::createDefaultRenderPass()
		{
			AttachmentInfo colorAttachment;
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

			AttachmentInfo depthAttachment;
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

			AttachmentInfo resolveAttachment;
			resolveAttachment.p_AttachmentType = AttachmentType::Resolve;
			resolveAttachment.p_Format = m_WindowPtr->m_Swapchain.m_SwapchainImageFormat;
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

			m_DefaultInheritanceInfo = {};
			m_DefaultInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			m_DefaultInheritanceInfo.renderPass = m_DefaultRenderPass.m_RenderPass;
			m_DefaultInheritanceInfo.framebuffer = VK_NULL_HANDLE;
			m_DefaultInheritanceInfo.subpass = 0;
		}

		// For screen buffer
		void Renderer::createDepthImageAttachment()
		{
			ImageAttachmentInfo info;
			info.p_Size = m_WindowPtr->m_WindowSize;
			info.p_AttachmentInfo = &m_DefaultRenderPass.m_AttachmentInfos[1];

			m_DepthImageAttachment.create(info);
		}

		void Renderer::destroySwapChain()
		{
			m_WindowPtr->m_Swapchain.destroy();

			m_DefaultRenderPass.destroy();

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

			destroySwapChain();

			m_WindowPtr->m_Swapchain.initSwapchain(m_WindowPtr->getWindowSize());

			createDefaultRenderPass();
			m_WindowPtr->createColorResources();
			createDepthImageAttachment();

			m_WindowPtr->m_Swapchain.initFramebuffers(m_DefaultRenderPass.m_RenderPass, m_DepthImageAttachment.m_ImageView, m_WindowPtr->m_ColorImageView);

			m_WindowPtr->m_DeltaTime = 0;
			m_WindowPtr->m_PreviousTime = Utils::Logger::getTimePoint();
#ifdef IMGUI_VK_IMPL
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
					Utils::Logger::logMSG("Failed to create synchronization objects for frame", "Render Sync Objects", Utils::Severity::Error);
#endif
				}
			}
		}
	}
}