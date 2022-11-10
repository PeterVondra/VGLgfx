#include "VkImage.h"
#include "../../VGL_Logging.h"
#include <cstring>

#include <SOIL/SOIL.h>

#undef max
#undef min

namespace vgl
{
	namespace vk
	{
		Image::Image() : m_ContextPtr(&ContextSingleton::getInstance())
		{
			m_Channels.resize(4);
			m_Channels[0].first = Channels::R; m_Channels[0].second = 1;
			m_Channels[1].first = Channels::RG; m_Channels[1].second = 2;
			m_Channels[2].first = Channels::RGB; m_Channels[2].second = 3;
			m_Channels[3].first = Channels::RGBA; m_Channels[3].second = 4;
		}

		Image::Image(Vector2i p_Size, unsigned char* p_ImageData, Channels p_Channels, SamplerMode p_SamplerMode, Filter p_MagFilter, Filter p_MinFilter)
			: m_ContextPtr(&ContextSingleton::getInstance())
		{
			m_Channels.resize(4);
			m_Channels[0].first = Channels::R; m_Channels[0].second = 1;
			m_Channels[1].first = Channels::RG; m_Channels[1].second = 2;
			m_Channels[2].first = Channels::RGB; m_Channels[2].second = 3;
			m_Channels[3].first = Channels::RGBA; m_Channels[3].second = 4;

		}

		Image::Image(Vector3i p_Size, unsigned char* p_ImageData, Channels p_Channels, SamplerMode p_SamplerMode, Filter p_MagFilter, Filter p_MinFilter)
			: m_ContextPtr(&ContextSingleton::getInstance())
		{
			m_Channels.resize(4);
			m_Channels[0].first = Channels::R; m_Channels[0].second = 1;
			m_Channels[1].first = Channels::RG; m_Channels[1].second = 2;
			m_Channels[2].first = Channels::RGB; m_Channels[2].second = 3;
			m_Channels[3].first = Channels::RGBA; m_Channels[3].second = 4;

		}

		void Image::create(Vector2i p_Size, unsigned char* p_ImageData, Channels p_Channels, SamplerMode p_SamplerMode, bool p_CalcMipLevels, Filter p_MagFilter, Filter p_MinFilter)
		{
			m_IsValid = false;

			m_MipLevels = p_CalcMipLevels ? static_cast<uint32_t>(std::floor(std::log2(std::max(p_Size.x, p_Size.y)))) + 1 : 1;
			m_Size = p_Size;
			m_ImageData = p_ImageData;

			m_MagFilter = p_MagFilter;
			m_MinFilter = p_MinFilter;

			if (!m_ImageData)
			{
				VGL_LOG_MSG("p_Image was nullptr", "VkImage", Utils::Severity::Warning);
				m_IsValid = false;
			}

			m_CurrentChannels = p_Channels;


			createImage();
			createImageView();
			createSampler(p_SamplerMode);
			createDescriptors();

			m_IsValid = true;
		}

		/*void Image::create(Vector3i p_Size, unsigned char* p_ImageData, Channels p_Channels, SamplerMode p_SamplerMode, bool p_CalcMipLevels, Filter p_MagFilter, Filter p_MinFilter)
		{
			m_IsValid = false;

			m_MipLevels = p_CalcMipLevels ? static_cast<uint32_t>(std::floor(std::log2(std::max(p_Size.x, p_Size.y)))) + 1 : 1;
			m_Size = p_Size;
			m_ImageData = p_ImageData;

			m_MagFilter = p_MagFilter;
			m_MinFilter = p_MinFilter;

			if (!m_ImageData)
			{
				VGL_LOG_MSG("p_Image was nullptr", "Texture", Utils::Severity::Warning);
				m_IsValid = false;
			}

			m_CurrentChannels = p_Channels;

			createImage();
			createImageView();
			createSampler(p_SamplerMode);
			createDescriptors();

			m_IsValid = true;
		}*/

