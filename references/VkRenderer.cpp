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

		Renderer::Renderer() : BaseRenderer() 
		{ 
			GraphicsContext::init();

			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				for (int32_t j = 0; j < 1000; j++)
					m_CommandBuffers[i].emplace_back(i, Level::Secondary);
		}
		Renderer::Renderer(Window* p_Window) : BaseRenderer(p_Window) 
		{
			GraphicsContext::init();

			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				for (int32_t j = 0; j < 1000; j++)
					m_CommandBuffers[i].emplace_back(i, Level::Secondary);
		}
		Renderer::Renderer(Window& p_Window) : BaseRenderer(p_Window) 
		{
			GraphicsContext::init();

			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				for (int32_t j = 0; j < 1000; j++)
					m_CommandBuffers[i].emplace_back(i, Level::Secondary);
		}
		Renderer::Renderer(Window& p_Window, float p_RenderResolutionScale) : BaseRenderer(p_Window, p_RenderResolutionScale) 
		{ 
			GraphicsContext::init();
			m_CommandBuffers.resize(m_ContextPtr->m_SwapchainImageCount);

			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				for (int32_t j = 0; j < 1000; j++)
					m_CommandBuffers[i].emplace_back(i, Level::Secondary);
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
		void Renderer::beginRenderPass(RenderInfo& p_RenderInfo, D_ShadowMap& p_ShadowMap)
		{
			p_ShadowMap.m_CommandBufferIdx = 0;

			m_RenderInfo = p_RenderInfo;

			m_FramebufferAttachmentPtr = &p_ShadowMap.m_Attachment;
			m_DShadowMapPtr = &p_ShadowMap;

			m_RecordMeshDataFun = &Renderer::shadowMapRecordMeshData;

			m_RecordPrimaryCmdBuffersFunPtrs.emplace_back(&p_ShadowMap, &Renderer::primaryShadowRecFun);
		}
		void Renderer::submit(Viewport& p_Viewport)
		{

		}
		void Renderer::submit(D_ShadowMap& p_ShadowMap)
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
				m_DescriptorSetInfo.p_ImageDescriptors.emplace_back(i, &p_Images[i].getImage(), 0);
		}
		void Renderer::submit(MeshData& p_MeshData, const uint32_t p_MTLIndex, Transform3D& p_Transform)
		{

		}
		void Renderer::submit(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			(this->*(m_RecordMeshDataFun))(p_MeshData, p_Transform);
		}
		void Renderer::submit(Particle_Emitter& p_Particle_Emitter)
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
		}
		void Renderer::submit(EnvData& p_EnvironmentData)
		{
			m_EnvData.p_BRDFLut = p_EnvironmentData.p_BRDFLut;
			m_EnvData.p_IrradianceMap = p_EnvironmentData.p_IrradianceMap;
			m_EnvData.p_PrefilteredMap = p_EnvironmentData.p_PrefilteredMap;
		}
		void Renderer::submit(CubeMap& p_IrradianceMap, CubeMap& p_PrefilteredMap, Image& p_BRDFLut)
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
		void Renderer::f_PrimaryCmdRec(CommandBuffer& p_PrimaryCommandBuffer, uint32_t p_ImageIndex)
		{
			//if(!m_DescriptorSetManager.isComplete())
			//	m_DescriptorSetManager.create(m_DescriptorSetInfo);
			//m_DescriptorSetInfo.clearDescriptors();

			for (auto& cmdFun : m_RecordPrimaryCmdBuffersFunPtrs)
				(this->*cmdFun.pcmdRecFun)(cmdFun.ptr, p_PrimaryCommandBuffer, p_ImageIndex);

			if (p_ImageIndex == m_ContextPtr->m_SwapchainImageCount - 1) {
				m_RecordPrimaryCmdBuffersFunPtrs.clear();
				m_CommandBufferIdx = 0;
			}
		}
		void Renderer::shadowMapRecordMeshData(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			DescriptorSetManager& descriptor = m_DShadowMapPtr->m_Descriptors[m_DShadowMapPtr->m_CommandBufferIdx];

			if (!descriptor.isComplete())
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
		void Renderer::recordMeshData(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			waitForFences();

			if (p_MeshData.newRT || p_MeshData.recreate) {
				if (p_MeshData.recreate) {
					for (auto& mtl : p_MeshData.m_MtlUniforms)
						if (mtl.isComplete()) {
							m_ContextPtr->deviceWaitIdle();
							mtl.destroy();
						}
				}

				if (!p_MeshData.m_MTLCompleted || p_MeshData.recreate || p_MeshData.newRT)
					p_MeshData.initMaterials(m_EnvData);

				createPipelines(p_MeshData, m_RenderInfo, m_FramebufferAttachmentPtr->m_RenderPass);

				p_MeshData.recreate = false;
				p_MeshData.newRT = false;
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
					p_MeshData.m_MtlUniforms[i].copy(ShaderStage::FragmentBit, Vector4f(
						p_MeshData.m_Materials[i].config.m_Albedo.r,
						p_MeshData.m_Materials[i].config.m_Albedo.g,
						p_MeshData.m_Materials[i].config.m_Albedo.b,
						p_MeshData.m_Materials[i].config.m_PDMDepth), 0);
					p_MeshData.m_MtlUniforms[i].copy(ShaderStage::FragmentBit,
						Vector4f(
							p_MeshData.m_Materials[i].config.m_Metallic,
							p_MeshData.m_Materials[i].config.m_Roughness,
							p_MeshData.m_Materials[i].config.m_Ambient, lumaThreshold
						), 4 * sizeof(float)
					);
				//}

				p_MeshData.m_MtlUniforms[i].copy(ShaderStage::FragmentBit, Vector3f(3, 3, 3), 12 * sizeof(float));
				for(int k = 0; k < p_MeshData.m_Materials.size(); k++)
					for(int j = 0; j < p_MeshData.m_Materials[k].m_ShaderInfo.p_PointLights.size(); j++)
						p_MeshData.m_MtlUniforms[i].copy(ShaderStage::FragmentBit, p_MeshData.m_Materials[k].m_ShaderInfo.p_PointLights[j], 16 * sizeof(float) + sizeof(P_Light)*j);

				// MVP Matrix
				p_MeshData.m_MtlUniforms[i].copy(ShaderStage::VertexBit, p_Transform.model * m_ViewProjection, MeshUBOAlignment::mvp_offset);
				// Model Matrix
				p_MeshData.m_MtlUniforms[i].copy(ShaderStage::VertexBit, p_Transform.model, MeshUBOAlignment::model_offset);

				// Camera position
				p_MeshData.m_MtlUniforms[i].copy(ShaderStage::VertexBit, m_Camera->getPosition(), MeshUBOAlignment::viewPosition_offset + sizeof(Matrix4f));

				p_MeshData.m_Materials[i].m_PrevConfig = p_MeshData.m_Materials[i].config;
			}

			m_CommandBufferIdx++;
		}
		void Renderer::recordMeshDataSkybox(MeshData& p_MeshData, Transform3D& p_Transform)
		{
			for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
				m_CommandBuffers[i][m_CommandBufferIdx].cmdBegin(m_FramebufferAttachmentPtr->m_InheritanceInfo[0]);
				setCmdDefautlViewport(m_CommandBuffers[i][m_CommandBufferIdx]);
			
				if(m_AtmosphericScatteringInfoPtr)
					m_CommandBuffers[i][m_CommandBufferIdx].cmdBindPipeline(GraphicsContext::getAtmosphericScatteringPipeline());
				else 
					m_CommandBuffers[i][m_CommandBufferIdx].cmdBindPipeline(GraphicsContext::getSkyBoxPipeline());

				m_CommandBuffers[i][m_CommandBufferIdx].cmdBindDescriptorSets(p_MeshData.m_MtlUniforms[0]);
			
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
		}
		void Renderer::primaryShadowRecFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex)
		{
			D_ShadowMap* shadowPtr = (D_ShadowMap*)p_Ptr;

			if (shadowPtr)
				if (!shadowPtr->m_CommandBuffers.empty()) {
					shadowPtr->m_Attachment.cmdBeginRenderPass(p_PrimaryCommandBuffer, p_ImageIndex, SubpassContents::Secondary);
					p_PrimaryCommandBuffer.cmdExecuteCommands(shadowPtr->m_CommandBuffers[p_ImageIndex], shadowPtr->m_CommandBufferIdx);
					shadowPtr->m_Attachment.cmdEndRenderPass();
				}
		}
		void Renderer::blitImageFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex)
		{
			Image* p_Image = (Image*)p_Ptr;

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = p_Image->m_Image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = 1;

			int mipWidth = p_Image->m_Size.x;
			int mipHeight = p_Image->m_Size.y;

			m_ContextPtr->setImageLayout(p_PrimaryCommandBuffer.m_CommandBuffer, p_Image->m_Image, VK_IMAGE_ASPECT_COLOR_BIT, p_Image->m_CurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, p_Image->mipLevels);
			p_Image->m_CurrentLayout = p_Image->m_FinalLayout;

			for (int i = 1; i < p_Image->mipLevels; i++)
			{
				//m_ContextPtr->setImageLayout(p_PrimaryCommandBuffer.m_CommandBuffer, p_Image->m_Image, VK_IMAGE_ASPECT_COLOR_BIT, p_Image->m_InitialLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(p_PrimaryCommandBuffer.m_CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

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

				vkCmdBlitImage(p_PrimaryCommandBuffer.m_CommandBuffer, p_Image->m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, p_Image->m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(p_PrimaryCommandBuffer.m_CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

				if (mipWidth > 1) mipWidth /= 2;

				if (mipHeight > 1) mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = p_Image->mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(p_PrimaryCommandBuffer.m_CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}
		void Renderer::primaryRecFun(void* p_Ptr, CommandBuffer& p_PrimaryCommandBuffer, const uint32_t& p_ImageIndex)
		{
			FramebufferAttachment* framebufferPtr = (FramebufferAttachment*)p_Ptr;

			if (framebufferPtr) {
				if (framebufferPtr->m_FramebufferAttachmentInfo.p_CreateGraphicsPipeline)
					framebufferPtr->recordCmdBuffer(p_PrimaryCommandBuffer, p_ImageIndex);
				else if (!m_CommandBuffers.empty()) {
					framebufferPtr->cmdBeginRenderPass(p_PrimaryCommandBuffer, p_ImageIndex, SubpassContents::Secondary);
					p_PrimaryCommandBuffer.cmdExecuteCommands(m_CommandBuffers[p_ImageIndex], m_CommandBufferIdx);
					framebufferPtr->cmdEndRenderPass();
				}
			}
		}
	}
}