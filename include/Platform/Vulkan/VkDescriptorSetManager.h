#include "VkContext.h"
#include "VkImage.h"
#include "../../Utils/Logger.h"

namespace vgl
{
	namespace vk
	{
		class ImGuiContext;

		// Get vulkan handle for the shader stage
		VkShaderStageFlagBits getShaderStageVkH(ShaderStage p_ShaderStage);

		// Double/Triple buffered depending on the swapchain setup
		struct UniformBuffer
		{
			UniformBuffer() : m_ContextPtr(&ContextSingleton::getInstance()) {};
			UniformBuffer(const uint64_t p_Size, const uint32_t p_Binding) : m_ContextPtr(&ContextSingleton::getInstance()), m_Size(p_Size), m_Binding(p_Binding) {};
			~UniformBuffer() {};

			// Must be set before creation (before DescriptorSetManager::create(...);
			uint64_t m_Size = 0;
			uint32_t m_Binding = UINT16_MAX; // Has to be set

			private:
				friend class DescriptorSetInfo;
				friend class DescriptorSetManager;

				// Use one buffer but multiple segments for Double/Triple buffering
				std::vector<void*> m_MappedData;
				VkBuffer m_Buffer;
				VkDeviceMemory m_Memory;

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

			// Must be set before creation (before DescriptorSetManager::create(...);
			uint64_t m_Size = UINT64_MAX; // Max size is UINT64_MAX - 1
			uint32_t m_Binding = UINT32_MAX; // Max binding is UINT32_MAX - 1

			//private:
				//friend class DescriptorSetInfo;
				//friend class DescriptorSetManager;
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
			ImageType* p_Image = nullptr;
			uint32_t p_Binding = UINT32_MAX; // Has to be set
			// Frame index is used when image is only used for an individual frame,
			// in case of double buffering (2 frames), index 0 and 1 refers to two different images/frames.
			// If index < 0 then index will not be used
			int32_t p_FrameIndex = -1;

			ShaderStage p_ShaderStage = ShaderStage::FragmentBit; // Can be set to vertexbit, but fragmentbit is the default

			bool isValid() { return p_Image && p_Binding != UINT32_MAX; }

			private:
				friend class DescriptorSetManager;

				VkDescriptorImageInfo p_ImageInfo;
				VkImageLayout p_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Optional
		};
		
		template<> struct SamplerDescriptorData<std::vector<Image>>
		{
			SamplerDescriptorData() {}
			SamplerDescriptorData(std::vector<Image>* p_Image_, uint32_t p_Binding_);

			std::vector<Image>* p_Image = nullptr;
			uint32_t p_Binding = UINT32_MAX; // Has to be set

			ShaderStage p_ShaderStage = ShaderStage::FragmentBit; // Can be set to vertexbit, but fragmentbit is the default
			
			bool isValid() { return p_Image && p_Binding != UINT32_MAX; }
			
			private:
				friend class DescriptorSetManager;

				std::vector<std::pair<VkImageView, VkSampler>> p_ImageInfo;
				VkImageLayout p_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Optional
				std::vector<VkDescriptorImageInfo> m_ImageArrayDescriptorInfo;
				std::vector<uint32_t> m_ImageArrayDescriptorBindings;

		};

		struct DescriptorSetInfo
		{
			DescriptorSetInfo() {};
			~DescriptorSetInfo() {};

			StorageBuffer p_StorageBuffer;

			union{
				struct{
					UniformBuffer p_VertexUniformBuffer;
					UniformBuffer p_FragmentUniformBuffer;
					UniformBuffer p_GeometryUniformBuffer;
				};
				UniformBuffer p_UniformBuffers[3];
			};

			void addImage(Image* p_Image, uint32_t p_Binding, int32_t p_FrameIndex = -1){
				p_ImageDescriptors.emplace_back(SamplerDescriptorData(p_Image, p_Binding, p_FrameIndex));
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
				friend class DescriptorSetManager;

				bool isImageDescriptorsValid();
				bool isStorageImage3dDescriptorsValid();
				bool isImageArrayDescriptorsValid();
				bool isCubeMapDescriptorsValid();
		};

		class DescriptorSetManager
		{
			public:
				DescriptorSetManager();
				~DescriptorSetManager();

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

			protected:
			private:
				friend class Renderer;

				//void createDescriptorPool();
				void createDescriptorSetLayout();
				void createDescriptorSets();
				void updateDescriptorSets();
				void flush();

			private:

				friend class CommandBuffer;

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