		void Image::create(Vector2i p_Size, std::vector<std::pair<unsigned char*, unsigned int>>& p_ImageData, Channels p_Channels, SamplerMode p_SamplerMode, Filter p_MagFilter, Filter p_MinFilter)
		{
			m_IsValid = false;
			m_MipLevels = 1;
			m_Size = p_Size;

			m_MagFilter = p_MagFilter;
			m_MinFilter = p_MinFilter;

			if (!m_ImageData){
				VGL_LOG_MSG("p_Image was nullptr", "Texture", Utils::Severity::Warning);
				m_IsValid = false;
			}

			m_CurrentChannels = p_Channels;

			createImage(p_ImageData);
			createImageView();
			createSampler(p_SamplerMode);
			createDescriptors();

			m_IsValid = true;
		}

		void Image::init()
		{
			createImage();
			createImageView();
			createSampler(m_SamplerMode);
			createDescriptors();
			m_IsValid = true;
		}

		void Image::freeImageData()
		{
			if(m_ImageData)
				SOIL_free_image_data(m_ImageData);
		}

		bool Image::isValid()
		{
			return m_IsValid;
		}

		void Image::destroy()
		{
			m_IsValid = false;

			m_ContextPtr->destroyImage(m_VkImageHandle, m_ImageAllocation);
			
			vkDestroySampler(m_ContextPtr->m_Device, m_Sampler, nullptr);
			vkDestroyImageView(m_ContextPtr->m_Device, m_ImageView, nullptr);
			
			vkDestroyDescriptorSetLayout(m_ContextPtr->m_Device, m_DescriptorSetLayout, nullptr);
			vkFreeDescriptorSets(m_ContextPtr->m_Device, m_ContextPtr->m_DefaultDescriptorPool, 1, &m_DescriptorSet);

			VGL_LOG_MSG("Destroyed vulkan Image", "VulkanImage", Utils::Severity::Trace);
		}

		// Use for imgui texture id
		void Image::createDescriptors()
		{
			VkDescriptorSetLayoutBinding binding = {};
			//binding.binding = 0;
			binding.descriptorCount = 1;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			
			VkDescriptorSetLayoutCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = 1;
			info.pBindings = &binding;
			
			vkCreateDescriptorSetLayout(m_ContextPtr->m_Device, &info, nullptr, &m_DescriptorSetLayout);
			
			// Create Descriptor Set:
			VkDescriptorSetAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.descriptorPool = m_ContextPtr->m_DefaultDescriptorPool;
			alloc_info.descriptorSetCount = 1;
			alloc_info.pSetLayouts = &m_DescriptorSetLayout;
			
			VkResult result = vkAllocateDescriptorSets(m_ContextPtr->m_Device, &alloc_info, &m_DescriptorSet);
			
			// Update the Descriptor Set:
			VkDescriptorImageInfo desc_image = {};
			desc_image.sampler = m_Sampler;
			desc_image.imageView = m_ImageView;
			desc_image.imageLayout = m_FinalLayout;
			
			VkWriteDescriptorSet write_desc = {};
			write_desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc.dstSet = m_DescriptorSet;
			write_desc.descriptorCount = 1;
			write_desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc.pImageInfo = &desc_image;
			
			vkUpdateDescriptorSets(m_ContextPtr->m_Device, 1, &write_desc, 0, NULL);
		}

		void Image::createImage()
		{
			if (m_CurrentChannels != Channels::None)
			{
				VkDeviceSize imageSize = 0;

				for (auto& channels : m_Channels)
				{
					if (m_CurrentChannels == channels.first)
					{
						imageSize = static_cast<size_t>(m_Size.x * m_Size.y * channels.second);
						break;
					}
				}

				VkFormat format = (VkFormat)m_CurrentChannels;

				auto alloc = m_ContextPtr->createBuffer(
					imageSize,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VMA_MEMORY_USAGE_CPU_ONLY,
					m_StagingBuffer
				).p_Alloc;

				m_Mapped = m_ContextPtr->mapMemory(alloc);
				memcpy(m_Mapped, m_ImageData, static_cast<size_t>(imageSize));
				m_ContextPtr->unmapMemory(alloc);


				m_ImageAllocation = m_ContextPtr->createImage
				(
					m_Size.x, m_Size.y,
					format,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VMA_MEMORY_USAGE_GPU_ONLY,
					m_VkImageHandle, m_MipLevels
				).p_Alloc;

				m_ContextPtr->transitionLayoutImage
				(
					m_VkImageHandle,
					format,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels
				);
				m_ContextPtr->copyBufferToImage
				(
					m_StagingBuffer,
					m_VkImageHandle,
					static_cast<uint32_t>(m_Size.x),
					static_cast<uint32_t>(m_Size.y)
				);

				generateMipMaps(m_VkImageHandle, VK_FORMAT_R16G16B16A16_SFLOAT, m_Size, m_MipLevels);

				m_CurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				m_FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				m_ContextPtr->destroyBuffer(m_StagingBuffer, alloc);
			}
		}

