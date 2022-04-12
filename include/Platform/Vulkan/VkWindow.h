#pragma once

#include "VkContext.h"
#include "../BaseWindow.h"
#include "../../Math/Vector.h"

namespace vgl
{
	namespace vk
	{
		struct Swapchain
		{
			Swapchain(VkSurfaceKHR* p_Surface) : m_Surface(p_Surface), m_ContextPtr(&ContextSingleton::getInstance()) 
				{ m_SwapchainSupport = querySwapchainSupport(*m_Surface, m_ContextPtr->m_PhysicalDevice.m_VkHandle); };
			~Swapchain() {};

			Context* m_ContextPtr;
			VkSurfaceKHR* m_Surface;
			SwapchainSupportDetails m_SwapchainSupport;

			VkExtent2D		  m_SwapchainExtent;
			VkSwapchainKHR	  m_Swapchain;
			VkFormat		  m_SwapchainImageFormat;

			std::vector<VkImage>		m_SwapchainImages;
			std::vector<VkImageView>    m_SwapchainImageViews;
			std::vector<VkFramebuffer>  m_SwapchainFramebuffers;

			void destroy();

			void initFramebuffers(VkRenderPass& p_RenderPass, VkImageView& p_DepthImageView, VkImageView& p_ColorAttachmentView);

			void initSwapchain(const Vector2i p_Extent);

			void initImageViews();

			static SwapchainSupportDetails querySwapchainSupport(VkSurfaceKHR& p_Surface, VkPhysicalDevice p_Device);

			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& p_AvailableFormats);
			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> p_AvailablePresentModes);
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& p_Capabilites, const Vector2i p_Extent);
		};

		class Window : public vgl::BaseWindow
		{
		public:
			Window();
			Window(const Vector2i p_Size, const char* p_Title, const int p_MSAASamples);
			~Window();

			void destroy();

		protected:
		private:
			friend class Renderer;

			VkSampleCountFlagBits getMaxUsableSampleCount();
			void createColorResources();
			void destroyMSAAColorImage();

			uint32_t m_MSAASamples;

			Context* m_ContextPtr;
			VkSurfaceKHR m_Surface;

			Swapchain m_Swapchain;

			VkImage m_ColorImage;
			VkDeviceMemory m_ColorImageMemory;
			VkImageView m_ColorImageView;

			bool m_IsValid = false;

		};
	}
}