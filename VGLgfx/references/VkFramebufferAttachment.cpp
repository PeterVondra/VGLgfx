#include "VkFramebufferAttachment.h"
#include "../../Utils/Logger.h"

namespace vgl
{
	namespace vk
	{
		void FramebufferAttachment::create(FramebufferAttachmentInfo& p_Info)
		{
			m_FramebufferAttachmentInfo = p_Info;

			// Subpass
			VkSubpassDependency dependency = {};
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
			dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			m_RenderPass.m_Dependencies.push_back(dependency);

			// Add attachments to renderpass
			for(auto& attachmentDescriptor : m_FramebufferAttachmentInfo.p_AttachmentDescriptors)
				m_RenderPass.addAttachment(attachmentDescriptor.getAttachmentInfo());

			m_RenderPass.create();

			// FramebufferCount == Swapchain Image Count
			for (int32_t i = 0; i < m_FramebufferAttachmentInfo.p_FramebufferCount; i++) {
				m_ImageAttachments[i].resize(m_FramebufferAttachmentInfo.p_AttachmentDescriptors.size());
				for (int32_t j = 0; j < m_FramebufferAttachmentInfo.p_AttachmentDescriptors.size(); j++) {
					ImageAttachmentDescriptor& attachmentDescriptor = m_FramebufferAttachmentInfo.p_AttachmentDescriptors[j];

					ImageAttachmentInfo imageInfo;
					imageInfo.p_AttachmentInfo = &attachmentDescriptor.getAttachmentInfo();
					imageInfo.p_SamplerMode = attachmentDescriptor.p_SamplerMode;
					imageInfo.p_BorderColor = attachmentDescriptor.p_BorderColor;
					imageInfo.p_CreateSampler = attachmentDescriptor.p_CreateSampler;
					imageInfo.p_AllowMipMapping = m_FramebufferAttachmentInfo.p_AllowMipMapping;
					imageInfo.p_Size = m_FramebufferAttachmentInfo.p_Size;

					m_ImageAttachments[i][j].create(imageInfo);
				}
			}

			m_Framebuffer.resize(m_FramebufferAttachmentInfo.p_FramebufferCount);
			m_InheritanceInfo.resize(m_FramebufferAttachmentInfo.p_FramebufferCount);
			m_FramebufferInfo.resize(m_FramebufferAttachmentInfo.p_FramebufferCount);

			// Create framebuffer for each swapchain image
			for (uint32_t i = 0; i < m_FramebufferAttachmentInfo.p_FramebufferCount; i++) {
				m_FramebufferInfo[i].p_RenderPass = &m_RenderPass;
				m_FramebufferInfo[i].p_Size = m_FramebufferAttachmentInfo.p_Size;
				m_FramebufferInfo[i].p_ImageAttachments = m_ImageAttachments[i];
				m_Framebuffer[i].create(m_FramebufferInfo[i]);
			}

			if (m_FramebufferAttachmentInfo.p_UseGraphicsPipeline) {
				m_GraphicsPipeline.resize(m_ContextPtr->m_SwapchainImageCount);
				m_FramebufferAttachmentInfo.p_GraphicsPipelineInfo.p_RenderPass = &m_RenderPass;
				m_FramebufferAttachmentInfo.p_GraphicsPipelineInfo.p_PushConstantSize = m_FramebufferAttachmentInfo.p_PushConstantSize;
				m_FramebufferAttachmentInfo.p_GraphicsPipelineInfo.p_PushConstantOffset = 0;

				for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
					if (m_Descriptors.isComplete())
						m_FramebufferAttachmentInfo.p_GraphicsPipelineInfo.p_DescriptorSetLayout = &m_Descriptors.m_DescriptorSetLayout[i];
					else if (m_FramebufferAttachmentInfo.p_DescriptorMananger)
						if (m_FramebufferAttachmentInfo.p_DescriptorMananger->isComplete())
							m_FramebufferAttachmentInfo.p_GraphicsPipelineInfo.p_DescriptorSetLayout = &m_FramebufferAttachmentInfo.p_DescriptorMananger->m_DescriptorSetLayout[i];

					m_GraphicsPipeline[i].create(m_FramebufferAttachmentInfo.p_GraphicsPipelineInfo);
				}
			}
		}

		const VkCommandBufferInheritanceInfo& FramebufferAttachment::getInheritanceInfo(const uint16_t p_ImageIndex) {
			return m_InheritanceInfo[p_ImageIndex];
		}

		void FramebufferAttachment::destroy()
		{
			for (auto& pipeline : m_GraphicsPipeline)
				pipeline.destroy();

			m_RenderPass.destroy();
			for(auto& frameBuf : m_Framebuffer)
				frameBuf.destroy();
			for(auto& pipeline : m_GraphicsPipeline)
				pipeline.destroy();
			if (m_Descriptors.isComplete())
				m_Descriptors.destroy();
			
			// For each swapchain image
			for(auto& attachments : m_ImageAttachments)
				for(auto& image : attachments)
					image.destroy();
		}
        
