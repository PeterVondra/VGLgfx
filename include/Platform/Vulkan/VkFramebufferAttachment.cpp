#include "VkFramebufferAttachment.h"

namespace vgl
{
	namespace vk
	{
		void FramebufferAttachment::create()
		{
			////////////////////////////////////////////////////////
			// CREATE RENDERPASS
			////////////////////////////////////////////////////////

			// If no renderpass is provided in m_FramebufferAttachmentInfo then create one here
			if(!m_FramebufferAttachmentInfo.p_ShaderPackage.p_RenderPass){
				VkSubpassDependency dependency = {};

				bool flag = false;
				for(auto& attachment : m_FramebufferAttachmentInfo.p_AttachmentDescriptors){
					m_RenderPass.addAttachment(attachment.p_AttachmentInfo);

					// If depth will be read from in the future, layout needs to be transitioned (using subpass dependencies)
					if(attachment.p_Layout == Layout::DepthR){
						flag = true;
						dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
						dependency.dstSubpass = 0;
						dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
						dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
						m_RenderPass.m_Dependencies.push_back(dependency);
						
						dependency.srcSubpass = 0;
						dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
						dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
						dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
						dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
						m_RenderPass.m_Dependencies.push_back(dependency);
						break;
					}
				}
				// Else use the default subpass dependencies
				if(!flag){
					dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
					dependency.dstSubpass = 0;
					dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
					dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
					dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
					m_RenderPass.m_Dependencies.push_back(dependency);

					dependency.srcSubpass = 0;
					dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
					dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
					dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
					dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
					m_RenderPass.m_Dependencies.push_back(dependency);
				}

				m_RenderPass.create();

				m_FramebufferAttachmentInfo.p_ShaderPackage.p_RenderPass = &m_RenderPass;
			}

			////////////////////////////////////////////////////////
			// CREATE GRAPHICS PIPELINE
			////////////////////////////////////////////////////////

			if(m_FramebufferAttachmentInfo.p_PushConstantSize > 0) {
				m_FramebufferAttachmentInfo.p_ShaderPackage.p_GraphicsPipelineInfo.p_UsePushConstants = true;
				m_FramebufferAttachmentInfo.p_ShaderPackage.p_GraphicsPipelineInfo.p_PushConstantShaderStage = getShaderStageVkH(m_FramebufferAttachmentInfo.p_PushConstantShaderStage);
				m_FramebufferAttachmentInfo.p_ShaderPackage.p_GraphicsPipelineInfo.p_PushConstantSize = m_FramebufferAttachmentInfo.p_PushConstantSize;
			}
			m_FramebufferAttachmentInfo.p_ShaderPackage.p_GraphicsPipelineInfo.p_RenderPass = m_FramebufferAttachmentInfo.p_ShaderPackage.p_RenderPass;
			m_FramebufferAttachmentInfo.p_ShaderPackage.p_GraphicsPipelineInfo.p_Shader = m_FramebufferAttachmentInfo.p_ShaderPackage.p_Shader;

			if(m_FramebufferAttachmentInfo.p_ShaderPackage.p_CreateGraphicsPipeline)
				m_Pipeline.create(m_FramebufferAttachmentInfo.p_ShaderPackage.p_GraphicsPipelineInfo);

			for(uint16_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++){
				m_ImageAttachments[i].resize(m_FramebufferAttachmentInfo.p_AttachmentDescriptors.size());

				FramebufferInfo framebufferInfo;

				for(uint16_t j = 0; j < m_FramebufferAttachmentInfo.p_AttachmentDescriptors.size(); j++){
					ImageAttachmentInfo attachmentInfo;
					attachmentInfo.p_AttachmentInfo = &m_FramebufferAttachmentInfo.p_AttachmentDescriptors[j].p_AttachmentInfo;
					attachmentInfo.p_AllowMipMapping = m_FramebufferAttachmentInfo.p_AttachmentDescriptors[j].p_AllowMipMapping;
					attachmentInfo.p_CreateSampler = m_FramebufferAttachmentInfo.p_AttachmentDescriptors[j].m_CreateSampler;
					attachmentInfo.p_BorderColor = m_FramebufferAttachmentInfo.p_AttachmentDescriptors[j].p_BorderColor;
					attachmentInfo.p_SamplerMode = m_FramebufferAttachmentInfo.p_AttachmentDescriptors[j].p_SamplerMode;
					attachmentInfo.p_Size = m_FramebufferAttachmentInfo.p_Size;

					m_ImageAttachments[i][j].create(attachmentInfo);

					if(m_FramebufferAttachmentInfo.p_AttachmentDescriptors[j].p_AllowMipMapping) 
						framebufferInfo.p_AllowMipMapping = true;
				}

				framebufferInfo.p_ImageAttachments = m_ImageAttachments[i];
				framebufferInfo.p_Size = m_FramebufferAttachmentInfo.p_Size;
				framebufferInfo.p_RenderPass = m_FramebufferAttachmentInfo.p_ShaderPackage.p_RenderPass;
				m_Framebuffer[i].create(framebufferInfo);
			}

			m_InheritanceInfo.resize(m_ContextPtr->m_SwapchainImageCount);
			for(uint16_t i = 0; i < m_InheritanceInfo.size(); i++){
				m_InheritanceInfo[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
				m_InheritanceInfo[i].renderPass = m_RenderPass.m_RenderPass;
				m_InheritanceInfo[i].framebuffer = m_Framebuffer[i].m_Framebuffer;
			}
		}

		DescriptorSetManager& FramebufferAttachment::getDescriptors()
		{
			return m_Descriptors;
		}
		std::vector<std::vector<ImageAttachment>>&  FramebufferAttachment::getImageAttachments()
		{
			return m_ImageAttachments;
		}
		std::vector<ImageAttachment>& FramebufferAttachment::getCurrentImageAttachments()
		{
			return m_ImageAttachments[m_ContextPtr->m_ImageIndex];
		}
		const std::vector<VkCommandBufferInheritanceInfo>& FramebufferAttachment::getInheritanceInfo()
		{
			return m_InheritanceInfo;
		}

		void FramebufferAttachment::destroy()
		{
			m_Pipeline.destroy();

			m_RenderPass.destroy();
			for(auto& frameBuf : m_Framebuffer)
				frameBuf.destroy();
			if (m_Descriptors.isValid())
				m_Descriptors.destroy();
			
			// For each swapchain image
			for(auto& attachments : m_ImageAttachments)
				for(auto& image : attachments)
					image.destroy();
		}

		void FramebufferAttachment::cmdBeginRenderPass(CommandBuffer& p_CommandBuffer, SubpassContents p_SubpassContents, const uint32_t p_ImageIndex) 
		{
			m_CurrentCommandBuffer = &p_CommandBuffer;
			m_CurrentCommandBuffer->cmdBeginRenderPass(
				*m_FramebufferAttachmentInfo.p_ShaderPackage.p_RenderPass,
				p_SubpassContents,
				m_Framebuffer[p_ImageIndex]
			);
		}
		void FramebufferAttachment::cmdEndRenderPass()
		{
			m_CurrentCommandBuffer->cmdEndRenderPass();
		}

		void FramebufferAttachment::recordCmdBuffer(CommandBuffer& p_CommandBuffer, const uint32_t p_ImageIndex)
		{			
			if (m_AllowMipMapping) {
				m_AttachmentBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
				m_AttachmentBeginInfo.attachmentCount = m_Framebuffer[p_ImageIndex].m_ImageViews.size();
				m_AttachmentBeginInfo.pAttachments = m_Framebuffer[p_ImageIndex].m_ImageViews.data();

				p_CommandBuffer.cmdBeginRenderPass(m_RenderPass, SubpassContents::Inline, m_Framebuffer[p_ImageIndex], &m_AttachmentBeginInfo);
			} else p_CommandBuffer.cmdBeginRenderPass(m_RenderPass, SubpassContents::Inline, m_Framebuffer[p_ImageIndex]);

			p_CommandBuffer.cmdSetViewport(Viewport({ m_FramebufferAttachmentInfo.p_Size.x, -m_FramebufferAttachmentInfo.p_Size.y }, { 0, m_FramebufferAttachmentInfo.p_Size.y }));
			p_CommandBuffer.cmdSetScissor(Scissor({ m_FramebufferAttachmentInfo.p_Size.x, m_FramebufferAttachmentInfo.p_Size.y }, { 0, 0 }));
			p_CommandBuffer.cmdBindPipeline(m_Pipeline);
			p_CommandBuffer.cmdBindVertexArray(m_RecVao);
			if(m_DescriptorSets.isValid())
				p_CommandBuffer.cmdBindDescriptorSets(m_DescriptorSets, p_ImageIndex);
			else if(m_FramebufferAttachmentInfo.p_Descriptors)
				p_CommandBuffer.cmdBindDescriptorSets(*m_FramebufferAttachmentInfo.p_Descriptors, p_ImageIndex);

			if(m_FramebufferAttachmentInfo.p_PushConstantSize > 0 && m_FramebufferAttachmentInfo.p_PushConstantData != nullptr)
				vkCmdPushConstants(
					p_CommandBuffer.m_CommandBuffer,
					m_Pipeline.m_PipelineLayout,
					getShaderStageVkH(m_FramebufferAttachmentInfo.p_PushConstantShaderStage),
					0,
					m_FramebufferAttachmentInfo.p_PushConstantSize,
					m_FramebufferAttachmentInfo.p_PushConstantData
				);

			p_CommandBuffer.cmdDrawIndexed(0, m_RecVao.getIndexCount());

			p_CommandBuffer.cmdEndRenderPass();
		}
	}
}
