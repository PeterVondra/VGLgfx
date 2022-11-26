#pragma once

#include "../../VGL_Logger.h"

#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include<vulkan/vulkan.h>

#include <vector>
#include <deque>
#include <set>
#include <string>
#include <functional>

#include "VkDescriptorLayoutCache.h"

#include "../../../libs/VulkanMemoryAllocator/include/vk_mem_alloc.h"

namespace vgl
{
	namespace vk
	{
		// Allocator for descriptors
		class DescriptorAllocator
		{
			public:
				DescriptorAllocator(VkDevice p_Device);
				~DescriptorAllocator();

				struct PoolSizes
				{
					std::vector<std::pair<VkDescriptorType, float>> sizes =
					{
						{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
						{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
						{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
						{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
						{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
						{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
						{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
						{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
						{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
						{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
						{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
					};
				};

				void resetPools();
				bool allocate(VkDescriptorSet* p_DescriptorSet, VkDescriptorSetLayout p_Layout);

				void cleanup();

				VkDescriptorPool getPool();
			private:
				friend class Context;

				VkDevice m_Device;

				VkDescriptorPool m_CurrentPool{ VK_NULL_HANDLE };
				PoolSizes m_DescriptorSizes;
				std::vector<VkDescriptorPool> m_ActivePools;
				std::vector<VkDescriptorPool> m_FreePools;

				VkDescriptorPool createPool(const PoolSizes& p_PoolSizes, uint32_t p_Count, VkDescriptorPoolCreateFlags p_Flags);
		};

		struct AllocationInfo
		{
			VmaAllocation p_Alloc;
			VmaAllocationInfo p_AllocInfo;
		};

		// Struct containing the indices for the queue family that will be used
		struct QueueFamilyIndices
		{
			uint32_t graphicsFamily = UINT32_MAX;
			uint32_t presentFamily = UINT32_MAX;
			uint32_t computeFamily = UINT32_MAX;

			bool isValid() { return graphicsFamily >= 0 && presentFamily >= 0 && computeFamily >= 0; }
		};

		struct PhysicalDevice
		{
			// Suitability, higher = better
			uint32_t m_Score;

			QueueFamilyIndices m_QueueFamily;
			VkPhysicalDevice m_VkHandle;
			VkPhysicalDeviceFeatures m_DeviceFeatures;
			VkPhysicalDeviceFeatures2 m_DeviceFeatures2;
			VkPhysicalDeviceProperties m_DeviceProperties;
			VkPhysicalDeviceProperties2 m_DeviceProperties2;

			VkPhysicalDeviceImagelessFramebufferFeatures m_ImagelessFramebufferFeatures = {};
			VkPhysicalDeviceBufferDeviceAddressFeaturesEXT m_DeviceBufferAdressFeatures = {};

			VkPhysicalDeviceMultiviewFeatures m_MultiViewFeatures = {};
			VkPhysicalDeviceMultiviewProperties m_MultiViewProperties = {};

			std::string getDeviceType()
			{
				std::string deviceType;
				deviceType = m_DeviceProperties.deviceType == 0 ? "unknown type" : deviceType;
				deviceType = m_DeviceProperties.deviceType == 1 ? "integrated" : deviceType;
				deviceType = m_DeviceProperties.deviceType == 2 ? "discrete" : deviceType;
				deviceType = m_DeviceProperties.deviceType == 3 ? "virtual" : deviceType;
				deviceType = m_DeviceProperties.deviceType == 4 ? "cpu" : deviceType;
				return deviceType;
			}
		};

		struct SwapchainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		class Context
		{
			public:
				uint64_t TotalDrawCalls = 0;

			public:
				Context();
				~Context();

				void shutdown();

				VkDevice m_Device;
				VkInstance m_Instance;

				PhysicalDevice m_PhysicalDevice;
				VkQueue m_GraphicsQueue;
				VkQueue m_ComputeQueue;
				VkQueue m_PresentQueue;

				VmaAllocator m_VmaAllocator;

				VkCommandPool m_DefaultCommandPool;
				VkDescriptorPool m_DefaultDescriptorPool;
				std::vector<VkDescriptorPoolSize> m_DefaultDescriptorPoolSizes;

				VkSurfaceKHR* m_SurfacePtr;
				uint16_t m_SwapchainImageCount = UINT16_MAX;
				uint32_t m_ImageIndex = UINT32_MAX; // Image index for frame that is currently occupied by renderer
				uint32_t m_CurrentFrame = UINT32_MAX; // Current frame that is currently not occupied by the renderer

				bool m_Initialized = false;

				size_t padUniformBufferSize(const size_t& p_OriginalSize);

				// Size = swapchain image count, used for dynamic descriptors / one allocator per frame
				std::vector<DescriptorAllocator> m_DescriptorAllocators;
				// For persistent descriptors
				DescriptorAllocator m_DescriptorAllocator;
				DescriptorLayoutCache m_DescriptorSetLayoutCache;

				std::vector<VkFence>* m_InFlightFencesPtr = nullptr;

			public:
				// Initialize Vulkan Memory Allocator
				void VMA_Init();

				void deviceWaitIdle();
				void waitForFences();

				VkCommandBuffer beginSingleTimeCmds();
				void endSingleTimeCmds(VkCommandBuffer& p_CommandBuffer);

				// Utility functions for image handling
				void setImageLayout(VkCommandBuffer p_CommandBuffer, VkImage p_Image, VkImageAspectFlags p_Aspect, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout);
				void setImageLayout(VkCommandBuffer p_CommandBuffer, VkImage p_Image, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout, VkImageSubresourceRange p_SubRSRCRange, VkFormat p_Format = (VkFormat)0);
				void transitionLayoutImage(VkImage p_Image, VkFormat p_Format, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout, uint32_t p_MipLevels = 1, uint32_t p_ArrayLayers = 1);
				void copyBufferToImage(VkBuffer& p_Buffer, VkImage& p_Image, uint32_t p_Width, uint32_t p_Height, uint32_t p_ArrayLayers = 1);

				AllocationInfo createImage(
					uint32_t p_Width, uint32_t p_Height,
					VkFormat p_Format,
					VkImageTiling p_Tiling, 
					VkImageUsageFlags p_UsageFlags,
					VmaMemoryUsage p_MemoryUsage,
					VkImage& p_Image, uint32_t p_MipLevels = 1,
					uint32_t p_ArrayLayers = 1, VkSampleCountFlagBits p_Samples = VK_SAMPLE_COUNT_1_BIT
				);
				AllocationInfo createImage(
					uint32_t p_Width, uint32_t p_Height,
					VkFormat p_Format,
					VkImageTiling p_Tiling, 
					VkImageUsageFlags p_UsageFlags, VkImageCreateFlags p_CreateFlags,
					VmaMemoryUsage p_MemoryUsage,
					VkImage& p_Image, uint32_t p_MipLevels = 1,
					uint32_t p_ArrayLayers = 1, VkSampleCountFlagBits p_Samples = VK_SAMPLE_COUNT_1_BIT
				);

				AllocationInfo createImageS(
					uint32_t p_Width, uint32_t p_Height,
					VkFormat p_Format,
					VkImageTiling p_Tiling, VkImageUsageFlags p_UsageFlags,
					VmaMemoryUsage p_MemoryUsage,
					VkImage& p_Image, VkSampleCountFlagBits p_Samples
				);

				VkImageView createImageView(VkImage p_Image, VkFormat p_Format, VkImageAspectFlags p_AspectFlags, uint32_t p_MipLevels = 1, uint32_t p_ArrayLayers = 1, VkImageViewType p_ImageViewType = VK_IMAGE_VIEW_TYPE_2D);

				void destroyImage(VkImage& p_Image, VmaAllocation& p_BufferAlloc);

				void* mapMemory(VmaAllocation& p_BufferAlloc);
				void unmapMemory(VmaAllocation& p_BufferAlloc);
				void destroyBuffer(VkBuffer& p_Buffer, VmaAllocation& p_BufferAlloc);

				AllocationInfo createBuffer(VkDeviceSize p_Size, VkBufferUsageFlags p_Usage, VmaMemoryUsage p_MemoryUsage, VkBuffer& p_Buffer);
				VkBufferCreateInfo createBuffer(VkDeviceSize p_Size, VkBufferUsageFlags p_Usage, VkMemoryPropertyFlags p_Properties, VkBuffer& p_Buffer, VkDeviceMemory& p_BufferMemory);

				void copyBuffer(VkBuffer p_SrcBuffer, VkBuffer p_DstBuffer, VkDeviceSize p_Size);
				uint32_t findMemoryType(uint32_t p_TypeFilter, VkMemoryPropertyFlags p_Properties);

			protected:
			private:
				friend class Swapchain;
				friend class CommandBuffer;
				friend class Window;

				// Functions passed from delete submissions (via function ptrs)
				struct VkDeleteQueueFun {
					VkDeleteQueueFun(std::function<void()> p_VkDeleteFun, void* p_DeleteObjectHash = nullptr)
						: delete_object_hash(p_DeleteObjectHash), vkDeleteFun(p_VkDeleteFun) {}
					// Relevant data for data copy (to uniform/storage buffers)
					void* delete_object_hash = nullptr;
					std::function<void()> vkDeleteFun = nullptr;

					bool operator==(void* p_DeleteObjectHash) { return delete_object_hash == p_DeleteObjectHash;  }
				};

				std::deque<VkDeleteQueueFun> m_VkDeleteQueue;

				bool m_Shutdown = false;

				void initStructs();
				std::vector<const char*> getRequiredExtensions();

				void initInstance();
				void initCommandPool();
				void initLogicalDevice();
				void setPhysicalDevice(PhysicalDevice& p_PhysicalDevice);
				std::vector<PhysicalDevice> getPhysicalDevices(VkSurfaceKHR& p_Surface, SwapchainSupportDetails& p_SwapchainSupport);

				SwapchainSupportDetails querySwapchainSupport(VkSurfaceKHR& p_Surface, VkPhysicalDevice& p_Device);

			public:
				VkFormat findDepthFormat();
				bool hasStencilComponent(VkFormat p_Format);

				// Find supported depth formats
				VkFormat findSupportedFormats(const std::vector<VkFormat>& p_Candidates, VkImageTiling p_Tiling, VkFormatFeatureFlags p_Features);

			private:
				// Setup vulkan validation layers
				VkResult setupDebugMessenger();

				static void DestroyDebugUtilsMessengerEXT(VkInstance p_Instance, VkDebugUtilsMessengerEXT p_Callback, const VkAllocationCallbacks* p_Allocator);

				static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
					VkDebugUtilsMessageSeverityFlagBitsEXT p_MessageSeverity,
					VkDebugUtilsMessageTypeFlagsEXT p_MessageType,
					const VkDebugUtilsMessengerCallbackDataEXT* p_CallbackData,
					void* p_UserData
				);

				bool d_CheckValidationLayerSupport();

				// Get suitable physical device, rates the physical devices and returns the one with best suitability/score
				PhysicalDevice getSuitablePhysicalDevice(std::vector<PhysicalDevice>& p_PhysicalDevices);

				// Get the queue family indices 
				QueueFamilyIndices getQueueFamilyIndices(VkSurfaceKHR& p_Surface, VkPhysicalDevice& p_PhysicalDevice);

				// Rates the physical device for suitability
				uint32_t rateDeviceSuitability(VkSurfaceKHR& p_Surface, SwapchainSupportDetails& p_SwapchainSupport, PhysicalDevice& p_PhysicalDevice, QueueFamilyIndices& p_QueueFamilyIndices);

				// Get the physical device properties
				VkPhysicalDeviceProperties getDeviceProperties(PhysicalDevice& p_PhysicalDevice);
				// Get the physical device features
				VkPhysicalDeviceFeatures getDeviceFeatures(PhysicalDevice& p_PhysicalDevice);
				void getDeviceFeatures2(PhysicalDevice& p_PhysicalDevice);
			private:

				// Check if device supports extensions
				static bool checkDeviceExtensionSupport(std::vector<const char*>& p_DeviceExtensions, VkPhysicalDevice& p_Device);

			private:
				static std::string m_DebugMessage;
				VkDebugUtilsMessengerEXT m_DebugMessenger;

				std::vector<const char*> m_ValidationLayerExtensions;

			private:
				VkApplicationInfo m_AppInfo;
				VkInstanceCreateInfo m_InstanceCreateInfo;

				// The extensions from the vulkan context
				std::vector<const char*> m_DeviceExtensions;
				std::vector<VmaAllocatorCreateFlagBits> m_VmaDeviceExtensions;
		};

		// Using singleton to access all objects in vulkan context, since device is necessary for every vulkan call
		class ContextSingleton
		{
			public:
				inline static Context& getInstance()
				{
					static Context instance;
					return instance;
				}
			public:
				ContextSingleton(ContextSingleton const&) = delete;
				ContextSingleton(ContextSingleton&&) = delete;
				void operator=(ContextSingleton const&) = delete;
				void operator=(ContextSingleton&&) = delete;
		};
	}
}
