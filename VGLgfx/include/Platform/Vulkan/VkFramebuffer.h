#pragma once

#include "VkContext.h"
#include "VkRenderPass.h"
#include "../../Math/Vector.h"
#include "VkImage.h"

namespace vgl
{
	namespace vk
	{
		enum class AttachmentViewType
		{
			Image2D = VK_IMAGE_VIEW_TYPE_2D,
			Image3D = VK_IMAGE_VIEW_TYPE_3D,
			ImageCube = VK_IMAGE_VIEW_TYPE_CUBE
		};

		struct ImageAttachmentInfo
		{
			AttachmentViewType p_ViewType = AttachmentViewType::Image2D;
			AttachmentInfo* p_AttachmentInfo;
			BorderColor p_BorderColor;
			SamplerMode p_SamplerMode;
			Vector2i p_Size;
			bool p_CreateSampler = false;
			bool p_TransitionLayoutImage = true;

			private:
				friend class ImageAttachment;
				friend class FramebufferAttachment;

				bool p_AllowMipMapping = false;
		};

		class ImageAttachment
		{
		public:
			ImageAttachment();
			~ImageAttachment();

			// Create an image attachment for rendering, based of renderpass attachments
			bool create(ImageAttachmentInfo& p_ImageAttachmentInfo);
			
			// Get Image attachment as image to be used for descriptors
			Image& getImage();
			ImageCube& getImageCube();
			void destroy();

			bool isValid() { return m_IsValid; }

		protected:
		private:
			friend class Renderer;
			friend class Framebuffer;
			friend class FramebufferAttachment;
			friend class GraphicsContext;

			// Attachment binding
			uint32_t m_AttachmentId;
			ImageAttachmentInfo info;
			bool m_IsValid = false;

			Image m_ImageObj;
			ImageCube m_ImageCubeObj;

			Context* m_ContextPtr;

			VkSampler m_Sampler;
			VkImage m_VkImageHandle;
			VkImageView m_ImageView;
			VkImageView m_ImageViewAttachment;
			VmaAllocation m_ImageAllocation;

			uint32_t m_MipLevels = 1;

			VkImageCreateInfo m_ImageCreateInfo;
			VkImageViewCreateInfo m_ImageViewCreateInfo;

			VkFramebufferAttachmentImageInfo m_AttachmentImageInfo;
		};

		struct FramebufferInfo
		{
			bool p_AllowMipMapping = false;
			bool p_ImageViewTypeCubeEnabled = false;

			Vector2i p_Size;
			RenderPass* p_RenderPass;
			std::vector<ImageAttachment> p_ImageAttachments;
		};

		class Framebuffer
		{
			public:

				Framebuffer();
				~Framebuffer();

				// Create framebuffer based on image attachments
				bool create(FramebufferInfo& p_FramebufferInfo);
				// Only one image attachment
				bool create(FramebufferInfo& p_FramebufferInfo, VkImageView& p_ImageView);

				bool destroy();

				// Get vulkan framebuffer handle
				VkFramebuffer& getVkFramebufferHandle() { return m_Framebuffer; };

			protected:
			private:
				friend class Renderer;
				friend class FramebufferAttachment;
				friend class CommandBuffer;
				friend class GraphicsContext;

				Context* m_ContextPtr;

				VkFramebufferCreateInfo m_FrameBufferCreateInfo;
				std::vector<VkFramebufferAttachmentImageInfo> m_AttachmentImageInfos;
				std::vector<VkImageView> m_ImageViews;

				Vector2i m_Size;
				VkFramebuffer m_Framebuffer;

				RenderPass* m_RenderPassPtr;
		};
	}
}