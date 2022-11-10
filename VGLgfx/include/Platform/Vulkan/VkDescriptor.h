#pragma once

#include "VkContext.h"
#include "VkImage.h"
#include "../../VGL_Logging.h"

namespace vgl
{
	namespace vk
	{
		// Pre-declarations
		class CommandBuffer;
		class FramebufferAttachment;
		class ImGuiContext;

		// Double/Triple buffered depending on the swapchain setup
		struct UniformBuffer
		{
			UniformBuffer() : m_ContextPtr(&ContextSingleton::getInstance()) {};
			UniformBuffer(const uint64_t p_Size, const uint32_t p_Binding = 0) : m_ContextPtr(&ContextSingleton::getInstance()), m_Size(p_Size), m_Binding(p_Binding) {};	
			~UniformBuffer() {};

			// Must be set before creation (before Descriptor::create(...);
			uint64_t m_Size = 0;
			uint32_t m_Binding = UINT16_MAX; // Has to be set

			private:
				friend class DescriptorSetInfo;
				friend class Descriptor;

				// Use one buffer but multiple segments for Double/Triple buffering
				std::vector<void*> m_MappedData;
				VkBuffer m_Buffer;
				AllocationInfo m_AllocInfo;

				std::vector<uint64_t> m_Offsets;

				bool m_IsValid = false;

				Context* m_ContextPtr;

				void createBuffer();
				void destroy();
				bool isValid() { return m_Size > 0 && m_Binding != UINT32_MAX; }
		};


		struct StorageBuffer
		{
			StorageBuffer() : m_ContextPtr(&ContextSingleton::getInstance()), m_Size(0) {};
			StorageBuffer(ShaderStage p_ShaderStage, const uint64_t p_Size, const uint32_t p_Binding) : m_ContextPtr(&ContextSingleton::getInstance()), m_ShaderStage(p_ShaderStage), m_Size(p_Size), m_Binding(p_Binding) {};
			~StorageBuffer() {};

			// Must be set before creation (before Descriptor::create(...);
			uint64_t m_Size = UINT64_MAX; // Max size is UINT64_MAX - 1
			uint32_t m_Binding = UINT32_MAX; // Max binding is UINT32_MAX - 1

			//private:
				//friend class DescriptorSetInfo;
				//friend class Descriptor;
			Context* m_ContextPtr = nullptr;

			// Use one buffer but multiple segments for Double/Triple buffering
			std::vector<void*> m_MappedData;
			VkBuffer m_Buffer;
			AllocationInfo m_AllocInfo;

			ShaderStage m_ShaderStage;

			std::vector<uint64_t> m_Offsets;

			bool m_IsValid = false;

			void createBuffer();
			void destroy();
			inline bool isValid() { return (m_Size > 0 && m_Size < UINT64_MAX&& m_Binding < UINT32_MAX&& m_ContextPtr == &ContextSingleton::getInstance()); }
		};

		// Image, Image Array or CubeMap
		template<typename ImageType> struct SamplerDescriptorData
		{
			SamplerDescriptorData() {}
			SamplerDescriptorData(ImageType* p_Image_, uint32_t p_Binding_, int32_t p_FrameIndex_ = -1);
			SamplerDescriptorData(ImageType* p_Image_, uint32_t p_Binding_, VkImageLayout p_ImageLayout_, int32_t p_FrameIndex_ = -1);  

			ImageType* p_Image = nullptr;
			uint32_t p_Binding = UINT32_MAX; // Has to be set
			// Frame index is used when image is only used for an individual frame,
			// in case of double buffering (2 frames), index 0 and 1 refers to two different images/frames.
			// If index < 0 then index will not be used
			int32_t p_FrameIndex = -1;

			ShaderStage p_ShaderStage = ShaderStage::FragmentBit; // Can be set to vertexbit, but fragmentbit is the default

			bool isValid() { return p_Image && p_Binding != UINT32_MAX; }

			private:
				friend class Descriptor;

				VkDescriptorImageInfo p_ImageInfo;
				VkImageLayout p_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Optional
		};

