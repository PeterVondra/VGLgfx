#pragma once

#include "VkContext.h"

#include "../Definitions.h"

namespace vgl
{
	namespace vk
	{
		namespace {
			VkFormat getImageFormat(ColorSpace p_ColorSpace, Channels p_Channels)
			{
				if (p_ColorSpace == ColorSpace::RGB) {
					switch (p_Channels) {
						case Channels::R: return VK_FORMAT_R8_UNORM;
						case Channels::RG: return VK_FORMAT_R8G8_UNORM;
						case Channels::RGB: return VK_FORMAT_R8G8B8_UNORM;
						case Channels::RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
					}
				}
				else if (p_ColorSpace == ColorSpace::SRGB) {
					switch (p_Channels) {
					case Channels::R: return VK_FORMAT_R8_SRGB;
					case Channels::RG: return VK_FORMAT_R8G8_SRGB;
					case Channels::RGB: return VK_FORMAT_R8G8B8_SRGB;
					case Channels::RGBA: return VK_FORMAT_R8G8B8A8_SRGB;
					}
				}
			}
		}
		class Image
		{
		public:
			Image();
			Image(Vector2i p_Size, unsigned char* p_ImageData, ColorSpace p_ColorSpace, Channels p_Channels, SamplerMode p_SamplerMode, Filter p_MagFilter = Filter::Linear, Filter p_MinFilter = Filter::Linear);
			Image(Vector3i p_Size, unsigned char* p_ImageData, ColorSpace p_ColorSpace, Channels p_Channels, SamplerMode p_SamplerMode, Filter p_MagFilter = Filter::Linear, Filter p_MinFilter = Filter::Linear);

			// Create 2D image with image data
			void create(Vector2i p_Size, unsigned char* p_ImageData, ColorSpace p_ColorSpace, Channels p_Channels, SamplerMode p_SamplerMode,
				bool p_CalcMipLevels = false, Filter p_MagFilter = Filter::Linear, Filter p_MinFilter = Filter::Linear);
			// Create 3D Image with image data
			void create(Vector3i p_Size, unsigned char* p_ImageData, ColorSpace p_ColorSpace, Channels p_Channels, SamplerMode p_SamplerMode,
				bool p_CalcMipLevels = false, Filter p_MagFilter = Filter::Linear, Filter p_MinFilter = Filter::Linear);
			// Reserved for font and text rendering
			void create(Vector2i p_Size, std::vector<std::pair<unsigned char*, uint32_t>>& p_ImageData, ColorSpace p_ColorSpace, Channels p_Channels, SamplerMode p_SamplerMode,
				Filter p_MagFilter = Filter::Linear, Filter p_MinFilter = Filter::Linear);

			void init();
			void freeImageData();

			bool isValid();
			void destroy();

			//VkDescriptorSet& getImGuiID() { return m_DescriptorSet; }
		private:
			void createDescriptors();

			// Create the image
			void createImage();
			void createImage(std::vector<std::pair<unsigned char*, uint32_t>>& p_ImageData);
			// Create the image view
			void createImageView();
			// Create the sampler to be used in the shader
			void createSampler(SamplerMode p_SamplerMode);
			// Generate mip maps for image
			void generateMipMaps(VkImage p_Image, VkFormat p_Format, Vector2i p_Size, uint32_t mipLevels);

		private:
			friend class ImageLoader;
			template<typename ImageType> friend class SamplerDescriptorData;
			friend class Descriptor;
			friend class ImageAttachment;
			friend class Framebuffer;
			friend class Renderer;
			friend class GraphicsContext;
			friend class ImGuiContext;

			Context* m_ContextPtr;

			bool m_IsValid = false;
			bool m_IsDescriptorSetValid = false;

			unsigned char* m_ImageData;

			Vector2i m_Size = { 0, 0 };
			uint32_t m_MipLevels;

			SamplerMode m_SamplerMode;
			Filter m_MagFilter;
			Filter m_MinFilter;

			VkBuffer m_StagingBuffer;
			void* m_Mapped;

			VkFormat		m_ImageFormat;
			VkImage			m_VkImageHandle;
			VmaAllocation	m_ImageAllocation;
			VkImageView		m_ImageView;
			VkImageLayout	m_CurrentLayout;
			VkImageLayout	m_FinalLayout;
			VkSampler		m_Sampler;

			VkDescriptorSet m_DescriptorSet;
			VkDescriptorSetLayout m_DescriptorSetLayout;

			VkDescriptorSet m_ImGuiDescriptorSet;

			ColorSpace m_ColorSpace;
			Channels m_Channels;

		};

		class ImageCube
		{
		public:

			ImageCube();
			~ImageCube();

			// Create an empty cubemap
			void createEmpty(Vector2i p_Size, VkFormat p_Format);
			void createEmpty(Vector2i p_Size, VkFormat p_Format, const uint32_t p_MipLevels);

			bool isValid();

			void destroy();
		private:
			void createImage();
			void createImageView();
			void createSampler(SamplerMode p_SamplerMode);

		private:
			friend class ImageLoader;
			template<typename ImageType> friend class SamplerDescriptorData;
			friend class Descriptor;
			friend class ImageAttachment;
			friend class Framebuffer;
			friend class GraphicsContext;

			Context* m_ContextPtr;

			bool m_IsValid = false;

			std::vector<unsigned char*> m_ImageData;
			Vector2i m_Size;
			uint32_t m_MipLevels;

			VkBuffer m_StagingBuffer;
			void* m_Mapped;

			SamplerMode m_SamplerMode;
			Filter m_MagFilter;
			Filter m_MinFilter;

			VkFormat		m_ImageFormat;
			VkImage			m_VkImageHandle;
			VmaAllocation	m_ImageAllocation;
			VkImageView		m_ImageView;
			VkSampler		m_Sampler;
			VkImageLayout	m_Layout;

			ColorSpace m_ColorSpace;
			Channels m_Channels;
		};

		class ImageLoader
		{
		public:

			static bool getImageFromPath(
				Image& p_Image,
				const char* p_Path, 
				ColorSpace p_ColorSpace,
				SamplerMode p_SamplerMode = SamplerMode::Repeat,
				Filter p_MagFilter = Filter::Linear,
				Filter p_MinFilter = Filter::Linear
			);
			static bool getImageFromPath(
				Image& p_Image,
				const char* p_Path,
				ColorSpace p_ColorSpace,
				SamplerMode p_SamplerMode,
				const uint32_t p_MipLevels
			);
			static unsigned char* getImageDataFromPath(
				Image& p_Image,
				const char* p_Path,
				ColorSpace p_ColorSpace,
				SamplerMode p_SamplerMode = SamplerMode::Repeat,
				Filter p_MagFilter = Filter::Linear,
				Filter p_MinFilter = Filter::Linear
			);

			static bool getImageFromPath(
				ImageCube& p_Image,
				std::initializer_list<std::pair<const char*, Vector3f>> p_Path,
				ColorSpace p_ColorSpace,
				SamplerMode p_SamplerMode
			);
			static bool getImageFromPath(
				ImageCube& p_Image,
				std::vector<std::pair<const char*, Vector3f>> p_Path,
				ColorSpace p_ColorSpace,
				SamplerMode p_SamplerMode
			);

		private:

		};
	}
}