		void Image::createImage(std::vector<std::pair<unsigned char*, unsigned int>>& p_ImageData)
		{
			if (m_CurrentChannels != Channels::None){
				int numChannels = 0;

				VkDeviceSize imageSize = 0;

				for (auto& channels : m_Channels){
					if (m_CurrentChannels == channels.first){
						imageSize = static_cast<size_t>(m_Size.x * m_Size.y * channels.second);
						numChannels = channels.second;
						break;
					}
				}

				VkFormat format = (VkFormat)m_CurrentChannels;

				auto alloc = m_ContextPtr->createBuffer(
					imageSize,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VMA_MEMORY_USAGE_CPU_ONLY,
					m_StagingBuffer
				).p_Alloc;

				unsigned int offset = 0;
				m_Mapped = m_ContextPtr->mapMemory(alloc);
				for (auto& data : p_ImageData){
					memcpy((char*)m_Mapped + offset, data.first, static_cast<size_t>(data.second * numChannels));
					offset += data.second;
				}
				m_ContextPtr->unmapMemory(alloc);

				m_ImageAllocation = m_ContextPtr->createImage(
					m_Size.x, m_Size.y,
					format,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VMA_MEMORY_USAGE_GPU_ONLY,
					m_VkImageHandle, m_MipLevels
				).p_Alloc;

				m_ContextPtr->transitionLayoutImage(
					m_VkImageHandle,
					format,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels
				);
				m_ContextPtr->copyBufferToImage(
					m_StagingBuffer,
					m_VkImageHandle,
					static_cast<uint32_t>(m_Size.x),
					static_cast<uint32_t>(m_Size.y)
				);

				generateMipMaps(m_VkImageHandle, VK_FORMAT_R16G16B16A16_SFLOAT, m_Size, m_MipLevels);

				m_CurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				m_FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				m_ContextPtr->destroyBuffer(m_StagingBuffer, alloc);
			}
		}

		void Image::createImageView()
		{
			m_ImageView = m_ContextPtr->createImageView(m_VkImageHandle, (VkFormat)m_CurrentChannels, VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels);
		}

