#include "..\include\RenderPipelines.h"
#include "..\include\RenderPipelines.h"
#include "..\include\RenderPipelines.h"
#include "..\include\RenderPipelines.h"
#include "RenderPipelines.h"

namespace vgl
{
	RenderPipeline_Forward::RenderPipeline_Forward()
	{

	}
	RenderPipeline_Forward::~RenderPipeline_Forward()
	{

	}
	void RenderPipeline_Forward::setup(Renderer* p_Renderer, vk::Window* p_Window, Scene* p_Scene)
	{
		m_RendererPtr = p_Renderer;
		m_WindowPtr = p_Window;
		m_ScenePtr = p_Scene;

		// Create main color framebuffer
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_RectangleData = const_cast<vk::VertexArray*>(&vk::GraphicsContext::getRecVao());
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		// RGBA = Position, A = Depth
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_4C,
			vk::Layout::ShaderR,
			true, true, true
		);
		// Depth
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::D32SF_1C,
			vk::Layout::Depth, true
		);
		m_LightPassFramebuffer.create();

		//vk::GraphicsContext::createSkyboxPipelines(*m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_GraphicsPipelineInfo.p_RenderPass);

		static vk::DescriptorSetInfo infoHDR;
		infoHDR.p_StorageBuffer = vk::StorageBuffer(vk::ShaderStage::FragmentBit, sizeof(float) * 2, 2);
		infoHDR.p_FragmentUniformBuffer = vk::UniformBuffer(sizeof(m_DOFInfo), 1);
		for (int32_t i = 0; i < m_LightPassFramebuffer.getImageAttachment().size(); i++)
			infoHDR.p_ImageDescriptors.emplace_back(i, &m_LightPassFramebuffer.getImageAttachment()[i][0].getImage(), 0);
		m_HDRFramebuffer.getDescriptors().create(infoHDR);
		m_HDRFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, m_DOFInfo, 0);

		// Create Post-Processing framebuffer
		m_HDRShader.setShader("data/Shaders/HDR/vert.spv", "data/Shaders/HDR/frag.spv");
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_RectangleData = const_cast<vk::VertexArray*>(&vk::GraphicsContext::getRecVao());
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_HDRShader;
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantData = &m_HDRInfo;
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantSize = sizeof(m_HDRInfo);
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantShaderStage = vk::ShaderStage::FragmentBit;
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_CreateGraphicsPipeline = true;
		auto& info = m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_4C,
			vk::Layout::ShaderR,
			vk::BorderColor::OpaqueBlack,
			vk::SamplerMode::ClampToBorder,
			true, true, false
		);
		info.p_Channels = 4; // Set for read pixels
		info.p_Bytes = 4; // Set for read pixels
		m_HDRFramebuffer.create();
	}
	void RenderPipeline_Forward::render(vk::RenderInfo& p_RenderInfo)
	{
		m_HDRInfo.deltaTime = m_WindowPtr->getDeltaTime();
		m_HDRFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, m_DOFInfo, 0);

		// Render into shadow map
		for (auto& entity : m_ScenePtr->getEntities()) {
			auto shadow_map = m_ScenePtr->getComponent<DShadowMapComponent>(entity);
			if (shadow_map) {
				m_RendererPtr->beginRenderPass(p_RenderInfo, shadow_map->ShadowMap);
				for (auto& entity : m_ScenePtr->getEntities()) {
					auto mesh = m_ScenePtr->getComponent<Mesh3DComponent>(entity);
					auto transform = m_ScenePtr->getComponent<Transform3DComponent>(entity);
					if (!mesh) continue;
					if (!transform) continue;
					if (mesh->mesh)
						m_RendererPtr->submit(*mesh->mesh, transform->transform);
				}
				m_RendererPtr->endRenderPass();
			}
		}

		m_RendererPtr->beginRenderPass(p_RenderInfo, m_LightPassFramebuffer);
		for (auto& entity : m_ScenePtr->getEntities()) {
			auto directional_light = m_ScenePtr->getComponent<DirectionalLight3DComponent>(entity);
			auto point_light = m_ScenePtr->getComponent<PointLight3DComponent>(entity);
			auto spot_light = m_ScenePtr->getComponent<SpotLight3DComponent>(entity);
		
			if (directional_light || point_light || spot_light) {
				auto shadow_map = m_ScenePtr->getComponent<DShadowMapComponent>(entity);
				if (shadow_map)
					m_RendererPtr->submit(shadow_map->ShadowMap);
			}
		}
		for (auto& entity : m_ScenePtr->getEntities()) {
			auto skybox = m_ScenePtr->getComponent<SkyboxComponent>(entity);
			if (skybox) {
				if(skybox->EnvironmentData.p_IrradianceMap && skybox->EnvironmentData.p_PrefilteredMap && skybox->EnvironmentData.p_BRDFLut)
					if(skybox->EnvironmentData.p_IrradianceMap->isComplete() && skybox->EnvironmentData.p_PrefilteredMap->isComplete() && skybox->EnvironmentData.p_BRDFLut->isComplete())
						m_RendererPtr->submit(*skybox->EnvironmentData.p_IrradianceMap, *skybox->EnvironmentData.p_PrefilteredMap, *skybox->EnvironmentData.p_BRDFLut);
				if(skybox->AtmosphericScattering)
					m_RendererPtr->submit(skybox->SkyBox, skybox->AtmosphericScatteringInfo);
			}
		}
		for (auto& entity : m_ScenePtr->getEntities()) {
			auto mesh = m_ScenePtr->getComponent<Mesh3DComponent>(entity);
			auto transform = m_ScenePtr->getComponent<Transform3DComponent>(entity);
			if (!mesh) continue;
			if (!transform) continue;
			if (mesh->mesh)
				m_RendererPtr->submit(*mesh->mesh, transform->transform);
		}
		m_RendererPtr->endRenderPass();

		for (auto& img : m_LightPassFramebuffer.getImageAttachment())
			m_RendererPtr->blitImage(img[0].getImage());

		m_RendererPtr->beginRenderPass(p_RenderInfo, m_HDRFramebuffer);
		m_RendererPtr->endRenderPass();
	}
	RenderPipeline_Deferred::RenderPipeline_Deferred()
	{
	}
	RenderPipeline_Deferred::~RenderPipeline_Deferred()
	{
	}
	void RenderPipeline_Deferred::setup(Renderer* p_Renderer, vk::Window* p_Window, Scene* p_Scene)
	{
		m_RendererPtr = p_Renderer;
		m_WindowPtr = p_Window;
		m_ScenePtr = p_Scene;

		/////////////////////////////////////////////////////////////////////////////////////////////
		//				Geometry Buffer
		/////////////////////////////////////////////////////////////////////////////////////////////
		m_GBuffer.m_FramebufferAttachmentInfo.p_RectangleData = const_cast<vk::VertexArray*>(&vk::GraphicsContext::getRecVao());
		m_GBuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		// RGBA = Position
		m_GBuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_4C,
			vk::Layout::ShaderR,
			true, true
		);
		// RGBA = Normal
		m_GBuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_4C,
			vk::Layout::ShaderR,
			true, true
		);
		// RGBA = Albedo
		m_GBuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_4C,
			vk::Layout::ShaderR,
			true, true
		);
		// RGBA = (R = Metallic) (G = Roughness) (B = Ambient Occlussion) (A = Depth)
		m_GBuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_4C,
			vk::Layout::ShaderR,
			true, true
		);
		// Depth
		m_GBuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::D32SF_1C,
			vk::Layout::DepthR, true, true
		);
		m_GBuffer.create();

		vk::GraphicsContext::createShapesPipelines(*m_GBuffer.m_FramebufferAttachmentInfo.p_GraphicsPipelineInfo.p_RenderPass);
		vk::GraphicsContext::createSkyboxPipelines(*m_GBuffer.m_FramebufferAttachmentInfo.p_GraphicsPipelineInfo.p_RenderPass);

		/////////////////////////////////////////////////////////////////////////////////////////////
		//				Screen-Space-Ambient-Occlusion
		/////////////////////////////////////////////////////////////////////////////////////////////
		for (uint32_t i = 0; i < Sample_Count; i++) {
			Vector4f sample(
				Math::getRandomNumber(0.0f, 1.0f) * 2.0f - 1.0f,
				Math::getRandomNumber(0.0f, 1.0f) * 2.0f - 1.0f,
				Math::getRandomNumber(0.0f, 1.0f), 0.0f
			);
			sample = Math::normalize(sample);
			sample = sample * Math::getRandomNumber(0.0f, 1.0f);
			float scale = (float)i / Sample_Count;
			scale = Math::lerp(0.1f, 1.0f, scale * scale);
			sample = sample * scale;
			SSAO_Kernel.push_back(sample);
		}
		
		for (uint32_t i = 0; i < 16; i++) {
			SSAO_Noise.emplace_back(
				Math::getRandomNumber(0.0f, 1.0f) * 2.0f - 1.0f,
				Math::getRandomNumber(0.0f, 1.0f) * 2.0f - 1.0f,
				0.0f, 0.0f
			);
		}
		
		SSAO_NoiseImage.initImage({ 4, 4 }, (unsigned char*)SSAO_Noise.data(), vk::Channels::RGBA, vk::SamplerMode::Repeat, false, vk::Filter::Nearest, vk::Filter::Nearest);
		
		static vk::DescriptorSetInfo infoSSAO;
		infoSSAO.p_FragmentUniformBuffer = vk::UniformBuffer(sizeof(Vector4f) * Sample_Count + 2*sizeof(Matrix4f) + sizeof(Vector4f), 0);
		for (int32_t i = 0; i < m_GBuffer.getImageAttachment().size(); i++) {
			infoSSAO.p_ImageDescriptors.emplace_back(i, &m_GBuffer.getImageAttachment()[i][0].getImage(), 1);
			infoSSAO.p_ImageDescriptors.emplace_back(i, &m_GBuffer.getImageAttachment()[i][1].getImage(), 2);
		}
		infoSSAO.p_ImageDescriptors.emplace_back(&SSAO_NoiseImage, 3);
		
		m_SSAOFramebuffer.getDescriptors().create(infoSSAO);
		m_SSAOFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, SSAO_Kernel.data(), SSAO_Kernel.size() * sizeof(Vector4f), 2 * sizeof(Vector4f) + sizeof(Matrix4f));
		
		// Create Post-Processing framebuffer
		m_SSAOShader.setShader("data/Shaders/SSAO/vert.spv", "data/Shaders/SSAO/frag.spv");
		m_SSAOFramebuffer.m_FramebufferAttachmentInfo.p_RectangleData = const_cast<vk::VertexArray*>(&vk::GraphicsContext::getRecVao());
		m_SSAOFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_SSAOFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_SSAOShader;
		m_SSAOFramebuffer.m_FramebufferAttachmentInfo.p_CreateGraphicsPipeline = true;
		m_SSAOFramebuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_1C,
			vk::Layout::ShaderR,
			vk::BorderColor::OpaqueBlack,
			vk::SamplerMode::ClampToBorder,
			true, true, false
		);
		m_SSAOFramebuffer.create();
		
		static vk::DescriptorSetInfo infoSSAOBlur;
		for (int32_t i = 0; i < m_SSAOFramebuffer.getImageAttachment().size(); i++)
			infoSSAOBlur.p_ImageDescriptors.emplace_back(i, &m_SSAOFramebuffer.getImageAttachment()[i][0].getImage(), 0);

		m_SSAOBlurFramebuffer.getDescriptors().create(infoSSAOBlur);
		m_SSAOBlurShader.setShader("data/Shaders/SSAO/Blur/vert.spv", "data/Shaders/SSAO/Blur/frag.spv");
		m_SSAOBlurFramebuffer.m_FramebufferAttachmentInfo.p_RectangleData = const_cast<vk::VertexArray*>(&vk::GraphicsContext::getRecVao());
		m_SSAOBlurFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_SSAOBlurFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_SSAOBlurShader;
		m_SSAOBlurFramebuffer.m_FramebufferAttachmentInfo.p_CreateGraphicsPipeline = true;
		m_SSAOBlurFramebuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_1C,
			vk::Layout::ShaderR,
			vk::BorderColor::OpaqueBlack,
			vk::SamplerMode::ClampToBorder,
			true, true, false
		);
		m_SSAOBlurFramebuffer.create();

		/////////////////////////////////////////////////////////////////////////////////////////////
		//				Lighting Pass
		/////////////////////////////////////////////////////////////////////////////////////////////
		// Create main color framebuffer
		ShaderInfo shader_info;
		shader_info.p_LightingType = (uint32_t)LightingType::PBR;
		shader_info.p_DirectionalLight = true;

		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_RectangleData = const_cast<vk::VertexArray*>(&vk::GraphicsContext::getRecVao());
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_GBufferLightPassShader;
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_CreateGraphicsPipeline = true;
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantShaderStage = vk::ShaderStage::FragmentBit;
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantData = &volumetric_light_mie_scattering;
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantSize = sizeof(float);
		// RGBA = Position, A = Depth
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_4C,
			vk::Layout::ShaderR,
			true, true, true
		);
		static vk::DescriptorSetInfo infoLightPass;
		infoLightPass.p_FragmentUniformBuffer = vk::UniformBuffer(3 * sizeof(Vector4f) + sizeof(Matrix4f) + sizeof(P_Light) * 500, 0);
		for (int32_t i = 0; i < m_GBuffer.getImageAttachment().size(); i++) {
			infoLightPass.p_ImageDescriptors.emplace_back(i, &m_GBuffer.getImageAttachment()[i][0].getImage(), 1);
			infoLightPass.p_ImageDescriptors.emplace_back(i, &m_GBuffer.getImageAttachment()[i][1].getImage(), 2);
			infoLightPass.p_ImageDescriptors.emplace_back(i, &m_GBuffer.getImageAttachment()[i][2].getImage(), 3);
			infoLightPass.p_ImageDescriptors.emplace_back(i, &m_GBuffer.getImageAttachment()[i][3].getImage(), 4);
			infoLightPass.p_ImageDescriptors.emplace_back(i, &m_SSAOBlurFramebuffer.getImageAttachment()[i][0].getImage(), 5);
			//infoLightPass.p_ImageDescriptors.emplace_back(i, &m_SSRFramebuffer.getImageAttachment()[i][0].getImage(), 6);
			for (auto& entity : m_ScenePtr->getEntities()) {
				auto d_shadow_map = m_ScenePtr->getComponent<DShadowMapComponent>(entity);
				if (d_shadow_map) {
					infoLightPass.p_ImageDescriptors.emplace_back(i, &d_shadow_map->ShadowMap.m_Attachment.getImageAttachment()[i][0].getImage(), 6, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
					shader_info.p_Effects |= (uint32_t)Effects::ShadowMapping;
				}
			}
		}
		m_LightPassFramebuffer.getDescriptors().create(infoLightPass);
		m_GBufferLightPassShader.compile(ShaderPermutationsGenerator::generateShaderGBufferLightPass(shader_info));
		m_LightPassFramebuffer.create();

		/////////////////////////////////////////////////////////////////////////////////////////////
		//				Screen-Space-Reflections
		/////////////////////////////////////////////////////////////////////////////////////////////
		static vk::DescriptorSetInfo infoSSR;
		infoSSR.p_FragmentUniformBuffer = vk::UniformBuffer(4 * sizeof(float) + 2 * sizeof(Matrix4f) + sizeof(Vector4f), 0);
		for (int32_t i = 0; i < m_GBuffer.getImageAttachment().size(); i++) {
			infoSSR.p_ImageDescriptors.emplace_back(i, &m_LightPassFramebuffer.getImageAttachment()[i][0].getImage(), 1);
			infoSSR.p_ImageDescriptors.emplace_back(i, &m_GBuffer.getImageAttachment()[i][4].getImage(), 2, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL);
			infoSSR.p_ImageDescriptors.emplace_back(i, &m_GBuffer.getImageAttachment()[i][1].getImage(), 3);
			infoSSR.p_ImageDescriptors.emplace_back(i, &m_GBuffer.getImageAttachment()[i][3].getImage(), 4);
		}
		
		m_SSRFramebuffer.getDescriptors().create(infoSSR);
		
		// Create SSR framebuffer
		m_SSRShader.setShader("data/Shaders/SSR/vert.spv", "data/Shaders/SSR/frag.spv");
		m_SSRFramebuffer.m_FramebufferAttachmentInfo.p_RectangleData = const_cast<vk::VertexArray*>(&vk::GraphicsContext::getRecVao());
		m_SSRFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_SSRFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_SSRShader;
		m_SSRFramebuffer.m_FramebufferAttachmentInfo.p_CreateGraphicsPipeline = true;
		m_SSRFramebuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_4C,
			vk::Layout::ShaderR,
			vk::BorderColor::OpaqueBlack,
			vk::SamplerMode::ClampToBorder,
			true, true, false
		);
		m_SSRFramebuffer.create();

		/////////////////////////////////////////////////////////////////////////////////////////////
		//				Hight-Definition-Range & Post-Processing
		/////////////////////////////////////////////////////////////////////////////////////////////
		static vk::DescriptorSetInfo infoHDR;
		infoHDR.p_StorageBuffer = vk::StorageBuffer(vk::ShaderStage::FragmentBit, sizeof(float) * 2, 2);
		infoHDR.p_FragmentUniformBuffer = vk::UniformBuffer(sizeof(m_DOFInfo), 1);
		for (int32_t i = 0; i < m_LightPassFramebuffer.getImageAttachment().size(); i++)
			infoHDR.p_ImageDescriptors.emplace_back(i, &m_LightPassFramebuffer.getImageAttachment()[i][0].getImage(), 0);
		m_HDRFramebuffer.getDescriptors().create(infoHDR);
		m_HDRFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, m_DOFInfo, 0);

		// Create Post-Processing framebuffer
		m_HDRShader.setShader("data/Shaders/HDR/vert.spv", "data/Shaders/HDR/frag.spv");
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_RectangleData = const_cast<vk::VertexArray*>(&vk::GraphicsContext::getRecVao());
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_HDRShader;
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantData = &m_HDRInfo;
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantSize = sizeof(m_HDRInfo);
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantShaderStage = vk::ShaderStage::FragmentBit;
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_CreateGraphicsPipeline = true;
		auto& info = m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
			p_Window->getWindowSize(),
			vk::ImageFormat::C16SF_4C,
			vk::Layout::ShaderR,
			vk::BorderColor::OpaqueBlack,
			vk::SamplerMode::ClampToBorder,
			true, true, false
		);
		info.p_Channels = 4; // Set for read pixels
		info.p_Bytes = 4; // Set for read pixels
		m_HDRFramebuffer.create();
	}
	void RenderPipeline_Deferred::render(vk::RenderInfo& p_RenderInfo)
	{
		m_LightPassFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, m_RendererPtr->getCurrentCameraPtr()->getPosition(), 0);
		float point_light_count = 0;
		static P_Light p_light;

		m_SSAOFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, SSAO_Intensity, 0);
		m_SSAOFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, SSAO_Radius, sizeof(float));
		m_SSAOFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, SSAO_Bias, 2*sizeof(float));
		float kernel_size = Sample_Count;
		m_SSAOFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, kernel_size, 3*sizeof(float));
		m_SSAOFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, m_RendererPtr->getCurrentCameraPtr()->getPerspectiveMatrix(), sizeof(Vector4f));
		m_SSAOFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, m_RendererPtr->getViewProjection() , sizeof(Vector4f) + sizeof(Matrix4f));
		
		m_SSRFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, SSR_Data, 0);
		m_SSRFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, m_RendererPtr->getCurrentCameraPtr()->getViewDirection(), 4 * sizeof(float));
		m_SSRFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, m_RendererPtr->getCurrentCameraPtr()->getPerspectiveMatrix(), 4*sizeof(float) + sizeof(Vector4f));
		m_SSRFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, m_RendererPtr->getViewProjection() , 4 * sizeof(float) + sizeof(Matrix4f) + sizeof(Vector4f));

		for (auto& entity : m_ScenePtr->getEntities()) {
			auto directional_light = m_ScenePtr->getComponent<DirectionalLight3DComponent>(entity);
			auto d_shadow_map = m_ScenePtr->getComponent<DShadowMapComponent>(entity);
			if (directional_light && d_shadow_map) {
				volumetric_light_mie_scattering = directional_light->VolumetricLightDensity;
				m_LightPassFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, d_shadow_map->ShadowMap.m_View * d_shadow_map->ShadowMap.m_Projection, sizeof(Vector4f));
				m_LightPassFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, d_shadow_map->ShadowMap.m_Direction, sizeof(Matrix4f) + sizeof(Vector4f));
				m_LightPassFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, directional_light->Color, sizeof(Matrix4f) + 2 * sizeof(Vector4f));
			}
			auto point_light = m_ScenePtr->getComponent<PointLight3DComponent>(entity);
			if (point_light) {
				p_light.Color.xyz = point_light->Color;
				p_light.Position.xyz = point_light->Position;
				p_light.Radius = point_light->Radius;
				p_light.m_ShadowMapID = point_light->ShadowMapID;
				m_LightPassFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, p_light, 3 * sizeof(Vector4f) + sizeof(Matrix4f) + sizeof(P_Light) * point_light_count);
				point_light_count++;
			}
		}

		m_LightPassFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, point_light_count, 3 * sizeof(float));

		m_HDRInfo.deltaTime = m_WindowPtr->getDeltaTime();
		m_HDRFramebuffer.getDescriptors().copy(vk::ShaderStage::FragmentBit, m_DOFInfo, 0);

		// Render into shadow map
		for (auto& entity : m_ScenePtr->getEntities()) {
			auto shadow_map = m_ScenePtr->getComponent<DShadowMapComponent>(entity);
			if (shadow_map) {
				m_RendererPtr->beginRenderPass(p_RenderInfo, shadow_map->ShadowMap);
				for (auto& entity : m_ScenePtr->getEntities()) {
					auto mesh = m_ScenePtr->getComponent<Mesh3DComponent>(entity);
					auto transform = m_ScenePtr->getComponent<Transform3DComponent>(entity);
					if (!mesh) continue;
					if (!transform) continue;
					if (mesh->mesh)
						m_RendererPtr->submit(*mesh->mesh, transform->transform);
				}
				m_RendererPtr->endRenderPass();
			}
		}

		m_RendererPtr->beginRenderPass(p_RenderInfo, m_GBuffer);

		for (auto& entity : m_ScenePtr->getEntities()) {
			auto directional_light = m_ScenePtr->getComponent<DirectionalLight3DComponent>(entity);
			auto point_light = m_ScenePtr->getComponent<PointLight3DComponent>(entity);
			auto spot_light = m_ScenePtr->getComponent<SpotLight3DComponent>(entity);
		
			if (directional_light) {
				auto shadow_map = m_ScenePtr->getComponent<DShadowMapComponent>(entity);
				if (shadow_map)
					m_RendererPtr->submit(shadow_map->ShadowMap);
			}

			auto skybox = m_ScenePtr->getComponent<SkyboxComponent>(entity);
			if (skybox) {
				if (skybox->EnvironmentData.p_IrradianceMap && skybox->EnvironmentData.p_PrefilteredMap && skybox->EnvironmentData.p_BRDFLut)
					if (skybox->EnvironmentData.p_IrradianceMap->isComplete() && skybox->EnvironmentData.p_PrefilteredMap->isComplete() && skybox->EnvironmentData.p_BRDFLut->isComplete())
						m_RendererPtr->submit(*skybox->EnvironmentData.p_IrradianceMap, *skybox->EnvironmentData.p_PrefilteredMap, *skybox->EnvironmentData.p_BRDFLut);
				if (skybox->AtmosphericScattering)
					m_RendererPtr->submit(skybox->SkyBox, skybox->AtmosphericScatteringInfo);
			}

			auto mesh = m_ScenePtr->getComponent<Mesh3DComponent>(entity);
			auto transform = m_ScenePtr->getComponent<Transform3DComponent>(entity);
			if (!mesh) continue;
			if (!transform) continue;
			if (mesh->mesh)
				m_RendererPtr->submit(*mesh->mesh, transform->transform);
		}
		m_RendererPtr->endRenderPass();

		// SSAO
		m_RendererPtr->beginRenderPass(p_RenderInfo, m_SSAOFramebuffer);
		m_RendererPtr->endRenderPass();
		
		m_RendererPtr->beginRenderPass(p_RenderInfo, m_SSAOBlurFramebuffer);
		m_RendererPtr->endRenderPass();
		
		m_RendererPtr->beginRenderPass(p_RenderInfo, m_SSRFramebuffer);
		m_RendererPtr->endRenderPass();
		
		// Light pass
		m_RendererPtr->beginRenderPass(p_RenderInfo, m_LightPassFramebuffer);
		m_RendererPtr->endRenderPass();

		for (auto& img : m_LightPassFramebuffer.getImageAttachment())
			m_RendererPtr->blitImage(img[0].getImage());

		m_RendererPtr->beginRenderPass(p_RenderInfo, m_HDRFramebuffer);
		m_RendererPtr->endRenderPass();
	}
	vk::FramebufferAttachment& RenderPipeline_Deferred::getGBloomFramebuffer(Vector2i p_Size)
	{
		// TODO: insert return statement here
		return vk::FramebufferAttachment();
	}
	void RenderPipeline_Deferred::setupGBuffer()
	{
	}
	void RenderPipeline_Deferred::setupSSAOBuffers()
	{
	}
	void RenderPipeline_Deferred::setupLightPassBuffer()
	{
	}
	void RenderPipeline_Deferred::setupPostProcessBuffers()
	{
	}
}
