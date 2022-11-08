#include "VkWindow.h"

namespace vgl
{
	namespace vk
	{
		Window::Window() : vgl::BaseWindow(), m_ContextPtr(&ContextSingleton::getInstance()), m_MSAASamples(1), m_Swapchain(&m_Surface)
		{
			//Empty ctor
		}
		Window::~Window()
		{
			
		}
		void Window::destroy()
		{
			glfwTerminate();

			destroyMSAAColorImage();
			vkDestroySurfaceKHR(m_ContextPtr->m_Instance, m_Surface, nullptr);
		}

		Window::Window(const Vector2i p_Size, const char* p_Title, const int p_MSAASamples) :
			vgl::BaseWindow(p_Size, p_Title), m_ContextPtr(&ContextSingleton::getInstance()), m_MSAASamples(1), m_Swapchain(&m_Surface)
		{
			//creating the surface for a render context
			VkResult result = glfwCreateWindowSurface(m_ContextPtr->m_Instance, m_Window, nullptr, &m_Surface);
			
			#ifdef USE_LOGGING
			if (result != VK_SUCCESS)
				Utils::Logger::logMSG("Failed to create window surface", "Window", Utils::Severity::Error);
			else
				Utils::Logger::logMSG("Succesfully created window surface", "Window", Utils::Severity::Trace);
			#endif

			if (!m_ContextPtr->m_Initialized) {
				m_ContextPtr->setPhysicalDevice(m_ContextPtr->getSuitablePhysicalDevice(m_ContextPtr->getPhysicalDevices(m_Surface, m_Swapchain.m_SwapchainSupport)));
				m_ContextPtr->initLogicalDevice();
				m_ContextPtr->m_Initialized = true;
				m_ContextPtr->VMA_Init();
			}

			//Create the swap chain for the window
			m_Swapchain.initSwapchain(getWindowSize());
			m_ContextPtr->m_SwapchainImageCount = m_Swapchain.m_SwapchainImages.size();
			m_ContextPtr->m_DescriptorAllocators.resize(m_ContextPtr->m_SwapchainImageCount, DescriptorAllocator(m_ContextPtr->m_Device));

			m_MSAASamples = p_MSAASamples > getMaxUsableSampleCount() ? getMaxUsableSampleCount() : p_MSAASamples;

			#ifdef USE_LOGGING
			Utils::Logger::logMSG("MSAA : " + Utils::to_string(m_MSAASamples) + "x" + "  Max samples : " + Utils::to_string(getMaxUsableSampleCount()) + "x", "MSAA", Utils::Severity::Info);
			#endif

			createColorResources();
		}

