#define VMA_IMPLEMENTATION
#include "VkContext.h"
#include <chrono>
#include "../../Utils/Logger.h"

#ifdef VGL_INTERNAL_LOGGING_ENABLED
//#define VGL_VK_VALIDATION_LAYERS_ENABLED
#endif

namespace vgl
{
	namespace vk
	{
		std::string Context::m_DebugMessage;

		Context::Context() : m_DescriptorSetLayoutCache(m_Device), m_DescriptorAllocator(m_Device)
		{
			Utils::Logger::setStartTimePoint(std::chrono::steady_clock::now());

			#ifdef VGL_VK_VALIDATION_LAYERS_ENABLED
			m_ValidationLayerExtensions.push_back("VK_LAYER_KHRONOS_validation");
			m_DeviceExtensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
			#endif
			
			m_DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			m_DeviceExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
			m_DeviceExtensions.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
			m_DeviceExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
			m_DeviceExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
			//m_DeviceExtensions.push_back(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
			m_DeviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
			
			initStructs();
			initInstance();

			#ifdef VGL_VK_VALIDATION_LAYERS_ENABLED
			setupDebugMessenger();
			#endif
		}
		Context::~Context()
		{

		}
		void Context::shutdown()
		{
			m_Shutdown = true;
			for (auto delete_fun : m_VkDeleteQueue)
				delete_fun.vkDeleteFun();
		}
		void Context::initStructs()
		{
			m_AppInfo = {};
			m_AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			m_AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			m_AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			m_AppInfo.apiVersion = VK_API_VERSION_1_2;
			m_AppInfo.pNext = NULL;

			VGL_INTERNAL_INFO("[vk::Context]API version: " + Utils::to_string(m_AppInfo.apiVersion));
			VGL_INTERNAL_INFO("[vk::Context]Engine version: " + Utils::to_string(m_AppInfo.engineVersion));
			VGL_INTERNAL_INFO("[vk::Context]Application version: " + Utils::to_string(m_AppInfo.applicationVersion));

			m_InstanceCreateInfo = {};
			m_InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			m_InstanceCreateInfo.pApplicationInfo = &m_AppInfo;
			m_InstanceCreateInfo.pNext = NULL;
		}
		std::vector<const char*> Context::getRequiredExtensions()
		{
			uint32_t glfwExtensionCount = 0;

			const char** glfwExtensions;

			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			#ifdef VGL_VK_VALIDATION_LAYERS_ENABLED
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			#endif

			return extensions;

		}
		void Context::initInstance()
		{
			#ifdef VGL_VK_VALIDATION_LAYERS_ENABLED
			if (!d_CheckValidationLayerSupport())
				VGL_INTERNAL_WARNING("[vk::Context]Validation layers requested but not available");
			else
				VGL_INTERNAL_INFO("[vk::Context]Validation layers available");
			#endif

			auto extensions = getRequiredExtensions();
			extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
			m_InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			m_InstanceCreateInfo.ppEnabledExtensionNames = extensions.data();

			#ifdef VGL_VK_VALIDATION_LAYERS_ENABLED
			m_InstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayerExtensions.size());
			m_InstanceCreateInfo.ppEnabledLayerNames = m_ValidationLayerExtensions.data();
			#else
			m_InstanceCreateInfo.enabledLayerCount = 0;
			#endif

			VkResult result = vkCreateInstance(&m_InstanceCreateInfo, nullptr, &m_Instance);
			
			for (int i = 0; i < extensions.size(); i++)
				VGL_INTERNAL_INFO("[vk::Context]Instance extensions" + std::string(extensions[i]));

			VGL_ASSERT_FATAL(result == VK_SUCCESS, "[vk::Context]Failed to create Vulkan instance, VkResult: %i", (uint64_t)result);

			if (result != VK_SUCCESS) {
				m_VkDeleteQueue.emplace_front(VkDeleteQueueFun([&] { vkDestroyInstance(m_Instance, nullptr); }));
				VGL_INTERNAL_TRACE("[vk::Context]Succesfully created Vulkan instance, VkResult: %i", (uint64_t)result);
			}
		}
		void Context::initCommandPool()
		{
			QueueFamilyIndices queueFamilyIndices = getQueueFamilyIndices(*m_SurfacePtr, m_PhysicalDevice.m_VkHandle);

			VkCommandPoolCreateInfo commandPoolInfo = {};
			commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
			commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPoolInfo.pNext = nullptr;

			VkResult result = vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_DefaultCommandPool);

			VGL_ASSERT_FATAL(result == VK_SUCCESS, "[vk::Context]Failed to create default command pool, VkResult: %i", (uint64_t)result);