		void Image::createSampler(SamplerMode p_SamplerMode)
		{
			m_SamplerMode = p_SamplerMode;

			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = (VkFilter)m_MagFilter;
			samplerInfo.minFilter = (VkFilter)m_MinFilter;
			samplerInfo.addressModeU = (VkSamplerAddressMode)p_SamplerMode;
			samplerInfo.addressModeV = (VkSamplerAddressMode)p_SamplerMode;
			samplerInfo.addressModeW = (VkSamplerAddressMode)p_SamplerMode;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = 16;
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = static_cast<float>(m_MipLevels);

			if (vkCreateSampler(m_ContextPtr->m_Device, &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {

				VGL_LOG_MSG("Failed to create image sampler", "VkImage->Sampler", Utils::Severity::Error);
#endif
			}
		}

		void Image::generateMipMaps(VkImage p_Image, VkFormat p_Format, Vector2i p_Size, uint32_t mipLevels)
		{
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(m_ContextPtr->m_PhysicalDevice.m_VkHandle, p_Format, &formatProperties);

			if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
				VGL_LOG_MSG("Image format does not support linear blitting!", "VkImage", Utils::Severity::Warning);

				m_ContextPtr->transitionLayoutImage(
					p_Image,
					p_Format,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels
				);

				return;
			}

			VkCommandBuffer cmd = m_ContextPtr->beginSingleTimeCmds();

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = p_Image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = 1;

			int mipWidth = p_Size.x;
			int mipHeight = p_Size.y;

			for (int i = 1; i < mipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

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

				vkCmdBlitImage(cmd, p_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, p_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

				if (mipWidth > 1) mipWidth /= 2;

				if (mipHeight > 1) mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			m_CurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			m_ContextPtr->endSingleTimeCmds(cmd);
		}

		ImageCube::ImageCube()
			: m_ContextPtr(&ContextSingleton::getInstance())
		{
		}
		ImageCube::~ImageCube()
		{
		}
		void ImageCube::createEmpty(Vector2i p_Size, VkFormat p_Format)
		{
		}
		void ImageCube::createEmpty(Vector2i p_Size, VkFormat p_Format, const uint32_t p_MipLevels)
		{
		}
		bool ImageCube::isValid()
		{
			return false;
		}
		void ImageCube::destroy()
		{
			m_IsValid = false;
			
			m_ContextPtr->destroyImage(m_VkImageHandle, m_ImageAllocation);
			
			vkDestroySampler(m_ContextPtr->m_Device, m_Sampler, nullptr);
			vkDestroyImageView(m_ContextPtr->m_Device, m_ImageView, nullptr);

			VGL_LOG_MSG("Destroyed vulkan ImageCube", "VulkanImageCube", Utils::Severity::Trace);
		}
		void ImageCube::createImage()
		{
			if (m_CurrentChannels != Channels::None){
				VkDeviceSize imageSize = 0;
				VkDeviceSize imageLayerSize = 0;

				for (auto& channels : m_Channels){
					if (m_CurrentChannels == channels.first){
						imageSize = static_cast<size_t>(m_Size[0].x * m_Size[0].y * channels.second * 6); // 6 faces to the cubemap
						imageLayerSize = imageSize/6; // 6 faces to the cubemap
						break;
					}
				}

				VkFormat format = (VkFormat)m_CurrentChannels;

				auto alloc = m_ContextPtr->createBuffer(
					imageSize,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VMA_MEMORY_USAGE_CPU_ONLY,
					m_StagingBuffer
				).p_Alloc;

				m_Mapped = m_ContextPtr->mapMemory(alloc);
				for(uint8_t i = 0; i < 6; i++)
					memcpy((void*)((int64_t)m_Mapped + imageLayerSize * i), m_ImageData[i], static_cast<size_t>(imageSize));
				m_ContextPtr->unmapMemory(alloc);


				m_ImageAllocation = m_ContextPtr->createImage(
					m_Size[0].x, m_Size[0].y,
					format,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VMA_MEMORY_USAGE_GPU_ONLY,
					m_VkImageHandle, m_MipLevels, 6
				).p_Alloc;

				m_ContextPtr->transitionLayoutImage(
					m_VkImageHandle,
					format,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels, 6
				);
				m_ContextPtr->copyBufferToImage(
					m_StagingBuffer,
					m_VkImageHandle,
					static_cast<uint32_t>(m_Size[0].x),
					static_cast<uint32_t>(m_Size[0].y), 6
				);
				m_ContextPtr->transitionLayoutImage(
					m_VkImageHandle,
					format,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_MipLevels
				);

				m_Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				m_ContextPtr->destroyBuffer(m_StagingBuffer, alloc);
			}
		}
		void ImageCube::createImageView()
		{
			m_ImageView = m_ContextPtr->createImageView(m_VkImageHandle, (VkFormat)m_CurrentChannels, VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels, VK_IMAGE_VIEW_TYPE_CUBE);
		}
		void ImageCube::createSampler(SamplerMode p_SamplerMode)
		{
			m_SamplerMode = p_SamplerMode;

			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = (VkFilter)m_MagFilter;
			samplerInfo.minFilter = (VkFilter)m_MinFilter;
			samplerInfo.addressModeU = (VkSamplerAddressMode)p_SamplerMode;
			samplerInfo.addressModeV = (VkSamplerAddressMode)p_SamplerMode;
			samplerInfo.addressModeW = (VkSamplerAddressMode)p_SamplerMode;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = 16;
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = static_cast<float>(m_MipLevels);

			if (vkCreateSampler(m_ContextPtr->m_Device, &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {

				VGL_LOG_MSG("Failed to create image sampler", "VkImage->Sampler", Utils::Severity::Error);
#endif
			}
		}

		bool ImageLoader::getImageFromPath(Image& p_Image, const char* p_Path, SamplerMode p_SamplerMode, Filter p_MagFilter, Filter p_MinFilter)
		{
			int channels;

			p_Image.m_MagFilter = p_MagFilter;
			p_Image.m_MinFilter = p_MinFilter;
			p_Image.m_ImageData = (unsigned char*)SOIL_load_image(p_Path, &p_Image.m_Size.x, &p_Image.m_Size.y, &channels, SOIL_LOAD_RGBA);
			p_Image.m_IsValid = false;

			if (!p_Image.m_ImageData){
				VGL_LOG_MSG("Failed to load image " + std::string(p_Path) + "", " Image Loader", Utils::Severity::Warning);
				return false;
			}

			p_Image.m_CurrentChannels = Channels::RGBA;

			// Calculate mip levels
			p_Image.m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(p_Image.m_Size.x, p_Image.m_Size.y)))) + 1;
			p_Image.m_IsValid = true;
			p_Image.m_SamplerMode = p_SamplerMode;

			p_Image.createImage();
			p_Image.createImageView();
			p_Image.createSampler(p_SamplerMode);
			p_Image.createDescriptors();

			SOIL_free_image_data(p_Image.m_ImageData);

			return true;
		}
		bool ImageLoader::getImageFromPath(Image& p_Image, const char* p_Path, SamplerMode p_SamplerMode, const uint32_t p_MipLevels)
		{
			int channels;
			p_Image.m_ImageData = (unsigned char*)SOIL_load_image(p_Path, &p_Image.m_Size.x, &p_Image.m_Size.y, &channels, SOIL_LOAD_RGBA);

			p_Image.m_IsValid = false;

			if (!p_Image.m_ImageData){
				VGL_LOG_MSG("Failed to load image " + std::string(p_Path) + "", " Image Loader", Utils::Severity::Warning);
				return false;
			}

			p_Image.m_CurrentChannels = Channels::RGBA;
			p_Image.m_MipLevels = p_MipLevels;
			p_Image.m_IsValid = true;
			p_Image.m_SamplerMode = p_SamplerMode;

			p_Image.createImage();
			p_Image.createImageView();
			p_Image.createSampler(p_SamplerMode);
			p_Image.createDescriptors();

			SOIL_free_image_data(p_Image.m_ImageData);

			return true;
		}
		unsigned char* ImageLoader::getImageDataFromPath(Image& p_Image, const char* p_Path, SamplerMode p_SamplerMode, Filter p_MagFilter, Filter p_MinFilter)
		{
			int channels;

			p_Image.m_MagFilter = p_MagFilter;
			p_Image.m_MinFilter = p_MinFilter;
			p_Image.m_ImageData = (unsigned char*)SOIL_load_image(p_Path, &p_Image.m_Size.x, &p_Image.m_Size.y, &channels, SOIL_LOAD_RGBA);
			p_Image.m_IsValid = false;

			if (!p_Image.m_ImageData) {
				VGL_LOG_MSG("Failed to load image " + std::string(p_Path) + "", " Image Loader", Utils::Severity::Warning);
				return false;
			}

			p_Image.m_CurrentChannels = Channels::RGBA;

			// Calculate mip levels
			p_Image.m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(p_Image.m_Size.x, p_Image.m_Size.y)))) + 1;
			p_Image.m_SamplerMode = p_SamplerMode;

