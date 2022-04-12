#include "ShadowMap.h"

namespace vgl
{
	ShadowMap::ShadowMap
	(
		Vector2i p_Resolution,
		float p_DepthBiasConstant,
		float p_DepthBiasSlope
	) : m_Resolution(p_Resolution), m_DepthBiasConstant(p_DepthBiasConstant), m_DepthBiasSlope(p_DepthBiasSlope), m_ShadowMapRenderPass(vk::RenderPassType::Graphics)
	{
		Matrix4f projection = Matrix4f::orthoRH_ZO(-10000, 10000, -10000, 10000, 1, 15000);
		Matrix4f view = Matrix4f::lookAtRH({ 1000, 3000, 1000 }, { 0, 0, 0 }, { 0, 1, 0 });
		m_SMMVP = view * projection;

		vk::AttachmentInfo depthAttachment;
		depthAttachment.p_AttachmentType = vk::AttachmentType::Depth;
		depthAttachment.p_Format = VK_FORMAT_D16_UNORM;
		depthAttachment.p_SampleCount = 1;// m_CurrentWindow->maxMSAASamples;
		depthAttachment.p_LoadOp = vk::LoadOp::Clear;
		depthAttachment.p_StoreOp = vk::StoreOp::Store;
		depthAttachment.p_StencilLoadOp = vk::LoadOp::Null;
		depthAttachment.p_StencilStoreOp = vk::StoreOp::Null;
		depthAttachment.p_InitialLayout = vk::Layout::Undefined;
		depthAttachment.p_FinalLayout = vk::Layout::DepthR;
		
		vk::ImageAttachmentInfo info;
		info.p_AttachmentInfo = &depthAttachment;
		info.p_BorderColor = vk::BorderColor::OpaqueWhite;
		info.p_CreateSampler = true;
		info.p_TransitionLayoutImage = false;
		info.p_SamplerMode = vk::SamplerMode::ClampToEdge;
		info.p_Size = Vector2i(m_Resolution.x, m_Resolution.y);

		m_ShadowMapImageAttachment.create(info);

		vk::FramebufferInfo framebufferInfo;
		framebufferInfo.p_ImageAttachments = { m_ShadowMapImageAttachment };
		framebufferInfo.p_RenderPass = &m_ShadowMapRenderPass;
		framebufferInfo.p_Size = Vector2i(m_Resolution.x, m_Resolution.y);

		m_ShadowMapFramebuffer.create(framebufferInfo);

		m_ShadowMapInheritanceInfo = {};
		m_ShadowMapInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		//m_ShadowMapInheritanceInfo.renderPass = m_ShadowMapRenderPass.m_RenderPass;
		m_ShadowMapInheritanceInfo.framebuffer = m_ShadowMapFramebuffer.getVkFramebufferHandle();
		m_ShadowMapInheritanceInfo.subpass = 0;
	}
	
}