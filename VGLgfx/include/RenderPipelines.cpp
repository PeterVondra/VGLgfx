#include "RenderPipelines.h"
#include "Platform/Definitions.h"
#include "Platform/Vulkan/VkGraphics_Internal.h"


namespace vgl
{
	RenderPipeline_Deferred::RenderPipeline_Deferred()
	{
	}
	RenderPipeline_Deferred::~RenderPipeline_Deferred()
	{
	}
	void RenderPipeline_Deferred::setup(Renderer* p_Renderer, Window* p_Window, Scene* p_Scene)
	{
		m_RendererPtr = p_Renderer;
		m_WindowPtr = p_Window;
		m_ScenePtr = p_Scene;

		/////////////////////////////////////////////////////////////////////////////////////////////
		//				Geometry Buffer
		/////////////////////////////////////////////////////////////////////////////////////////////
		m_GBuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_GBuffer.m_FramebufferAttachmentInfo.p_RenderPipelineInfo.p_CreateGraphicsPipeline = false;

		// RGBA = Position
		m_GBuffer.addAttachment(p_Window->getWindowSize(), ImageFormat::C16SF_4C, Layout::ShaderR);
		// RGBA = Normal
		m_GBuffer.addAttachment(p_Window->getWindowSize(), ImageFormat::C16SF_4C, Layout::ShaderR);
		// RGBA = Albedo
		m_GBuffer.addAttachment(p_Window->getWindowSize(), ImageFormat::C16SF_4C, Layout::ShaderR);
		// RGBA = (R = Metallic) (G = Roughness) (B = Ambient Occlussion) (A = Depth)
		m_GBuffer.addAttachment(p_Window->getWindowSize(), ImageFormat::C16SF_4C, Layout::ShaderR);
		// Depth
		m_GBuffer.addAttachment(p_Window->getWindowSize(), ImageFormat::D32SF, Layout::DepthR);
		m_GBuffer.create();

		//vk::GCS::getInstance().createShapesPipelines(*m_GBuffer.m_FramebufferAttachmentInfo.p_GraphicsPipelineInfo.p_RenderPass);
		vk::GCS::getInstance().createSkyboxPipelines(*m_GBuffer.m_FramebufferAttachmentInfo.p_RenderPipelineInfo.p_RenderPass);

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
		
		SSAO_NoiseImage.create({ 4, 4 }, (unsigned char*)SSAO_Noise.data(), Channels::RGBA, SamplerMode::Repeat, false, Filter::Nearest, Filter::Nearest);
		
		static vk::DescriptorSetInfo infoSSAO;
		infoSSAO.p_FragmentUniformBuffer = UniformBuffer(sizeof(Vector4f) * Sample_Count + 2*sizeof(Matrix4f) + sizeof(Vector4f), 0);
		for (int32_t i = 0; i < m_GBuffer.getImageAttachments().size(); i++) {
			infoSSAO.addImage(&m_GBuffer.getImageAttachments()[i][0].getImage(), 1, i);
			infoSSAO.addImage(&m_GBuffer.getImageAttachments()[i][1].getImage(), 2, i);
		}
		infoSSAO.addImage(&SSAO_NoiseImage, 3);
		
		m_SSAOFramebuffer.getDescriptors().create(infoSSAO);
		m_SSAOFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, SSAO_Kernel.data(), SSAO_Kernel.size() * sizeof(Vector4f), 2 * sizeof(Vector4f) + sizeof(Matrix4f));
		
		// Create Post-Processing framebuffer
		m_SSAOShader.setShader("data/Shaders/SSAO/vert.spv", "data/Shaders/SSAO/frag.spv");
		m_SSAOFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_SSAOFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_SSAOShader;
		m_SSAOFramebuffer.addAttachment(p_Window->getWindowSize(), ImageFormat::C16SF_1C, Layout::ShaderR);
		m_SSAOFramebuffer.create();
		
