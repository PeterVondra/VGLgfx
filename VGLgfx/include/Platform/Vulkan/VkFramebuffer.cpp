#include "VkFramebuffer.h"
#include "../../VGL_Logger.h"

#undef max
#undef min

namespace vgl
{
	namespace vk
	{
		Framebuffer::Framebuffer() : m_ContextPtr(&ContextSingleton::getInstance())
		{

		}

		Framebuffer::~Framebuffer()
		{

		}

		bool Framebuffer::destroy()
		{
			vkDestroyFramebuffer(m_ContextPtr->m_Device, m_Framebuffer, nullptr);
			return true;
		}

		bool Framebuffer::create(FramebufferInfo& p_FramebufferInfo)
		{
			m_Size = p_FramebufferInfo.p_Size;

			for (ImageAttachment& imageAttachment : p_FramebufferInfo.p_ImageAttachments){
				m_ImageViews.push_back(imageAttachment.m_ImageViewAttachment);
				if(p_FramebufferInfo.p_AllowMipMapping)
					m_AttachmentImageInfos.push_back(imageAttachment.m_AttachmentImageInfo);
			}

			m_RenderPassPtr = p_FramebufferInfo.p_RenderPass;

			m_FrameBufferCreateInfo = {};
			m_FrameBufferCreateInfo.pNext = nullptr;
			
			VkFramebufferAttachmentsCreateInfo attachmentCreateInfo = {};
			if (p_FramebufferInfo.p_AllowMipMapping) {
				attachmentCreateInfo.attachmentImageInfoCount = m_ImageViews.size();
				attachmentCreateInfo.pAttachmentImageInfos = m_AttachmentImageInfos.data();
				attachmentCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO;

				m_FrameBufferCreateInfo.pNext = &attachmentCreateInfo;
			}
			m_FrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			m_FrameBufferCreateInfo.renderPass = m_RenderPassPtr->m_RenderPass;

			m_FrameBufferCreateInfo.attachmentCount = m_ImageViews.size();
			if(!p_FramebufferInfo.p_AllowMipMapping){
				m_FrameBufferCreateInfo.pAttachments = m_ImageViews.data();
			}else{
				m_FrameBufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
			}

			m_FrameBufferCreateInfo.width = m_Size.x;
			m_FrameBufferCreateInfo.height = m_Size.y;
			m_FrameBufferCreateInfo.layers = 1;

			if (vkCreateFramebuffer(m_ContextPtr->m_Device, &m_FrameBufferCreateInfo, nullptr, &m_Framebuffer) != VK_SUCCESS){
				VGL_LOG_MSG("Failed to create framebuffer", "VkFramebuffer", Utils::Severity::Error);
				return false;
			}

			return true;
		}
		
		bool Framebuffer::create(FramebufferInfo& p_FramebufferInfo, VkImageView& p_ImageView)
		{
			m_Size = p_FramebufferInfo.p_Size;

			m_RenderPassPtr = p_FramebufferInfo.p_RenderPass;

			m_FrameBufferCreateInfo = {};
			m_FrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			m_FrameBufferCreateInfo.renderPass = m_RenderPassPtr->m_RenderPass;
			m_FrameBufferCreateInfo.attachmentCount = 1;
			m_FrameBufferCreateInfo.pAttachments = &p_ImageView;
			m_FrameBufferCreateInfo.width = m_Size.x;
			m_FrameBufferCreateInfo.height = m_Size.y;
			m_FrameBufferCreateInfo.layers = 1;

			if (vkCreateFramebuffer(m_ContextPtr->m_Device, &m_FrameBufferCreateInfo, nullptr, &m_Framebuffer) != VK_SUCCESS){

				VGL_LOG_MSG("Failed to create framebuffer", "VkFramebuffer", Utils::Severity::Error);
#endif
				return false;
			}
			return true;
		}

		ImageAttachment::ImageAttachment() : m_ContextPtr(&ContextSingleton::getInstance())
		{

		}
		ImageAttachment::~ImageAttachment()
		{

		}

		Image& ImageAttachment::getImage()
		{
			return m_ImageObj;
		}