			return p_Image.m_ImageData;
		}
		bool ImageLoader::getImageFromPath(ImageCube& p_ImageCube, std::initializer_list<std::pair<const char*, Vector3f>> p_Path, SamplerMode p_SamplerMode)
		{
			std::vector<std::pair<const char*, Vector3f>> path(p_Path.begin(), p_Path.end());
			path.resize(6);
			p_ImageCube.m_ImageData.resize(6);

			int channels;
			p_ImageCube.m_ImageData[0] = (unsigned char*)SOIL_load_image(path[0].first, &p_ImageCube.m_Size[0].x, &p_ImageCube.m_Size[0].y, &channels, SOIL_LOAD_RGBA);
			p_ImageCube.m_ImageData[1] = (unsigned char*)SOIL_load_image(path[1].first, &p_ImageCube.m_Size[1].x, &p_ImageCube.m_Size[1].y, &channels, SOIL_LOAD_RGBA);
			p_ImageCube.m_ImageData[2] = (unsigned char*)SOIL_load_image(path[2].first, &p_ImageCube.m_Size[2].x, &p_ImageCube.m_Size[2].y, &channels, SOIL_LOAD_RGBA);
			p_ImageCube.m_ImageData[3] = (unsigned char*)SOIL_load_image(path[3].first, &p_ImageCube.m_Size[3].x, &p_ImageCube.m_Size[3].y, &channels, SOIL_LOAD_RGBA);
			p_ImageCube.m_ImageData[4] = (unsigned char*)SOIL_load_image(path[4].first, &p_ImageCube.m_Size[4].x, &p_ImageCube.m_Size[4].y, &channels, SOIL_LOAD_RGBA);
			p_ImageCube.m_ImageData[5] = (unsigned char*)SOIL_load_image(path[5].first, &p_ImageCube.m_Size[5].x, &p_ImageCube.m_Size[5].y, &channels, SOIL_LOAD_RGBA);

			p_ImageCube.m_CurrentChannels = Channels::RGBA;

			// Calculate mip levels
			p_ImageCube.m_MipLevels = 1;
			p_ImageCube.m_SamplerMode = p_SamplerMode;
			p_ImageCube.m_IsValid = true;

			p_ImageCube.createImage();
			p_ImageCube.createImageView();
			p_ImageCube.createSampler(p_SamplerMode);

			SOIL_free_image_data(p_ImageCube.m_ImageData[0]);
			SOIL_free_image_data(p_ImageCube.m_ImageData[1]);
			SOIL_free_image_data(p_ImageCube.m_ImageData[2]);
			SOIL_free_image_data(p_ImageCube.m_ImageData[3]);
			SOIL_free_image_data(p_ImageCube.m_ImageData[4]);
			SOIL_free_image_data(p_ImageCube.m_ImageData[5]);

			return true;
		}
		bool ImageLoader::getImageFromPath(ImageCube& p_ImageCube, std::vector<std::pair<const char*, Vector3f>> p_Path, SamplerMode p_SamplerMode)
		{
			int channels;
			p_Path.resize(6);
			p_ImageCube.m_ImageData.resize(6);

			p_ImageCube.m_ImageData[0] = (unsigned char*)SOIL_load_image(p_Path[0].first, &p_ImageCube.m_Size[0].x, &p_ImageCube.m_Size[0].y, &channels, SOIL_LOAD_RGBA);
			p_ImageCube.m_ImageData[1] = (unsigned char*)SOIL_load_image(p_Path[1].first, &p_ImageCube.m_Size[1].x, &p_ImageCube.m_Size[1].y, &channels, SOIL_LOAD_RGBA);
			p_ImageCube.m_ImageData[2] = (unsigned char*)SOIL_load_image(p_Path[2].first, &p_ImageCube.m_Size[2].x, &p_ImageCube.m_Size[2].y, &channels, SOIL_LOAD_RGBA);
			p_ImageCube.m_ImageData[3] = (unsigned char*)SOIL_load_image(p_Path[3].first, &p_ImageCube.m_Size[3].x, &p_ImageCube.m_Size[3].y, &channels, SOIL_LOAD_RGBA);
			p_ImageCube.m_ImageData[4] = (unsigned char*)SOIL_load_image(p_Path[4].first, &p_ImageCube.m_Size[4].x, &p_ImageCube.m_Size[4].y, &channels, SOIL_LOAD_RGBA);
			p_ImageCube.m_ImageData[5] = (unsigned char*)SOIL_load_image(p_Path[5].first, &p_ImageCube.m_Size[5].x, &p_ImageCube.m_Size[5].y, &channels, SOIL_LOAD_RGBA);

			p_ImageCube.m_CurrentChannels = Channels::RGBA;

			// Calculate mip levels
			p_ImageCube.m_MipLevels = 1;
			p_ImageCube.m_SamplerMode = p_SamplerMode;

			p_ImageCube.m_IsValid = true;
			p_ImageCube.createImage();
			p_ImageCube.createImageView();
			p_ImageCube.createSampler(p_SamplerMode);

			SOIL_free_image_data(p_ImageCube.m_ImageData[0]);
			SOIL_free_image_data(p_ImageCube.m_ImageData[1]);
			SOIL_free_image_data(p_ImageCube.m_ImageData[2]);
			SOIL_free_image_data(p_ImageCube.m_ImageData[3]);
			SOIL_free_image_data(p_ImageCube.m_ImageData[4]);
			SOIL_free_image_data(p_ImageCube.m_ImageData[5]);

			return true;
		}
	}
}