		template<> inline SamplerDescriptorData<Image>::SamplerDescriptorData(Image* p_Image_, uint32_t p_Binding_, int32_t p_FrameIndex_)
			: p_Image(p_Image_), p_Binding(p_Binding_), p_FrameIndex(p_FrameIndex_), p_ImageLayout(p_Image->m_FinalLayout)
		{
			p_ImageInfo.imageLayout = p_Image->m_FinalLayout;
			p_ImageInfo.imageView = p_Image->m_ImageView;
			p_ImageInfo.sampler = p_Image->m_Sampler;
		}
		template<> inline SamplerDescriptorData<Image>::SamplerDescriptorData(Image* p_Image_, uint32_t p_Binding_, VkImageLayout p_ImageLayout_, int32_t p_FrameIndex_)
			: p_Image(p_Image_), p_Binding(p_Binding_), p_FrameIndex(p_FrameIndex_), p_ImageLayout(p_ImageLayout_)
		{
			p_ImageInfo.imageLayout = p_ImageLayout_;
			p_ImageInfo.imageView = p_Image->m_ImageView;
			p_ImageInfo.sampler = p_Image->m_Sampler;
		}
		template<> inline SamplerDescriptorData<ImageCube>::SamplerDescriptorData(ImageCube* p_Image_, uint32_t p_Binding_, int32_t p_FrameIndex_)
			: p_Image(p_Image_), p_Binding(p_Binding_), p_FrameIndex(p_FrameIndex_), p_ImageLayout(p_Image->m_Layout)
		{
			p_ImageInfo.imageLayout = p_Image->m_Layout;
			p_ImageInfo.imageView = p_Image->m_ImageView;
			p_ImageInfo.sampler = p_Image->m_Sampler;
		} 

		template<> struct SamplerDescriptorData<std::vector<Image>>
		{
			SamplerDescriptorData() {}
			SamplerDescriptorData(std::vector<Image>* p_Image_, uint32_t p_Binding_);

			std::vector<Image>* p_Image = nullptr;
			uint32_t p_Binding = UINT32_MAX; // Has to be set

			ShaderStage p_ShaderStage = ShaderStage::FragmentBit; // Can be set to vertexbit, but fragmentbit is the default
			
			bool isValid() { return p_Image && p_Binding != UINT32_MAX; }
			
			private:
				friend class Descriptor;

				std::vector<std::pair<VkImageView, VkSampler>> p_ImageInfo;
				VkImageLayout p_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Optional
				std::vector<VkDescriptorImageInfo> m_ImageArrayDescriptorInfo;
				std::vector<uint32_t> m_ImageArrayDescriptorBindings;

		};

		struct DescriptorSetInfo
		{
			DescriptorSetInfo() 
			: p_VertexUniformBuffer(p_UniformBuffers[0]),
			 p_FragmentUniformBuffer(p_UniformBuffers[1]),
			 p_GeometryUniformBuffer(p_UniformBuffers[2]) {};
			~DescriptorSetInfo() {};

			StorageBuffer p_StorageBuffer;

			UniformBuffer& p_VertexUniformBuffer;
			UniformBuffer& p_FragmentUniformBuffer;
			UniformBuffer& p_GeometryUniformBuffer;
			UniformBuffer p_UniformBuffers[3];

			void addImage(Image* p_Image, uint32_t p_Binding, int32_t p_FrameIndex = -1){
				p_ImageDescriptors.emplace_back(SamplerDescriptorData(p_Image, p_Binding, p_FrameIndex));
			}
			void addImage(Image* p_Image, uint32_t p_Binding, Layout p_Layout, int32_t p_FrameIndex = -1){
				p_ImageDescriptors.emplace_back(SamplerDescriptorData(p_Image, p_Binding, (VkImageLayout)p_Layout, p_FrameIndex));
			}
			void addImageCube(ImageCube* p_Image, uint32_t p_Binding, int32_t p_FrameIndex = -1){
				p_CubeMapDescriptors.emplace_back(SamplerDescriptorData(p_Image, p_Binding, p_FrameIndex));
			}
			void addImageArray(std::vector<Image>* p_Image, uint32_t p_Binding){
				p_ImageArrayDescriptors.emplace_back(SamplerDescriptorData(p_Image, p_Binding));
			}

			std::vector<SamplerDescriptorData<Image>> p_ImageDescriptors;
			//std::vector<SamplerDescriptorData<StorageImage3D>> p_StorageImage3dDescriptors; // TODO: Implement 3D image functionality
			std::vector<SamplerDescriptorData<ImageCube>> p_CubeMapDescriptors;
			std::vector<SamplerDescriptorData<std::vector<Image>>> p_ImageArrayDescriptors;

			inline void clearBuffers() {
				for (auto& buffer : p_UniformBuffers)
					if(buffer.m_IsValid) buffer.destroy();
			}
			inline void clearDescriptors(){
				p_ImageDescriptors.clear();
				p_CubeMapDescriptors.clear();
				//p_StorageImage3dDescriptors.clear();
				p_ImageArrayDescriptors.clear();
			}