		bool ImageAttachment::create(ImageAttachmentInfo& p_ImageAttachmentInfo)
		{
			info = p_ImageAttachmentInfo;
			if (p_ImageAttachmentInfo.p_AttachmentInfo == nullptr){

				VGL_LOG_MSG("Failed to create image attachment, p_AttachmentInfo == nullptr", "VulkanImageAttachment", Utils::Severity::Error);
#endif
				return false;
			}

			VkImageUsageFlags usageFlag = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

			m_MipLevels = 1;
			if(p_ImageAttachmentInfo.p_AllowMipMapping)
				m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(p_ImageAttachmentInfo.p_Size.x, p_ImageAttachmentInfo.p_Size.y)))) + 1;

			if (p_ImageAttachmentInfo.p_AttachmentInfo->p_AttachmentType == AttachmentType::Depth)
				usageFlag = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			if (p_ImageAttachmentInfo.p_CreateSampler)
				usageFlag |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (p_ImageAttachmentInfo.p_AllowMipMapping)
				usageFlag |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			m_ImageAllocation = m_ContextPtr->createImage
			(
				p_ImageAttachmentInfo.p_Size.x, p_ImageAttachmentInfo.p_Size.y,
				p_ImageAttachmentInfo.p_AttachmentInfo->p_Format,
				VK_IMAGE_TILING_OPTIMAL,
				usageFlag,
				VMA_MEMORY_USAGE_GPU_ONLY,
				m_VkImageHandle, m_MipLevels, 1, (VkSampleCountFlagBits)p_ImageAttachmentInfo.p_AttachmentInfo->p_SampleCount
			).p_Alloc;

			if (p_ImageAttachmentInfo.p_AttachmentInfo->p_AttachmentType == AttachmentType::Depth) {
				m_ImageView = m_ContextPtr->createImageView(m_VkImageHandle, p_ImageAttachmentInfo.p_AttachmentInfo->p_Format, VK_IMAGE_ASPECT_DEPTH_BIT, m_MipLevels);
				m_ImageViewAttachment = m_ContextPtr->createImageView(m_VkImageHandle, p_ImageAttachmentInfo.p_AttachmentInfo->p_Format, VK_IMAGE_ASPECT_DEPTH_BIT);
			}
			else if (p_ImageAttachmentInfo.p_AttachmentInfo->p_AttachmentType == AttachmentType::Color ||
				p_ImageAttachmentInfo.p_AttachmentInfo->p_AttachmentType == AttachmentType::Resolve) {
				m_ImageView = m_ContextPtr->createImageView(m_VkImageHandle, p_ImageAttachmentInfo.p_AttachmentInfo->p_Format, VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels);
				m_ImageViewAttachment = m_ContextPtr->createImageView(m_VkImageHandle, p_ImageAttachmentInfo.p_AttachmentInfo->p_Format, VK_IMAGE_ASPECT_COLOR_BIT);
			}

			m_ImageObj.m_CurrentLayout = (VkImageLayout)info.p_AttachmentInfo->p_InitialLayout;
			if (p_ImageAttachmentInfo.p_AllowMipMapping) {
				auto cmd = m_ContextPtr->beginSingleTimeCmds();
				for (int i = 0; i < m_MipLevels; i++) {
					VkImageSubresourceRange range = {};
					range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					range.baseMipLevel = i;
					range.levelCount = 1;
					range.baseArrayLayer = 0;
					range.layerCount = 1;
					m_ContextPtr->setImageLayout(
						cmd, m_VkImageHandle,
						(VkImageLayout)p_ImageAttachmentInfo.p_AttachmentInfo->p_InitialLayout,
						(VkImageLayout)p_ImageAttachmentInfo.p_AttachmentInfo->p_FinalLayout,
						range
					);
				}
				m_ContextPtr->endSingleTimeCmds(cmd);
			}
			else if (p_ImageAttachmentInfo.p_TransitionLayoutImage){
				auto cmd = m_ContextPtr->beginSingleTimeCmds();
				VkImageSubresourceRange range = {};
				if (p_ImageAttachmentInfo.p_AttachmentInfo->p_AttachmentType == AttachmentType::Depth)
					range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				else if (p_ImageAttachmentInfo.p_AttachmentInfo->p_AttachmentType == AttachmentType::Color)
					range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				range.baseMipLevel = 0;
				range.levelCount = 1;
				range.baseArrayLayer = 0;
				range.layerCount = 1;
				m_ContextPtr->setImageLayout(
					cmd, m_VkImageHandle,
					(VkImageLayout)p_ImageAttachmentInfo.p_AttachmentInfo->p_InitialLayout,
					(VkImageLayout)p_ImageAttachmentInfo.p_AttachmentInfo->p_FinalLayout,
					range, p_ImageAttachmentInfo.p_AttachmentInfo->p_Format
				);
				m_ContextPtr->endSingleTimeCmds(cmd);
				m_ImageObj.m_CurrentLayout = (VkImageLayout)info.p_AttachmentInfo->p_FinalLayout;
			}

			m_ImageObj.m_VkImageHandle = m_VkImageHandle;
			m_ImageObj.m_ImageAllocation = m_ImageAllocation;
			m_ImageObj.m_ImageView = m_ImageView;
			m_ImageObj.m_FinalLayout = (VkImageLayout)info.p_AttachmentInfo->p_FinalLayout;
			m_ImageObj.m_Size = p_ImageAttachmentInfo.p_Size;
			m_ImageObj.m_MipLevels = m_MipLevels;

			if (p_ImageAttachmentInfo.p_CreateSampler)
			{
				VkSamplerCreateInfo samplerCreateInfo = {};
				samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerCreateInfo.maxAnisotropy = 1.0f;
				samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
				samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
				samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				samplerCreateInfo.addressModeU = (VkSamplerAddressMode)p_ImageAttachmentInfo.p_SamplerMode;
				samplerCreateInfo.addressModeV = (VkSamplerAddressMode)p_ImageAttachmentInfo.p_SamplerMode;
				samplerCreateInfo.addressModeW = (VkSamplerAddressMode)p_ImageAttachmentInfo.p_SamplerMode;
				samplerCreateInfo.mipLodBias = 0.0f;
				samplerCreateInfo.minLod = 0.0f;
				samplerCreateInfo.maxLod = (p_ImageAttachmentInfo.p_AllowMipMapping ? static_cast<float>(m_MipLevels) : 1.0f);
				samplerCreateInfo.borderColor = (VkBorderColor)p_ImageAttachmentInfo.p_BorderColor;
				if (p_ImageAttachmentInfo.p_AllowMipMapping) {
					samplerCreateInfo.anisotropyEnable = VK_TRUE;
					samplerCreateInfo.maxAnisotropy = 16;
					samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
					samplerCreateInfo.compareEnable = VK_FALSE;
					samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
				}

				if (vkCreateSampler(m_ContextPtr->m_Device, &samplerCreateInfo, nullptr, &m_Sampler) != VK_SUCCESS){

					VGL_LOG_MSG("Failed to create sampler for image attachment", "VulkanImageAttachment", Utils::Severity::Error);
#endif
				}

				m_ImageObj.m_Sampler = m_Sampler;
				m_ImageObj.createDescriptors();
			}

			m_IsValid = true;
			m_ImageObj.m_IsValid = m_IsValid;

			m_AttachmentImageInfo = {};
			m_AttachmentImageInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
			m_AttachmentImageInfo.layerCount = 1;
			m_AttachmentImageInfo.usage = usageFlag;
			m_AttachmentImageInfo.pViewFormats = &p_ImageAttachmentInfo.p_AttachmentInfo->p_Format;
			m_AttachmentImageInfo.viewFormatCount = 1;
			m_AttachmentImageInfo.width = p_ImageAttachmentInfo.p_Size.x;
			m_AttachmentImageInfo.height = p_ImageAttachmentInfo.p_Size.y;
		}

		void ImageAttachment::destroy()
		{
			m_IsValid = false;

			m_ContextPtr->destroyImage(m_VkImageHandle, m_ImageAllocation);
			
			if(info.p_CreateSampler)
				vkDestroySampler(m_ContextPtr->m_Device, m_Sampler, nullptr);
			vkDestroyImageView(m_ContextPtr->m_Device, m_ImageView, nullptr);
		}
	}
}