        void FramebufferAttachment::createColorFramebufferNoDepth()
		{
			AttachmentInfo color;
			color.p_AttachmentType = AttachmentType::Color;
			if(m_Info.p_ColorType == ColorType::Custom)
				color.p_Format = m_Info.p_CustomFormat;
			else if(m_Info.p_ColorType == ColorType::C16SF)
				color.p_Format = VK_FORMAT_R16G16B16A16_SFLOAT;
			else if(m_Info.p_ColorType == ColorType::C32SF)
				color.p_Format = VK_FORMAT_R32G32B32A32_SFLOAT;

			color.p_SampleCount = 1;
			color.p_LoadOp = LoadOp::Clear;
			color.p_StoreOp = StoreOp::Store;
			color.p_StencilLoadOp = LoadOp::Null;
			color.p_StencilStoreOp = StoreOp::Null;
			color.p_InitialLayout = Layout::Undefined;
			color.p_FinalLayout = Layout::ShaderR;

			m_RenderPass.addAttachment(color);

			ImageAttachmentInfo colorInfo;
			colorInfo.p_AttachmentInfo = &m_RenderPass.m_AttachmentInfos[0];
			colorInfo.p_SamplerMode = m_Info.p_SamplerMode;
			colorInfo.p_BorderColor = m_Info.p_BorderColor;
			colorInfo.p_CreateSampler = true;
			colorInfo.p_TransitionLayoutImage = false;
			colorInfo.p_AllowMipMapping = m_Info.p_AllowMipMapping;
			colorInfo.p_Size = m_Info.p_Size;

			m_Framebuffer.resize(m_Info.p_FramebufferCount);
			m_InheritanceInfo.resize(m_Info.p_FramebufferCount);
			m_FramebufferInfo.resize(m_Info.p_FramebufferCount);

			for(auto& attachment : m_ImageAttachment)
				attachment.create(colorInfo);

			for (uint32_t i = 0; i < m_Info.p_FramebufferCount; i++) {
				m_FramebufferInfo[i].p_RenderPass = &m_RenderPass;
				m_FramebufferInfo[i].p_Size = m_Info.p_Size;
				m_FramebufferInfo[i].p_ImageAttachments = { m_ImageAttachment[i] };
				m_Framebuffer[i].create(m_FramebufferInfo[i]);
			}

			for (auto& inheritanceinfo : m_InheritanceInfo) {
				inheritanceinfo = {};
				inheritanceinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
				inheritanceinfo.renderPass = m_RenderPass.m_RenderPass;
				inheritanceinfo.framebuffer = VK_NULL_HANDLE;
				inheritanceinfo.subpass = 0;
			}
		}
		void FramebufferAttachment::createColorFramebuffer()
		{
			AttachmentInfo color;
			color.p_AttachmentType = AttachmentType::Color;
			color.p_Format = VK_FORMAT_R32G32B32A32_SFLOAT;
			color.p_SampleCount = 1;
			color.p_LoadOp = LoadOp::Clear;
			color.p_StoreOp = StoreOp::Store;
			color.p_StencilLoadOp = LoadOp::Null;
			color.p_StencilStoreOp = StoreOp::Null;
			color.p_InitialLayout = Layout::Undefined;
			color.p_FinalLayout = Layout::ShaderR;

			AttachmentInfo colorDiscarded;
			colorDiscarded.p_AttachmentType = AttachmentType::Color;
			colorDiscarded.p_Format = VK_FORMAT_R32_SFLOAT;
			colorDiscarded.p_LoadOp = LoadOp::Load;
			colorDiscarded.p_StoreOp = StoreOp::Store;
			colorDiscarded.p_SampleCount = 1;
			colorDiscarded.p_StencilLoadOp = LoadOp::Null;
			colorDiscarded.p_StencilStoreOp = StoreOp::Null;
			colorDiscarded.p_InitialLayout = Layout::ShaderR;
			colorDiscarded.p_FinalLayout = Layout::ShaderR;

			AttachmentInfo depth;
			depth.p_AttachmentType = AttachmentType::Depth;
			depth.p_Format = m_ContextPtr->findDepthFormat();
			depth.p_LoadOp = LoadOp::Clear;
			depth.p_SampleCount = 1;
			depth.p_StoreOp = StoreOp::Null;
			depth.p_StencilLoadOp = LoadOp::Null;
			depth.p_StencilStoreOp = StoreOp::Null;
			depth.p_InitialLayout = Layout::Undefined;
			depth.p_FinalLayout = Layout::Depth;

			m_RenderPass.addAttachment(color);
			m_RenderPass.addAttachment(colorDiscarded);
			m_RenderPass.addAttachment(depth);

			VkSubpassDependency dependency = {};
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
			dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			m_RenderPass.m_Dependencies.push_back(dependency);

			m_RenderPass.create();

			ImageAttachmentInfo colorInfo;
			colorInfo.p_AttachmentInfo = &m_RenderPass.m_AttachmentInfos[0];
			colorInfo.p_SamplerMode = m_Info.p_SamplerMode;
			colorInfo.p_BorderColor = m_Info.p_BorderColor;
			colorInfo.p_CreateSampler = true;
			colorInfo.p_TransitionLayoutImage = false;
			colorInfo.p_AllowMipMapping = m_Info.p_AllowMipMapping;
			colorInfo.p_Size = m_Info.p_Size;

			ImageAttachmentInfo colorDiscardedInfo;
			colorDiscardedInfo.p_AttachmentInfo = &m_RenderPass.m_AttachmentInfos[1];
			colorDiscardedInfo.p_SamplerMode = m_Info.p_SamplerMode;
			colorDiscardedInfo.p_BorderColor = m_Info.p_BorderColor;
			colorDiscardedInfo.p_CreateSampler = true;
			colorDiscardedInfo.p_TransitionLayoutImage = false;
			colorDiscardedInfo.p_Size = m_Info.p_Size;

			ImageAttachmentInfo depthInfo;
			depthInfo.p_AttachmentInfo = &m_RenderPass.m_AttachmentInfos[2];
			depthInfo.p_CreateSampler = false;
			depthInfo.p_TransitionLayoutImage = false;
			depthInfo.p_Size = m_Info.p_Size;

			m_ImageAttachment.resize(m_Info.p_FramebufferCount);
			m_ColorImgDiscarded.resize(m_Info.p_FramebufferCount);
			m_DepthImg.resize(m_Info.p_FramebufferCount);
			m_Framebuffer.resize(m_Info.p_FramebufferCount);
			m_InheritanceInfo.resize(m_Info.p_FramebufferCount);
			m_FramebufferInfo.resize(m_Info.p_FramebufferCount);

			for(auto& attachment : m_ImageAttachment)
				attachment.create(colorInfo);
			for(auto& img : m_ColorImgDiscarded)
				img.create(colorDiscardedInfo);
			for(auto& img : m_DepthImg)
				img.create(depthInfo);

			for (int i = 0; i < m_Info.p_FramebufferCount; i++) {
				m_FramebufferInfo[i].p_ImageAttachments = { m_ImageAttachment[i], m_ColorImgDiscarded[i], m_DepthImg[i] };
				m_FramebufferInfo[i].p_RenderPass = &m_RenderPass;
				m_FramebufferInfo[i].p_Size = m_Info.p_Size;
				m_Framebuffer[i].create(m_FramebufferInfo[i]);
			}

			for (auto& inheritanceinfo : m_InheritanceInfo) {
				inheritanceinfo = {};
				inheritanceinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
				inheritanceinfo.renderPass = m_RenderPass.m_RenderPass;
				inheritanceinfo.framebuffer = VK_NULL_HANDLE;
				inheritanceinfo.subpass = 0;
			}
		}
		void FramebufferAttachment::createDepthFramebuffer()
		{
			AttachmentInfo colorAttachment;
			colorAttachment.p_AttachmentType = AttachmentType::Depth;
			colorAttachment.p_Format = VK_FORMAT_D16_UNORM;
			colorAttachment.p_SampleCount = 1;
			colorAttachment.p_LoadOp = LoadOp::Clear;
			colorAttachment.p_StoreOp = StoreOp::Store;
			colorAttachment.p_StencilLoadOp = LoadOp::Null;
			colorAttachment.p_StencilStoreOp = StoreOp::Null;
			colorAttachment.p_InitialLayout = Layout::Undefined;
			colorAttachment.p_FinalLayout = Layout::DepthR;

			m_RenderPass.addAttachment(colorAttachment);

			VkSubpassDependency dependency = {};
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

			m_RenderPass.create();

			ImageAttachmentInfo info;
			info.p_AttachmentInfo = &m_RenderPass.m_AttachmentInfos[0];
			info.p_BorderColor = m_Info.p_BorderColor;
			info.p_CreateSampler = true;
			info.p_TransitionLayoutImage = false;
			info.p_SamplerMode = m_Info.p_SamplerMode;
			info.p_Size = Vector2i(m_Info.p_Size.x, m_Info.p_Size.y);

			m_ImageAttachment.resize(m_Info.p_FramebufferCount);
			m_Framebuffer.resize(m_Info.p_FramebufferCount);
			m_InheritanceInfo.resize(m_Info.p_FramebufferCount);
			m_FramebufferInfo.resize(m_Info.p_FramebufferCount);

			for(auto& attachment : m_ImageAttachment)
				attachment.create(info);

			for (uint32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
				m_FramebufferInfo[i].p_RenderPass = &m_RenderPass;
				m_FramebufferInfo[i].p_Size = m_Info.p_Size;
				m_FramebufferInfo[i].p_ImageAttachments = { m_ImageAttachment[i] };
				m_Framebuffer[i].create(m_FramebufferInfo[i]);
			}

			for (auto& inheritanceinfo : m_InheritanceInfo) {
				inheritanceinfo = {};
				inheritanceinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
				inheritanceinfo.renderPass = m_RenderPass.m_RenderPass;
				inheritanceinfo.framebuffer = VK_NULL_HANDLE;
				inheritanceinfo.subpass = 0;
			}
		}

