#pragma once

#include <iostream>
#include "../../Platform/Vulkan/Buffers/VulkanFramebuffer.h"
#include "../../Platform/Vulkan/Rendering/VulkanRenderPass.h"
#include "../../VGL-Core/Math/Matrix.h"

namespace vgl
{
	class ShadowMap
	{
		public:
			ShadowMap();
			ShadowMap(Vector2i p_Resolution, float p_DepthBiasConstant, float p_DepthBiasSlope);
			void setExtent(Vector3f p_Extent);
			void setDirection(Vector3f p_Direction);

		private:
			//std::vector<vk::UniformManager> m_SUBM;
			vk::RenderPass m_ShadowMapRenderPass;
			VkCommandBufferInheritanceInfo m_ShadowMapInheritanceInfo;
			vk::ImageAttachment m_ShadowMapImageAttachment;
			vk::Framebuffer m_ShadowMapFramebuffer;

			Vector2i m_Resolution;
			Matrix4f m_SMMVP;

			float m_DepthBiasConstant = 1.25f;
			float m_DepthBiasSlope = 1.75f;
	};
}