		static ShaderDescriptorInfo infoSSAOBlur;
		for (int32_t i = 0; i < m_SSAOFramebuffer.getImageAttachments().size(); i++)
			infoSSAOBlur.addImage(&m_SSAOFramebuffer.getImageAttachments()[i][0].getImage(), 0, i);

		m_SSAOBlurFramebuffer.getDescriptors().create(infoSSAOBlur);
		m_SSAOBlurShader.setShader("data/Shaders/SSAO/Blur/vert.spv", "data/Shaders/SSAO/Blur/frag.spv");
		m_SSAOBlurFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_SSAOBlurFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_SSAOBlurShader;
		m_SSAOBlurFramebuffer.addAttachment(p_Window->getWindowSize(), ImageFormat::C16SF_1C, Layout::ShaderR);
		m_SSAOBlurFramebuffer.create();

		/////////////////////////////////////////////////////////////////////////////////////////////
		//				Lighting Pass
		/////////////////////////////////////////////////////////////////////////////////////////////
		// Create main color framebuffer
		ShaderInfo shader_info;
		shader_info.p_LightingType = (uint32_t)LightingType::PBR;
		for (auto& entity : m_ScenePtr->getEntities()) {
			auto directional_light = m_ScenePtr->getComponent<DirectionalLight3DComponent>(entity);
			shader_info.p_DirectionalLight = directional_light != nullptr;
		}

		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_GBufferLightPassShader;
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantShaderStage = ShaderStage::FragmentBit;
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantData = &volumetric_light_mie_scattering;
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantSize = sizeof(float);
		// RGBA = Position, A = Depth
		m_LightPassFramebuffer.addAttachment(p_Window->getWindowSize(), ImageFormat::C16SF_4C, Layout::ShaderR, true, true);
		m_LightPassFramebuffer.m_FramebufferAttachmentInfo.p_AttachmentDescriptors[0].p_AllowMipMapping = true;