		void FramebufferAttachment::cmdBeginRenderPass(CommandBuffer& p_CommandBuffer, const uint16_t p_ImageIndex, SubpassContents p_SubpassContents) {
			m_CurrentCommandBuffer = &p_CommandBuffer;
			if (m_FramebufferAttachmentInfo.p_AllowMipMapping) {
				VkRenderPassAttachmentBeginInfo attachmentBeginInfo = {};
				attachmentBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
				attachmentBeginInfo.attachmentCount = m_Framebuffer[p_ImageIndex].m_Attachments.size();
				attachmentBeginInfo.pAttachments = m_Framebuffer[p_ImageIndex].m_Attachments.data();
				m_CurrentCommandBuffer->cmdBeginRenderPass(m_RenderPass, p_SubpassContents, m_Framebuffer[p_ImageIndex], &attachmentBeginInfo);
			}else m_CurrentCommandBuffer->cmdBeginRenderPass(m_RenderPass, p_SubpassContents, m_Framebuffer[p_ImageIndex]);
		}
		void FramebufferAttachment::cmdEndRenderPass() {
			m_CurrentCommandBuffer->cmdEndRenderPass();
		}

		void FramebufferAttachment::recordCmdBuffer(CommandBuffer& p_CommandBuffer, const uint16_t p_ImageIndex)
		{
			if (m_FramebufferAttachmentInfo.p_AllowMipMapping) {
				VkRenderPassAttachmentBeginInfo attachmentBeginInfo = {};
				attachmentBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
				attachmentBeginInfo.attachmentCount = m_Framebuffer[p_ImageIndex].m_Attachments.size();
				attachmentBeginInfo.pAttachments = m_Framebuffer[p_ImageIndex].m_Attachments.data();

				p_CommandBuffer.cmdBeginRenderPass(m_RenderPass, SubpassContents::Inline, m_Framebuffer[p_ImageIndex], &attachmentBeginInfo);
			} else p_CommandBuffer.cmdBeginRenderPass(m_RenderPass, SubpassContents::Inline, m_Framebuffer[p_ImageIndex]);

			p_CommandBuffer.cmdSetViewport(Viewport({ m_FramebufferAttachmentInfo.p_Size.x, -m_FramebufferAttachmentInfo.p_Size.y }, { 0, m_FramebufferAttachmentInfo.p_Size.y }));
			p_CommandBuffer.cmdSetScissor(Scissor({ m_FramebufferAttachmentInfo.p_Size.x, m_FramebufferAttachmentInfo.p_Size.y }, { 0, 0 }));
			p_CommandBuffer.cmdBindPipeline(m_GraphicsPipeline[p_ImageIndex]);
			p_CommandBuffer.cmdBindVertexArray(*m_FramebufferAttachmentInfo.p_RectangleData);
			if(m_Descriptors.isComplete())
				p_CommandBuffer.cmdBindDescriptorSets(m_Descriptors);
			else if(m_FramebufferAttachmentInfo.p_DescriptorMananger)
				p_CommandBuffer.cmdBindDescriptorSets(*m_FramebufferAttachmentInfo.p_DescriptorMananger);

			if(m_FramebufferAttachmentInfo.p_PushConstantSize > 0 && m_FramebufferAttachmentInfo.p_PushConstantData)
				vkCmdPushConstants(
					p_CommandBuffer.m_CommandBuffer,
					m_GraphicsPipeline[p_ImageIndex].m_PipelineLayout,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					m_FramebufferAttachmentInfo.p_PushConstantSize,
					m_FramebufferAttachmentInfo.p_PushConstantData
				);

			p_CommandBuffer.cmdDrawIndexed(0, m_FramebufferAttachmentInfo.p_RectangleData->getIndexCount());

			p_CommandBuffer.cmdEndRenderPass();
		}
	}
}