			bool isValid() {
				p_IsImageDescriptorsValid = isImageDescriptorsValid();
				p_IsStorageImage3dDescriptorsValid = isStorageImage3dDescriptorsValid();
				p_IsImageArrayDescriptorsValid = isImageArrayDescriptorsValid();
				p_IsCubeMapDescriptorsValid = isCubeMapDescriptorsValid();

				return
					p_VertexUniformBuffer.isValid()		||
					p_FragmentUniformBuffer.isValid()	||
					p_GeometryUniformBuffer.isValid()	||
					p_IsImageDescriptorsValid			||
					p_IsImageArrayDescriptorsValid		||
					p_IsCubeMapDescriptorsValid;
			}

			bool p_IsImageDescriptorsValid = false;
			bool p_IsStorageImage3dDescriptorsValid = false;
			bool p_IsImageArrayDescriptorsValid = false;
			bool p_IsCubeMapDescriptorsValid = false;

			private:
				friend class Descriptor;

				bool isImageDescriptorsValid();
				bool isStorageImage3dDescriptorsValid();
				bool isImageArrayDescriptorsValid();
				bool isCubeMapDescriptorsValid();
		};

		class Descriptor
		{
			public:
				Descriptor();
				~Descriptor();

				void copyToStorageBuffer(void* p_Data, uint64_t p_Size, uint64_t p_Offset);
				void copy(ShaderStage p_ShaderStage, void* p_Data, uint64_t p_Size, uint64_t p_Offset);

				template<typename T> void copy(ShaderStage p_ShaderStage, T&& p_Data, uint64_t p_Offset) {
					copy(p_ShaderStage, &p_Data, sizeof(T), p_Offset);
				}
				template<typename T> void copy(ShaderStage p_ShaderStage, T& p_Data, uint64_t p_Offset) {
					copy(p_ShaderStage, &p_Data, sizeof(T), p_Offset);
				}

				// Updates already bound [descriptor set info], can only be used for samplers
				void update(); // (not available)
				// Updates with new [descriptor set info], can only be used for samplers, p_Info will be bound to the [descriptor set manager]
				void update(DescriptorSetInfo& p_Info); // (not available)

				void create(DescriptorSetInfo& p_Info);
				void destroy();

				// Has it been created?
				bool isValid();

			private:
				// Data passed from copy submissions (via function ptrs)
				struct PCOPYFUN {
					PCOPYFUN(void(Descriptor::* p_CopyFun)(), void* p_Data, uint64_t p_Size, uint64_t p_Offset, ShaderStage p_ShaderStage = ShaderStage::None)
						: copy_Data(p_Data), copy_Size(p_Size), copy_Offset(p_Offset), copy_ShaderStage(p_ShaderStage), D_CopyF(p_CopyFun) {}
					// Relevant data for data copy (to uniform/storage buffers)
					void* copy_Data = nullptr;
					uint64_t copy_Size = 0;
					uint64_t copy_Offset = 0;
					ShaderStage copy_ShaderStage;
					void(Descriptor::* D_CopyF)();
				};

			private:
				friend class Renderer;

				//void createDescriptorPool();
				void createDescriptorSetLayout();
				void createDescriptorSets();
				void updateDescriptorSets();
				void flush();

			private:

				friend class CommandBuffer;
				friend class FramebufferAttachment;
				friend class ImGuiContext;

				Context* m_ContextPtr;

				uint16_t m_UniformBufferCount = 0;

				DescriptorSetInfo* m_DescriptorSetInfo;

				struct DescriptorSetBinding
				{
					VkDescriptorPool m_CurrentDescriptorPool;
					VkDescriptorSet m_DescriptorSet;
				};

				//std::vector<VkDescriptorPoolSize> m_PoolSize;
				VkDescriptorSetLayout m_DescriptorSetLayout;	

				// One for each swapchain image
				std::vector<DescriptorSetBinding> m_DescriptorSets;

				// One for each swapchain image
				std::vector<std::vector<VkDescriptorSetLayoutBinding>> m_LayoutBindings;
				
				// One for each swapchain image
				std::vector<std::vector<VkDescriptorImageInfo>> m_ImageDescriptorInfo;
				std::vector<uint32_t> m_ImageDescriptorBindings;
				
				bool m_IsValid = false;
		};
	}
}