		void Window::createColorResources()
		{
			VkFormat colorFormat = m_Swapchain.m_SwapchainImageFormat;

			m_AllocInfo = m_ContextPtr->createImageS
			(
				m_WindowSize.x, m_WindowSize.y,
				colorFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VMA_MEMORY_USAGE_GPU_ONLY,
				m_ColorImage,
				(VkSampleCountFlagBits)m_MSAASamples
			);

			m_ColorImageView = m_ContextPtr->createImageView(m_ColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		void Window::destroyMSAAColorImage()
		{
			vmaDestroyImage(m_ContextPtr->m_VmaAllocator, m_ColorImage, m_AllocInfo.p_Alloc);
			vkDestroyImageView(m_ContextPtr->m_Device, m_ColorImageView, nullptr);
		}

		VkSampleCountFlagBits Window::getMaxUsableSampleCount()
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(m_ContextPtr->m_PhysicalDevice.m_VkHandle, &physicalDeviceProperties);

			VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
			if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
			if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
			if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
			if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
			if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
			if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

			return VK_SAMPLE_COUNT_1_BIT;
		}

		void Swapchain::destroy()
		{
			for (auto& framebuffer : m_SwapchainFramebuffers)
				vkDestroyFramebuffer(m_ContextPtr->m_Device, framebuffer, nullptr);

			for (auto& imageView : m_SwapchainImageViews)
				vkDestroyImageView(m_ContextPtr->m_Device, imageView, nullptr);

			vkDestroySwapchainKHR(m_ContextPtr->m_Device, m_Swapchain, nullptr);
		}

		void Swapchain::initFramebuffers(VkRenderPass& p_RenderPass, VkImageView& p_DepthImageView, VkImageView& p_ColorAttachmentView)
		{
			m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());

			for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
			{
				VkImageView attachments[] = { p_ColorAttachmentView, p_DepthImageView, m_SwapchainImageViews[i] };

				VkFramebufferCreateInfo framebufferInfo = {};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = p_RenderPass;
				framebufferInfo.attachmentCount = 3;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = m_SwapchainExtent.width;
				framebufferInfo.height = m_SwapchainExtent.height;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer(m_ContextPtr->m_Device, &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]) != VK_SUCCESS)
					Utils::Logger::logMSG("Failed to create framebuffer", "Swap Chain", Utils::Severity::Fatal);
			}
		}

		void Swapchain::initSwapchain(const Vector2i p_Extent)
		{
			SwapchainSupportDetails SwapchainSupport = m_ContextPtr->querySwapchainSupport(*m_Surface, m_ContextPtr->m_PhysicalDevice.m_VkHandle);
			VkSurfaceFormatKHR		surfaceFormat = chooseSwapSurfaceFormat(SwapchainSupport.formats);
			VkPresentModeKHR		presentMode = chooseSwapPresentMode(SwapchainSupport.presentModes);
			VkExtent2D				extent = chooseSwapExtent(SwapchainSupport.capabilities, p_Extent);

			uint32_t imageCount = SwapchainSupport.capabilities.minImageCount;

			if (SwapchainSupport.capabilities.minImageCount > 0 && imageCount > SwapchainSupport.capabilities.maxImageCount)
				imageCount = SwapchainSupport.capabilities.maxImageCount;
			if (!(SwapchainSupport.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
				Utils::Logger::logMSG("Swap chain image does not support VK_IMAGE_USAGE_TRANSFER_DST_BIT usage", "Swap chain", Utils::Severity::Warning);

			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = *m_Surface;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			createInfo.minImageCount = imageCount;

			QueueFamilyIndices indices = m_ContextPtr->getQueueFamilyIndices(*m_Surface, m_ContextPtr->m_PhysicalDevice.m_VkHandle);
			uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

			if (indices.graphicsFamily != indices.presentFamily)
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 0;
				createInfo.pQueueFamilyIndices = nullptr;
			}
			createInfo.preTransform = SwapchainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			if (vkCreateSwapchainKHR(m_ContextPtr->m_Device, &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
				Utils::Logger::logMSG("Failed to create swap chain", "Swap Chain", Utils::Severity::Fatal);
			else
				Utils::Logger::logMSG("Succesfully created swap chain", "Swap Chain", Utils::Severity::Trace);

			vkGetSwapchainImagesKHR(m_ContextPtr->m_Device, m_Swapchain, &imageCount, nullptr);
			m_SwapchainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(m_ContextPtr->m_Device, m_Swapchain, &imageCount, m_SwapchainImages.data());

			m_SwapchainImageFormat = surfaceFormat.format;
			m_SwapchainExtent = extent;

			initImageViews();
		}
		void Swapchain::initImageViews()
		{
			m_SwapchainImageViews.resize(m_SwapchainImages.size());

			bool success = true;
			for (size_t i = 0; i < m_SwapchainImages.size(); i++)
			{
				VkImageViewCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = m_SwapchainImages[i];

				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = m_SwapchainImageFormat;

				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				if (vkCreateImageView(m_ContextPtr->m_Device, &createInfo, nullptr, &m_SwapchainImageViews[i]) != VK_SUCCESS) {
					Utils::Logger::logMSG("Failed to create image views", "Swap Chain", Utils::Severity::Error);
					success = false;
				}
				else if (success)
					success = true;
			}

			if (success)
				Utils::Logger::logMSG("Succesfully created image views for swap chain", "Swap Chain", Utils::Severity::Trace);
		}

		VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& p_AvailableFormats)
		{
			if (p_AvailableFormats.size() == 1 && p_AvailableFormats[0].format == VK_FORMAT_UNDEFINED)
				return { VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
			for (const auto& availableFormat : p_AvailableFormats)
			{
				if (availableFormat.format == VK_FORMAT_R16G16B16A16_SFLOAT && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					return availableFormat;
			}
			return p_AvailableFormats[0];
		}
		VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> p_AvailablePresentModes)
		{
			//for (const auto& availablePresentMode : p_AvailablePresentModes)
			//	if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR)
			//		return availablePresentMode;
			for (const auto& availablePresentMode : p_AvailablePresentModes)
				if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
					return availablePresentMode;
			//for (const auto& availablePresentMode : p_AvailablePresentModes)
			//	if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			//		return availablePresentMode;
		}
		VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& p_Capabilities, const Vector2i p_Extent)
		{
			#ifdef WIN32
				#undef min
				#undef max
			#endif

			if (p_Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
				return p_Capabilities.currentExtent;
			else
			{
				VkExtent2D actualExtent = { static_cast<uint32_t>(p_Extent.x), static_cast<uint32_t>(p_Extent.y) };

				actualExtent.width = std::max(p_Capabilities.minImageExtent.width, std::min(p_Capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = std::max(p_Capabilities.minImageExtent.height, std::min(p_Capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}
	}
}