		static ShaderDescriptorInfo infoLightPass;
		infoLightPass.p_FragmentUniformBuffer = UniformBuffer(3 * sizeof(Vector4f) + sizeof(Matrix4f) + sizeof(P_Light) * 500, 0);
		for (int32_t i = 0; i < m_GBuffer.getImageAttachments().size(); i++) {
			infoLightPass.addImage(&m_GBuffer.getImageAttachments()[i][0].getImage(), 1, i);
			infoLightPass.addImage(&m_GBuffer.getImageAttachments()[i][1].getImage(), 2, i);
			infoLightPass.addImage(&m_GBuffer.getImageAttachments()[i][2].getImage(), 3, i);
			infoLightPass.addImage(&m_GBuffer.getImageAttachments()[i][3].getImage(), 4, i);
			infoLightPass.addImage(&m_SSAOBlurFramebuffer.getImageAttachments()[i][0].getImage(), 5, i);
			//infoLightPass.addImage(i, &m_SSRFramebuffer.getImageAttachment()[i][0].getImage(), 6);
			for (auto& entity : m_ScenePtr->getEntities()) {
				auto d_shadow_map = m_ScenePtr->getComponent<DShadowMapComponent>(entity);
				if (d_shadow_map) {
					infoLightPass.addImage(&d_shadow_map->ShadowMap.m_Attachment.getImageAttachments()[i][0].getImage(), 6, i);
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
		//static ShaderDescriptorInfo infoSSR;
		//infoSSR.p_FragmentUniformBuffer = vk::UniformBuffer(4 * sizeof(float) + 2 * sizeof(Matrix4f) + sizeof(Vector4f), 0);
		//for (int32_t i = 0; i < m_GBuffer.getImageAttachments().size(); i++) {
		//	infoSSR.addImage(&m_LightPassFramebuffer.getImageAttachments()[i][0].getImage(), 1, i);
		//	infoSSR.addImage(&m_GBuffer.getImageAttachments()[i][4].getImage(), 2, Layout::DepthR, i);
		//	infoSSR.addImage(&m_GBuffer.getImageAttachments()[i][1].getImage(), 3, i);
		//	infoSSR.addImage(&m_GBuffer.getImageAttachments()[i][3].getImage(), 4, i);
		//}
		//
		//m_SSRFramebuffer.getDescriptors().create(infoSSR);
		//
		//// Create SSR framebuffer
		//m_SSRShader.setShader("data/Shaders/SSR/vert.spv", "data/Shaders/SSR/frag.spv");
		//m_SSRFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		//m_SSRFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_SSRShader;
		//m_SSRFramebuffer.addAttachment(
		//	p_Window->getWindowSize(),
		//	ImageFormat::C16SF_4C,
		//	Layout::ShaderR
		//);
		//m_SSRFramebuffer.create();

		/////////////////////////////////////////////////////////////////////////////////////////////
		//				Hight-Definition-Range & Post-Processing
		/////////////////////////////////////////////////////////////////////////////////////////////
		static ShaderDescriptorInfo infoHDR;
		infoHDR.p_FragmentUniformBuffer = UniformBuffer(sizeof(m_DOFInfo), 1);
		infoHDR.p_StorageBuffer = StorageBuffer(ShaderStage::FragmentBit, sizeof(float) * 2, 2);
		for (int32_t i = 0; i < m_LightPassFramebuffer.getImageAttachments().size(); i++)
			infoHDR.addImage(&m_LightPassFramebuffer.getImageAttachments()[i][0].getImage(), 0, i);
		m_HDRFramebuffer.getDescriptors().create(infoHDR);
		m_HDRFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, m_DOFInfo, 0);

		// Create Post-Processing framebuffer
		m_HDRShader.setShader("data/Shaders/HDR/vert.spv", "data/Shaders/HDR/frag.spv");
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_Size = p_Window->getWindowSize();
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_Shader = &m_HDRShader;
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantData = &m_HDRInfo;
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantSize = sizeof(m_HDRInfo);
		m_HDRFramebuffer.m_FramebufferAttachmentInfo.p_PushConstantShaderStage = ShaderStage::FragmentBit;
		auto& info = m_HDRFramebuffer.addAttachment(p_Window->getWindowSize(), ImageFormat::C16SF_4C, Layout::ShaderR);
		//info.p_Channels = 4; // Set for read pixels
		//info.p_Bytes = 4; // Set for read pixels
		m_HDRFramebuffer.create();

	}
	void RenderPipeline_Deferred::render(RenderInfo& p_RenderInfo)
	{
		m_LightPassFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, m_RendererPtr->getCurrentCameraPtr()->getPosition(), 0);
		float point_light_count = 0;
		static P_Light p_light;

		// copy SSAO specific data to uniformbuffer declared in SSAO shaders
		m_SSAOFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, SSAO_Intensity, 0);
		m_SSAOFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, SSAO_Radius, sizeof(float));
		m_SSAOFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, SSAO_Bias, 2*sizeof(float));
		float kernel_size = Sample_Count;
		m_SSAOFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, kernel_size, 3*sizeof(float));
		m_SSAOFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, m_RendererPtr->getCurrentCameraPtr()->getPerspectiveMatrix(), sizeof(Vector4f));
		m_SSAOFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, m_RendererPtr->getViewProjection() , sizeof(Vector4f) + sizeof(Matrix4f));
		
		// copy SSR specific data to uniformbuffer declared in SSR shaders
		//m_SSRFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, SSR_Data, 0);
		//m_SSRFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, m_RendererPtr->getCurrentCameraPtr()->getViewDirection(), 4 * sizeof(float));
		//m_SSRFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, m_RendererPtr->getCurrentCameraPtr()->getPerspectiveMatrix(), 4*sizeof(float) + sizeof(Vector4f));
		//m_SSRFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, m_RendererPtr->getViewProjection() , 4 * sizeof(float) + sizeof(Matrix4f) + sizeof(Vector4f));

		for (auto& entity : m_ScenePtr->getEntities()) {
			auto directional_light = m_ScenePtr->getComponent<DirectionalLight3DComponent>(entity);
			auto d_shadow_map = m_ScenePtr->getComponent<DShadowMapComponent>(entity);
			if (directional_light && d_shadow_map) {
				volumetric_light_mie_scattering = directional_light->VolumetricLightDensity;
				d_shadow_map->ShadowMap.m_View = Matrix4f::lookAtRH(directional_light->Direction * 3000, { 0,0,0 }, { 0, 1, 0 });
				m_LightPassFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, d_shadow_map->ShadowMap.m_View * d_shadow_map->ShadowMap.m_Projection, sizeof(Vector4f));
			}
			if(directional_light){
				m_LightPassFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, directional_light->Direction, sizeof(Matrix4f) + sizeof(Vector4f));
				m_LightPassFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, directional_light->Color, sizeof(Matrix4f) + 2 * sizeof(Vector4f));
			}
			auto point_light = m_ScenePtr->getComponent<PointLight3DComponent>(entity);
			if (point_light) {
				p_light.Color = { point_light->Color.r, point_light->Color.g, point_light->Color.b, 0};
				p_light.Position = { point_light->Position.x, point_light->Position.y, point_light->Position.z, 0 };
				p_light.Radius = point_light->Radius;
				p_light.m_ShadowMapID = point_light->ShadowMapID;
				m_LightPassFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, p_light, 3 * sizeof(Vector4f) + sizeof(Matrix4f) + sizeof(P_Light) * point_light_count);
				point_light_count++;
			}
		}

		m_LightPassFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, point_light_count, sizeof(Vector3f));

		m_HDRInfo.deltatime = m_WindowPtr->getDeltaTime();
		m_HDRFramebuffer.getDescriptors().copy(ShaderStage::FragmentBit, m_DOFInfo, 0);

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
				break;
			}
		}

		m_RendererPtr->beginRenderPass(p_RenderInfo, m_GBuffer);

		for (auto entity : m_ScenePtr->getEntities()) {
			auto skybox = m_ScenePtr->getComponent<SkyboxComponent>(entity);
			if (skybox) {
				if (skybox->_AtmosphericScattering)
					m_RendererPtr->submit(*skybox->skybox, skybox->_AtmosphericScatteringInfo);
			}
			auto mesh = m_ScenePtr->getComponent<Mesh3DComponent>(entity);
			auto transform = m_ScenePtr->getComponent<Transform3DComponent>(entity);
			if (!mesh) continue;
			if (!transform) continue;
			if (mesh->mesh != nullptr)
				m_RendererPtr->submit(*mesh->mesh, transform->transform);

		}
		m_RendererPtr->endRenderPass();

		// SSAO
		m_RendererPtr->beginRenderPass(p_RenderInfo, m_SSAOFramebuffer);
		m_RendererPtr->endRenderPass();
		
		m_RendererPtr->beginRenderPass(p_RenderInfo, m_SSAOBlurFramebuffer);
		m_RendererPtr->endRenderPass();
		
		//m_RendererPtr->beginRenderPass(p_RenderInfo, m_SSRFramebuffer);
		//m_RendererPtr->endRenderPass();
		
		// Light pass
		m_RendererPtr->beginRenderPass(p_RenderInfo, m_LightPassFramebuffer);
		m_RendererPtr->endRenderPass();
		
		for (auto& img : m_LightPassFramebuffer.getImageAttachments())
			m_RendererPtr->blitImage(img[0].getImage());

		m_RendererPtr->beginRenderPass(p_RenderInfo, m_HDRFramebuffer);
		m_RendererPtr->endRenderPass();
	}
	//vk::FramebufferAttachment& RenderPipeline_Deferred::getGBloomFramebuffer(Vector2i p_Size)
	//{
	//	// TODO: insert return statement here
	//	return vk::FramebufferAttachment();
	//}
}