			if(result == VK_SUCCESS) m_VkDeleteQueue.emplace_front(VkDeleteQueueFun([&] { vkDestroyCommandPool(m_Device, m_DefaultCommandPool, nullptr); }));
		}
		void Context::initLogicalDevice()
		{
			QueueFamilyIndices indices = getQueueFamilyIndices(*m_SurfacePtr, m_PhysicalDevice.m_VkHandle);

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

			float queuePriority = 1.0f;
			for (int queueFamily : uniqueQueueFamilies)
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;

				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			VkDeviceCreateInfo createInfo = {};
			createInfo.pNext = &m_PhysicalDevice.m_DeviceFeatures2;
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pQueueCreateInfos = queueCreateInfos.data();
			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			//createInfo.pEnabledFeatures = &m_PhysicalDevice.m_DeviceFeatures;
			createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
			createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

			createInfo.enabledLayerCount = 0;
			
			#ifdef VGL_VK_VALIDATION_LAYERS_ENABLED
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayerExtensions.size());
			createInfo.ppEnabledLayerNames = m_ValidationLayerExtensions.data();
			#endif

			{
				VkResult result = vkCreateDevice(m_PhysicalDevice.m_VkHandle, &createInfo, nullptr, &m_Device);

				VGL_ASSERT_FATAL(result == VK_SUCCESS, "[vk::Context]Failed to create logical device, VkResult: %i", (uint64_t)result);
			
				if (result == VK_SUCCESS) {
					m_VkDeleteQueue.emplace_front(VkDeleteQueueFun([&] { vkDestroyDevice(m_Device, nullptr); }));
					VGL_INTERNAL_TRACE("[vk::Context]Succesfully created logical device, VkResult: %i", (uint64_t)result);
				}
			}

			vkGetDeviceQueue(m_Device, indices.graphicsFamily, 0, &m_GraphicsQueue);
			vkGetDeviceQueue(m_Device, indices.presentFamily, 0, &m_PresentQueue);

			initCommandPool();
			m_DefaultDescriptorPoolSizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			{
				//Size of pool to limit size
				VkDescriptorPoolCreateInfo poolInfo = {};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = m_DefaultDescriptorPoolSizes.size();
				poolInfo.pPoolSizes = m_DefaultDescriptorPoolSizes.data();
				poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
				poolInfo.maxSets = static_cast<uint32_t>(1000);

				VkResult result = vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DefaultDescriptorPool);

				VGL_ASSERT_ERROR(result == VK_SUCCESS, "[vk::Context]Failed to create default descriptor pool, VkResult: %i", (uint64_t)result);
			
				if (result == VK_SUCCESS) {
					m_VkDeleteQueue.emplace_front(VkDeleteQueueFun([&] { vkDestroyDescriptorPool(m_Device, m_DefaultDescriptorPool, nullptr); }));
					VGL_INTERNAL_TRACE("[vk::Context]Succesfully created default descriptor pool, VkResult: %i", (uint64_t)result);
				}
			}

			m_DescriptorSetLayoutCache.m_Device = m_Device;
			m_DescriptorAllocator.m_Device = m_Device;
		}

		std::vector<PhysicalDevice> Context::getPhysicalDevices(VkSurfaceKHR& p_Surface, SwapchainSupportDetails& p_SwapchainSupport)
		{
			m_SurfacePtr = &p_Surface;

			m_PhysicalDevice.m_VkHandle = VK_NULL_HANDLE;
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

			VGL_ASSERT_ERROR(deviceCount > 0, "[vk::Context]Failed to find Vulkan supported GPU's, Device Count: %i", deviceCount);

			if (deviceCount > 0) VGL_INTERNAL_TRACE("[vk::Context]Found GPU's with vulkan support");

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

			std::vector<PhysicalDevice> candidates;
			for (auto& m_Device : devices)
			{
				PhysicalDevice physicalDevice;
				physicalDevice.m_VkHandle = m_Device;

				physicalDevice.m_DeviceProperties = getDeviceProperties(physicalDevice);
				physicalDevice.m_DeviceFeatures = getDeviceFeatures(physicalDevice);

				physicalDevice.m_Score = rateDeviceSuitability(p_Surface, p_SwapchainSupport, physicalDevice, physicalDevice.m_QueueFamily);
				candidates.push_back(physicalDevice);
			}

			return candidates;
		}

		SwapchainSupportDetails Context::querySwapchainSupport(VkSurfaceKHR& p_Surface, VkPhysicalDevice& p_Device)
		{
			SwapchainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_Device, p_Surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(p_Device, p_Surface, &formatCount, nullptr);

			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(p_Device, p_Surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;

			vkGetPhysicalDeviceSurfacePresentModesKHR(p_Device, p_Surface, &presentModeCount, nullptr);

			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(p_Device, p_Surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}
		void Context::setPhysicalDevice(PhysicalDevice& p_PhysicalDevice)
		{
			m_PhysicalDevice = p_PhysicalDevice;
			m_PhysicalDevice.m_QueueFamily = p_PhysicalDevice.m_QueueFamily;
			m_PhysicalDevice.m_DeviceFeatures.multiViewport = VK_TRUE;
			m_PhysicalDevice.m_DeviceFeatures.samplerAnisotropy = VK_TRUE;
			m_PhysicalDevice.m_DeviceFeatures.sampleRateShading = VK_TRUE;
			m_PhysicalDevice.m_DeviceFeatures.shaderClipDistance = VK_TRUE;
			m_PhysicalDevice.m_DeviceFeatures.shaderCullDistance = VK_TRUE;
			m_PhysicalDevice.m_DeviceFeatures.depthBiasClamp = VK_TRUE;
			m_PhysicalDevice.m_DeviceBufferAdressFeatures = {};
			m_PhysicalDevice.m_DeviceBufferAdressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
			m_PhysicalDevice.m_DeviceBufferAdressFeatures.bufferDeviceAddressCaptureReplay = VK_TRUE;

			// Enable device buffer adress extension
			m_PhysicalDevice.m_DeviceFeatures2 = {};
			m_PhysicalDevice.m_DeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			m_PhysicalDevice.m_DeviceFeatures2.features = m_PhysicalDevice.m_DeviceFeatures;
			m_PhysicalDevice.m_DeviceFeatures2.pNext = &m_PhysicalDevice.m_DeviceBufferAdressFeatures;
			getDeviceFeatures2(m_PhysicalDevice);

			VGL_INTERNAL_INFO("[vk::Context]Device buffer adress features:");
			VGL_INTERNAL_INFO("[vk::Context]\tBufferDeviceAdress = %i", m_PhysicalDevice.m_DeviceBufferAdressFeatures.bufferDeviceAddress);
			VGL_INTERNAL_INFO("[vk::Context]\tBufferDeviceAdressCaptureReplay = %i", m_PhysicalDevice.m_DeviceBufferAdressFeatures.bufferDeviceAddressCaptureReplay);
			VGL_INTERNAL_INFO("[vk::Context]\tBufferDeviceAddressMultiDevice = %i", m_PhysicalDevice.m_DeviceBufferAdressFeatures.bufferDeviceAddressMultiDevice);

			// Enable imageless framebuffer extension
			m_PhysicalDevice.m_ImagelessFramebufferFeatures = {};
			m_PhysicalDevice.m_ImagelessFramebufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
			m_PhysicalDevice.m_ImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
			m_PhysicalDevice.m_DeviceFeatures2.pNext = &m_PhysicalDevice.m_ImagelessFramebufferFeatures;
			getDeviceFeatures2(m_PhysicalDevice);

			VGL_INTERNAL_INFO("[vk::Context]Imageless framebuffer features:");
			VGL_INTERNAL_INFO("[vk::Context]\tImagelessFramebuffer = %i", m_PhysicalDevice.m_ImagelessFramebufferFeatures.imagelessFramebuffer);

			// Enable multi view extension extension
			m_PhysicalDevice.m_MultiViewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
			m_PhysicalDevice.m_MultiViewFeatures.multiview = VK_TRUE;
			m_PhysicalDevice.m_DeviceFeatures2.pNext = &m_PhysicalDevice.m_MultiViewFeatures;
			getDeviceFeatures2(m_PhysicalDevice);

			VGL_INTERNAL_INFO("[vk::Context]Multiview features:");
			VGL_INTERNAL_INFO("[vk::Context]\tmultiView = %i", m_PhysicalDevice.m_MultiViewFeatures.multiview);
			VGL_INTERNAL_INFO("[vk::Context]\tmultiViewGeometryShader = %i", m_PhysicalDevice.m_MultiViewFeatures.multiviewGeometryShader);
			VGL_INTERNAL_INFO("[vk::Context]\tmultiViewTessellationShader = %i", m_PhysicalDevice.m_MultiViewFeatures.multiviewTessellationShader);

			m_PhysicalDevice.m_MultiViewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
			m_PhysicalDevice.m_DeviceProperties2 = {};
			m_PhysicalDevice.m_DeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			m_PhysicalDevice.m_DeviceProperties2.pNext = &m_PhysicalDevice.m_MultiViewProperties;
			vkGetPhysicalDeviceProperties2(m_PhysicalDevice.m_VkHandle, &m_PhysicalDevice.m_DeviceProperties2);

			VGL_INTERNAL_INFO("[vk::Context]Using Multiview properties:");
			VGL_INTERNAL_INFO("[vk::Context]Using \tmaxMultiview->MultiViewViewCount = %i", m_PhysicalDevice.m_MultiViewProperties.maxMultiviewViewCount);
			VGL_INTERNAL_INFO("[vk::Context]Using \tmaxMultiview->MultiViewInstanceIndex = %i", m_PhysicalDevice.m_MultiViewProperties.maxMultiviewInstanceIndex);

			VGL_INTERNAL_INFO("[vk::Context]Using " + m_PhysicalDevice.getDeviceType() + " graphics " + "[" + m_PhysicalDevice.m_DeviceProperties.deviceName + "]");
		}

		VkResult Context::setupDebugMessenger()
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = DebugCallback;
			createInfo.pUserData = nullptr;

			auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)
				vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");

			if (vkCreateDebugUtilsMessengerEXT != nullptr) {
				VkResult result = vkCreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
				
				VGL_INTERNAL_ASSERT_WARNING(result == VK_SUCCESS, "[vk::Context]Failed to create debug messenger for Vulkan validation layers, VkResult: %i", (uint64_t)result);

				if (result == VK_SUCCESS) m_VkDeleteQueue.emplace_front(VkDeleteQueueFun([&] { DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr); }));
				return result;
			}
			else
				return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		void Context::DestroyDebugUtilsMessengerEXT(VkInstance p_Instance, VkDebugUtilsMessengerEXT p_Callback, const VkAllocationCallbacks* p_Allocator)
		{
			PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(p_Instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)
				func(p_Instance, p_Callback, p_Allocator);
		}
		VKAPI_ATTR VkBool32 VKAPI_CALL Context::DebugCallback
		(
			VkDebugUtilsMessageSeverityFlagBitsEXT p_MessageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT p_MessageType,
			const VkDebugUtilsMessengerCallbackDataEXT* p_CallbackData,
			void* p_UserData
		)
		{
			if (m_DebugMessage != p_CallbackData->pMessage) {
				m_DebugMessage = p_CallbackData->pMessage;

				switch (p_MessageSeverity) {
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
					VGL_INTERNAL_ERROR("[VkValidation]" + m_DebugMessage);
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
					VGL_INTERNAL_WARNING("[VkValidation]" + m_DebugMessage);
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
					VGL_INTERNAL_INFO("[VkValidation]" + m_DebugMessage);
					break;
				default:
					if (p_MessageType ==
						(VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
						VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
					{
						//VGL_LOG_MSG(m_DebugMessage, "Vk Validation", Utils::Severity::Debug);
						break;
					}
					VGL_INTERNAL_INFO("[VkValidation]" + m_DebugMessage);
					break;
				}
			}
			return VK_FALSE;
		}
		bool Context::d_CheckValidationLayerSupport()
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const char* layerName : m_ValidationLayerExtensions)
			{
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if (!layerFound) {
					VGL_INTERNAL_WARNING("[VkValidation]Vulkan validation layers not available");

					return false;
				}
			}
			return true;
		}

		// Get suitable physical m_Device, rates the physical devices and returns the one with best suitability/score
		PhysicalDevice Context::getSuitablePhysicalDevice(std::vector<PhysicalDevice>& p_PhysicalDevices)
		{
			PhysicalDevice suitableDevice;
			uint32_t SCORE = 0;
			for (int i = 0; i < p_PhysicalDevices.size(); i++) {
				if (p_PhysicalDevices[i].m_Score > SCORE) {
					SCORE = p_PhysicalDevices[i].m_Score;
					suitableDevice = p_PhysicalDevices[i];
				}
			}

			return suitableDevice;
		}

		// Get the queue family indices 
		QueueFamilyIndices Context::getQueueFamilyIndices(VkSurfaceKHR& p_Surface, VkPhysicalDevice& p_PhysicalDevice)
		{
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(p_PhysicalDevice, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(p_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

			int i = 0;
			for (const auto& queueFamily : queueFamilies)
			{
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(p_PhysicalDevice, i, p_Surface, &presentSupport);

				if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
					indices.graphicsFamily = i;
					indices.presentFamily = i;
					break;
				}
				if (presentSupport)
					indices.presentFamily = i;
				else if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					indices.graphicsFamily = i;

				i++;
			}

			return indices;

		}

		// Rates the physical m_Device for suitability
		uint32_t Context::rateDeviceSuitability(VkSurfaceKHR& p_Surface, SwapchainSupportDetails& p_SwapchainSupport, PhysicalDevice& p_PhysicalDevice, QueueFamilyIndices& p_QueueFamilyIndices)
		{
			VkPhysicalDeviceProperties deviceProperties = getDeviceProperties(p_PhysicalDevice);
			VkPhysicalDeviceFeatures deviceFeatures = getDeviceFeatures(p_PhysicalDevice);
			p_SwapchainSupport = querySwapchainSupport(p_Surface, p_PhysicalDevice.m_VkHandle);

			int score = 0;
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 1000;

			score += deviceProperties.limits.maxImageDimension1D;
			score += deviceProperties.limits.maxImageDimension2D;
			score += deviceProperties.limits.maxImageDimension3D;

			std::string deviceType;
			deviceType = deviceProperties.deviceType == 0 ? "unknown type" : deviceType;
			deviceType = deviceProperties.deviceType == 1 ? "integrated" : deviceType;
			deviceType = deviceProperties.deviceType == 2 ? "discrete" : deviceType;
			deviceType = deviceProperties.deviceType == 3 ? "virtual" : deviceType;
			deviceType = deviceProperties.deviceType == 4 ? "cpu" : deviceType;

			if (!deviceFeatures.geometryShader)
				return 0;

			p_QueueFamilyIndices = getQueueFamilyIndices(p_Surface, p_PhysicalDevice.m_VkHandle);

			VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
			vkGetPhysicalDeviceMemoryProperties(p_PhysicalDevice.m_VkHandle, &deviceMemoryProperties);

			VGL_INTERNAL_INFO("[vk::Context][" + std::string(deviceProperties.deviceName) + "]");
			VGL_INTERNAL_INFO("[vk::Context]score: " + Utils::to_string(score));
			VGL_INTERNAL_INFO("[vk::Context]m_Device type: " + std::string(deviceType));
			VGL_INTERNAL_INFO("[vk::Context]driver version: " + Utils::to_string(deviceProperties.driverVersion));
			VGL_INTERNAL_INFO("[vk::Context]vulkan version: " + Utils::to_string(deviceProperties.apiVersion));
			VGL_INTERNAL_INFO("[vk::Context]max viewports: " + Utils::to_string(deviceProperties.limits.maxViewports));
			VGL_INTERNAL_INFO("[vk::Context]max tesselation level: " + Utils::to_string(deviceProperties.limits.maxTessellationGenerationLevel));
			VGL_INTERNAL_INFO("[vk::Context]memory heap count: " + Utils::to_string(deviceMemoryProperties.memoryHeapCount));
			VGL_INTERNAL_INFO("[vk::Context]vendor id: " + Utils::to_string(deviceProperties.vendorID));

			bool extensionSupported = checkDeviceExtensionSupport(m_DeviceExtensions, p_PhysicalDevice.m_VkHandle);
			bool SwapchainAdequate = false;
			if (extensionSupported)
				SwapchainAdequate = !p_SwapchainSupport.formats.empty() && !p_SwapchainSupport.presentModes.empty();

			return (p_QueueFamilyIndices.isValid() && extensionSupported && SwapchainAdequate && deviceFeatures.samplerAnisotropy) ? score : 0;
		}

		// Get the physical m_Device properties
		VkPhysicalDeviceProperties Context::getDeviceProperties(PhysicalDevice& p_PhysicalDevice)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(p_PhysicalDevice.m_VkHandle, &deviceProperties);
			return deviceProperties;
		}
		// Get the physical m_Device features
		VkPhysicalDeviceFeatures Context::getDeviceFeatures(PhysicalDevice& p_PhysicalDevice)
		{
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(p_PhysicalDevice.m_VkHandle, &deviceFeatures);
			return deviceFeatures;
		}
		void Context::getDeviceFeatures2(PhysicalDevice& p_PhysicalDevice)
		{
			vkGetPhysicalDeviceFeatures2(p_PhysicalDevice.m_VkHandle, &p_PhysicalDevice.m_DeviceFeatures2);
		}
		bool Context::checkDeviceExtensionSupport(std::vector<const char*>& p_DeviceExtensions, VkPhysicalDevice& p_PhysicalDevice)
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(p_PhysicalDevice, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(p_PhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(p_DeviceExtensions.begin(), p_DeviceExtensions.end());

			for (const auto& extension : availableExtensions){
				requiredExtensions.erase(extension.extensionName);
			}
			return requiredExtensions.empty();
		}

		VkFormat Context::findSupportedFormats(const std::vector<VkFormat>& p_Candidates, VkImageTiling p_Tiling, VkFormatFeatureFlags p_Features)
		{
			for (VkFormat format : p_Candidates)
			{
				VkFormatProperties properties;
				vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice.m_VkHandle, format, &properties);

				if (p_Tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & p_Features) == p_Features)
					return format;
				else if (p_Tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & p_Features) == p_Features)
					return format;
			}

			VGL_INTERNAL_FATAL("[vk::Context]Failed to find supported image formats for swapchain");
		}

		VkFormat Context::findDepthFormat()
		{
			// Find supported depth formats
			return findSupportedFormats({
					VK_FORMAT_D32_SFLOAT, 
					VK_FORMAT_D32_SFLOAT_S8_UINT, 
					VK_FORMAT_D24_UNORM_S8_UINT 
				},
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			);
		}

		bool Context::hasStencilComponent(VkFormat p_Format)
		{
			return p_Format == VK_FORMAT_D32_SFLOAT_S8_UINT || p_Format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

		VkCommandBuffer Context::beginSingleTimeCmds()
		{
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = m_DefaultCommandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer cmdBuffer;
			vkAllocateCommandBuffers(m_Device, &allocInfo, &cmdBuffer);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(cmdBuffer, &beginInfo);

			return cmdBuffer;

		}

		void Context::endSingleTimeCmds(VkCommandBuffer& p_CommandBuffer)
		{
			vkEndCommandBuffer(p_CommandBuffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &p_CommandBuffer;

			vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(m_GraphicsQueue);

			vkFreeCommandBuffers(m_Device, m_DefaultCommandPool, 1, &p_CommandBuffer);
		}

		size_t Context::padUniformBufferSize(const size_t& p_OriginalSize)
		{
			// Calculate required alignment based on minimum device offset alignment
			size_t minUboAlignment = m_PhysicalDevice.m_DeviceProperties.limits.minUniformBufferOffsetAlignment;
			size_t alignedSize = p_OriginalSize;
			if (minUboAlignment > 0) {
				alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
			}
			return alignedSize;
		}

		void Context::VMA_Init()
		{
			VmaAllocatorCreateInfo allocInfo = {};
			allocInfo.vulkanApiVersion = m_AppInfo.apiVersion;
			allocInfo.physicalDevice = m_PhysicalDevice.m_VkHandle;
			allocInfo.device = m_Device;
			allocInfo.instance = m_Instance;

			vmaCreateAllocator(&allocInfo, &m_VmaAllocator);
		}

		void Context::deviceWaitIdle()
		{
			vkDeviceWaitIdle(m_Device);
		}

		void Context::waitForFences()
		{
			if(m_InFlightFencesPtr && m_CurrentFrame < UINT32_MAX) vkWaitForFences(m_Device, 1, &m_InFlightFencesPtr->operator[](m_CurrentFrame), VK_TRUE, UINT64_MAX);
		}

		void Context::setImageLayout(VkCommandBuffer p_CommandBuffer, VkImage p_Image, VkImageAspectFlags p_Aspect, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout)

		{
			VkPipelineStageFlags srcStage;
			VkPipelineStageFlags dstStage;

			VkImageMemoryBarrier imageBarrier = {};
			imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageBarrier.oldLayout = p_OldLayout;
			imageBarrier.newLayout = p_NewLayout;
			imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.image = p_Image;
			imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBarrier.subresourceRange.baseMipLevel = 0;
			imageBarrier.subresourceRange.levelCount = 1;
			imageBarrier.subresourceRange.baseArrayLayer = 0;
			imageBarrier.subresourceRange.layerCount = 1;
			imageBarrier.dstAccessMask = 0;
			imageBarrier.srcAccessMask = 0;

			if (p_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				imageBarrier.srcAccessMask = 0;
				imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (p_OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (p_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				imageBarrier.srcAccessMask = 0;
				imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (p_OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}

			else if (p_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				imageBarrier.srcAccessMask = 0;
				imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else if (p_OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			{
				imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			}
			else if (p_OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			}
			else if (p_OldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
				imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			}

			else
				VGL_INTERNAL_WARNING("[vk::Context/VkPipelineBarrier]Unsupported layout transition: p_OldLayout is %i and p_NewLayout is %i", p_OldLayout, p_NewLayout);

			imageBarrier.subresourceRange.aspectMask = p_Aspect;

			vkCmdPipelineBarrier(p_CommandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
		}
		void Context::setImageLayout(VkCommandBuffer p_CommandBuffer, VkImage p_Image, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout, VkImageSubresourceRange p_SubRSRCRange, VkFormat p_Format)
		{
			VkPipelineStageFlags srcStage;
			VkPipelineStageFlags dstStage;

			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = p_OldLayout;
			imageMemoryBarrier.newLayout = p_NewLayout;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.image = p_Image;
			imageMemoryBarrier.subresourceRange = p_SubRSRCRange;
			imageMemoryBarrier.dstAccessMask = 0;
			imageMemoryBarrier.srcAccessMask = 0;

			srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

			switch (p_OldLayout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				// Image layout is undefined (or does not matter)
				// Only valid as initial layout
				// No flags required, listed only for completeness
				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.srcAccessMask = 0;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				// Image is preinitialized
				// Only valid as initial layout for linear images, preserves memory contents
				// Make sure host writes have been finished
				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image is a color attachment
				// Make sure any writes to the color buffer have been finished
				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image is a depth/stencil attachment
				// Make sure any writes to the depth/stencil buffer have been finished
				srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image is a transfer source
				// Make sure any reads from the image have been finished
				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image is a transfer destination
				// Make sure any writes to the image have been finished
				srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image is read by a shader
				// Make sure any shader reads from the image have been finished
				srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
			}

			// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
			switch (p_NewLayout)
			{
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image will be used as a transfer destination
				// Make sure any writes to the image have been finished
				dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image will be used as a transfer source
				// Make sure any reads from the image have been finished
				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image will be used as a color attachment
				// Make sure any writes to the color buffer have been finished
				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image layout will be used as a depth/stencil attachment
				// Make sure any writes to depth/stencil buffer have been finished
				dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image will be read in a shader (sampler, input attachment)
				// Make sure any writes to the image have been finished
				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				if (imageMemoryBarrier.srcAccessMask == 0)
				{
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				}
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other dst layouts aren't handled (yet)
				VGL_INTERNAL_WARNING("[vk::Context/VkPipelineBarrier]Unsupported layout transition: p_OldLayout is %i and p_NewLayout is %i", p_OldLayout, p_NewLayout);
				break;
			}

			if (p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			{
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (hasStencilComponent(p_Format))
					imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			vkCmdPipelineBarrier(p_CommandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		void Context::transitionLayoutImage(
			VkImage p_Image,
			VkFormat p_Format, 
			VkImageLayout p_OldLayout, 
			VkImageLayout p_NewLayout, 
			uint32_t p_MipLevels, 
			uint32_t p_ArrayLayers
		){
			VkCommandBuffer cmdBuffer = beginSingleTimeCmds();

			VkPipelineStageFlags srcStage;
			VkPipelineStageFlags dstStage;

			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = p_OldLayout;
			imageMemoryBarrier.newLayout = p_NewLayout;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.image = p_Image;
			imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
			imageMemoryBarrier.subresourceRange.levelCount = p_MipLevels;
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			imageMemoryBarrier.subresourceRange.layerCount = p_ArrayLayers;
			imageMemoryBarrier.dstAccessMask = 0;
			imageMemoryBarrier.srcAccessMask = 0;

			srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

			switch (p_OldLayout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				// Image layout is undefined (or does not matter)
				// Only valid as initial layout
				// No flags required, listed only for completeness
				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.srcAccessMask = 0;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				// Image is preinitialized
				// Only valid as initial layout for linear images, preserves memory contents
				// Make sure host writes have been finished
				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image is a color attachment
				// Make sure any writes to the color buffer have been finished
				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image is a depth/stencil attachment
				// Make sure any writes to the depth/stencil buffer have been finished
				srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image is a transfer source
				// Make sure any reads from the image have been finished
				srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image is a transfer destination
				// Make sure any writes to the image have been finished
				srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image is read by a shader
				// Make sure any shader reads from the image have been finished
				srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
			}

			// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
			switch (p_NewLayout)
			{
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image will be used as a transfer destination
				// Make sure any writes to the image have been finished
				dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image will be used as a transfer source
				// Make sure any reads from the image have been finished
				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image will be used as a color attachment
				// Make sure any writes to the color buffer have been finished
				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image layout will be used as a depth/stencil attachment
				// Make sure any writes to depth/stencil buffer have been finished
				dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image will be read in a shader (sampler, input attachment)
				// Make sure any writes to the image have been finished
				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				if (imageMemoryBarrier.srcAccessMask == 0)
				{
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				}
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				VGL_INTERNAL_WARNING("[vk::Context/VkPipelineBarrier]Unsupported layout transition: p_OldLayout is %i and p_NewLayout is %i", p_OldLayout, p_NewLayout);
				break;
			}

			if (p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (hasStencilComponent(p_Format))
					imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			vkCmdPipelineBarrier(cmdBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			endSingleTimeCmds(cmdBuffer);
		}

		void Context::copyBufferToImage(VkBuffer& p_Buffer, VkImage& p_Image, uint32_t p_Width, uint32_t p_Height, uint32_t p_ArrayLayers)
		{
			VkCommandBuffer cmdBuffer = beginSingleTimeCmds();

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = p_ArrayLayers;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { p_Width, p_Height, 1 };

			vkCmdCopyBufferToImage(cmdBuffer, p_Buffer, p_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			endSingleTimeCmds(cmdBuffer);
		}

		AllocationInfo Context::createImage(
			uint32_t p_Width, uint32_t p_Height,
			VkFormat p_Format,
			VkImageTiling p_Tiling, VkImageUsageFlags p_UsageFlags,
			VmaMemoryUsage p_MemoryUsage,
			VkImage& p_Image, uint32_t p_MipLevels,
			uint32_t p_ArrayLayers, VkSampleCountFlagBits p_Samples
		)
		{
			return createImage(p_Width, p_Height, p_Format, p_Tiling, p_UsageFlags, 0, p_MemoryUsage, p_Image, p_MipLevels, p_ArrayLayers, p_Samples);
		}

		AllocationInfo Context::createImage(uint32_t p_Width, uint32_t p_Height, VkFormat p_Format, VkImageTiling p_Tiling, VkImageUsageFlags p_UsageFlags, VkImageCreateFlags p_CreateFlags, VmaMemoryUsage p_MemoryUsage, VkImage& p_Image, uint32_t p_MipLevels, uint32_t p_ArrayLayers, VkSampleCountFlagBits p_Samples)
		{
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = static_cast<uint32_t>(p_Width);
			imageInfo.extent.height = static_cast<uint32_t>(p_Height);
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = p_MipLevels;
			imageInfo.arrayLayers = p_ArrayLayers;
			imageInfo.format = p_Format;
			imageInfo.tiling = p_Tiling;
			imageInfo.usage = p_UsageFlags;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = p_Samples;
			imageInfo.flags = p_CreateFlags;

			VmaAllocationCreateInfo allocCreateInfo = {};
			allocCreateInfo.usage = p_MemoryUsage;
			VmaAllocationInfo allocInfo;
			VmaAllocation alloc;

			VkResult result = vmaCreateImage(m_VmaAllocator, &imageInfo, &allocCreateInfo, &p_Image, &alloc, &allocInfo);
			VGL_INTERNAL_ASSERT_WARNING(result == VK_SUCCESS, "[vk::Context/VkImage]Failed to create image, VkResult: %i", (uint64_t)result);

			return { alloc, allocInfo };
		}

		AllocationInfo Context::createImageS(
			uint32_t p_Width, uint32_t p_Height,
			VkFormat p_Format,
			VkImageTiling p_Tiling, VkImageUsageFlags p_UsageFlags,
			VmaMemoryUsage p_MemoryUsage,
			VkImage& p_Image, VkSampleCountFlagBits p_Samples
		)
		{
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = static_cast<uint32_t>(p_Width);
			imageInfo.extent.height = static_cast<uint32_t>(p_Height);
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = p_Format;
			imageInfo.tiling = p_Tiling;
			imageInfo.usage = p_UsageFlags;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = p_Samples;

			VmaAllocationCreateInfo allocCreateInfo = {};
			allocCreateInfo.usage = p_MemoryUsage;
			VmaAllocationInfo allocInfo;
			VmaAllocation alloc;

			VkResult result = vmaCreateImage(m_VmaAllocator, &imageInfo, &allocCreateInfo, &p_Image, &alloc, &allocInfo);
			VGL_INTERNAL_ASSERT_WARNING(result == VK_SUCCESS, "[vk::Context/VkImage]Failed to create image, VkResult: %i", (uint64_t)result);

			return { alloc, allocInfo };
		}

		VkImageView Context::createImageView(VkImage p_Image, VkFormat p_Format, VkImageAspectFlags p_AspectFlags, uint32_t p_MipLevels, uint32_t p_ArrayLayers, VkImageViewType p_ImageViewType)
		{
			VkImageView imageView;
			VkImageViewCreateInfo imageViewInfo = {};
			imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewInfo.image = p_Image;
			imageViewInfo.viewType = p_ImageViewType;
			imageViewInfo.format = p_Format;
			imageViewInfo.subresourceRange.aspectMask = p_AspectFlags;
			imageViewInfo.subresourceRange.baseMipLevel = 0;
			imageViewInfo.subresourceRange.levelCount = static_cast<float>(p_MipLevels);
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.layerCount = p_ArrayLayers;

			VkResult result = vkCreateImageView(m_Device, &imageViewInfo, nullptr, &imageView);
			VGL_INTERNAL_ASSERT_WARNING(result == VK_SUCCESS, "[vk::Context/VkImageView]Failed to create image view, VkResult: %i", (uint64_t)result);

			return imageView;
		}

		void Context::destroyImage(VkImage& p_Image, VmaAllocation& p_BufferAlloc)
		{
			vmaDestroyImage(m_VmaAllocator, p_Image, p_BufferAlloc);
		}

		void* Context::mapMemory(VmaAllocation& p_BufferAlloc)
		{
			void* mapped_data = nullptr;
			vmaMapMemory(m_VmaAllocator, p_BufferAlloc, &mapped_data);
			return mapped_data;
		}

		void Context::unmapMemory(VmaAllocation& p_BufferAlloc)
		{
			vmaUnmapMemory(m_VmaAllocator, p_BufferAlloc);
		}

		void Context::destroyBuffer(VkBuffer& p_Buffer, VmaAllocation& p_BufferAlloc)
		{
			vmaDestroyBuffer(m_VmaAllocator, p_Buffer, p_BufferAlloc);
		}

		// Create buffer using vma
		AllocationInfo Context::createBuffer(VkDeviceSize p_Size, VkBufferUsageFlags p_Usage, VmaMemoryUsage p_MemoryUsage, VkBuffer& p_Buffer)
		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = p_Size;
			bufferInfo.usage = p_Usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocCreateInfo = {};
			allocCreateInfo.usage = p_MemoryUsage;
			//allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
			VmaAllocation alloc;
			VmaAllocationInfo allocInfo;
			vmaCreateBuffer(m_VmaAllocator, &bufferInfo, &allocCreateInfo, &p_Buffer, &alloc, &allocInfo);

			return { alloc, allocInfo };
		}

		VkBufferCreateInfo Context::createBuffer(VkDeviceSize p_Size, VkBufferUsageFlags p_Usage, VkMemoryPropertyFlags p_Properties, VkBuffer& p_Buffer, VkDeviceMemory& p_BufferMemory)
		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = p_Size;
			bufferInfo.usage = p_Usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, &p_Buffer);
			VGL_INTERNAL_ASSERT_WARNING(result == VK_SUCCESS, "[vk::Context/VkBuffer]Failed to create buffer, VkResult: %i", (uint64_t)result);

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(m_Device, p_Buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, p_Properties);
			{
				VkResult result = vkAllocateMemory(m_Device, &allocInfo, nullptr, &p_BufferMemory);
				VGL_INTERNAL_ASSERT_WARNING(result == VK_SUCCESS, "[vk::Context/VkBuffer]Failed to allocate buffer memory, VkResult: %i", (uint64_t)result);
			}
			{
				VkResult result = vkBindBufferMemory(m_Device, p_Buffer, p_BufferMemory, 0);
				VGL_INTERNAL_ASSERT_WARNING(result == VK_SUCCESS, "[vk::Context/VkBuffer]Failed to bind buffer memory, VkResult: %i", (uint64_t)result);
			}

			return bufferInfo;
		}

		void Context::copyBuffer(VkBuffer p_SrcBuffer, VkBuffer p_DstBuffer, VkDeviceSize p_Size)
		{
			VkCommandBuffer cmdBuffer = beginSingleTimeCmds();
			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size = p_Size;

			vkCmdCopyBuffer(cmdBuffer, p_SrcBuffer, p_DstBuffer, 1, &copyRegion);
			endSingleTimeCmds(cmdBuffer);
		}

		//Memory type for buffers
		uint32_t Context::findMemoryType(uint32_t p_TypeFilter, VkMemoryPropertyFlags p_Properties)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice.m_VkHandle, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
				if ((p_TypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & p_Properties) == p_Properties)
					return i;
			}

			VGL_INTERNAL_ASSERT_ERROR("[vk::Context]Failed to find suitable memory type");
		}

		DescriptorAllocator::DescriptorAllocator(VkDevice p_Device) : m_Device(p_Device)
		{

		}
		DescriptorAllocator::~DescriptorAllocator()
		{

		}

		void DescriptorAllocator::resetPools()
		{
			for (auto& pool : m_ActivePools)
				vkResetDescriptorPool(m_Device, pool, 0);

			m_FreePools = m_ActivePools;
			m_ActivePools.clear();

			m_CurrentPool = VK_NULL_HANDLE;

		}
		bool DescriptorAllocator::allocate(VkDescriptorSet* p_DescriptorSet, VkDescriptorSetLayout p_Layout)
		{
			if (m_CurrentPool == VK_NULL_HANDLE) {
				m_CurrentPool = getPool();
				m_ActivePools.push_back(m_CurrentPool);
			}

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;

			allocInfo.pSetLayouts = &p_Layout;
			allocInfo.descriptorPool = m_CurrentPool;
			allocInfo.descriptorSetCount = 1;

			VkResult allocResult = vkAllocateDescriptorSets(m_Device, &allocInfo, p_DescriptorSet);
			bool reallocate = false;

			switch (allocResult) {
				case VK_SUCCESS:
					return true;
				case VK_ERROR_FRAGMENTED_POOL:
				case VK_ERROR_OUT_OF_POOL_MEMORY:
					reallocate = true;
					break;
				default:
					VGL_INTERNAL_ASSERT_ERROR("[VkDescriptorAllocator]Failed to allocate descriptor sets");
					return false;
			}

			if (reallocate) {
				m_CurrentPool = getPool();
				m_ActivePools.push_back(m_CurrentPool);

				allocResult = vkAllocateDescriptorSets(m_Device, &allocInfo, p_DescriptorSet);

				VGL_INTERNAL_ASSERT_ERROR(allocResult == VK_SUCCESS, "[VkDescriptorAllocator]Failed to allocate descriptor sets, VkResult: %i", allocResult);

				if (allocResult == VK_SUCCESS) return true;
			}

			VGL_INTERNAL_ASSERT_ERROR("[VkDescriptorAllocator]Failed to allocate descriptor sets");
			return false;
		}

		void DescriptorAllocator::cleanup()
		{
			for (auto& pool : m_FreePools)
				vkDestroyDescriptorPool(m_Device, pool, nullptr);
			for (auto& pool : m_ActivePools)
				vkDestroyDescriptorPool(m_Device, pool, nullptr);
		}

		VkDescriptorPool DescriptorAllocator::getPool()
		{
			if (!m_FreePools.empty()) {
				VkDescriptorPool pool = m_FreePools.back();
				m_FreePools.pop_back();
				return pool;
			}
			else
				return createPool(m_DescriptorSizes, 1000, 0);
		}

		VkDescriptorPool DescriptorAllocator::createPool(const DescriptorAllocator::PoolSizes& p_PoolSizes, uint32_t p_Count, VkDescriptorPoolCreateFlags p_Flags)
		{
			std::vector<VkDescriptorPoolSize> sizes;
			sizes.reserve(p_PoolSizes.sizes.size());

			for (auto& sz : p_PoolSizes.sizes)
				sizes.push_back({ sz.first, (uint32_t)(sz.second * p_Count) });

			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.flags = p_Flags;
			poolInfo.maxSets = p_Count;
			poolInfo.poolSizeCount = (uint32_t)sizes.size();
			poolInfo.pPoolSizes = sizes.data();

			VkDescriptorPool descriptorPool;
			vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &descriptorPool);

			return descriptorPool;
		}
	}
}
