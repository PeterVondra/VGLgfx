#include "VulkanRenderer.h"

namespace exz
{
	namespace vk
	{
		RenderInfo Renderer::defaultRenderInfo;

		//===============================================================================
		//	VULKAN RENDERER
		//===============================================================================

		Renderer::Renderer() 
			: m_DefaultRenderPass(RenderPassType::Graphics),
			//m_ShadowMapRenderPass(RenderPassType::Graphics),
			//m_GaussanBlurRenderPass(RenderPassType::Graphics),
			//colorRenderPass(RenderPassType::Graphics),
			m_CurrentWindowSize({0, 0}), m_MainRenderPass(RenderPassType::Graphics), m_ThreadPool(std::thread::hardware_concurrency())
		{
			m_ContextPtr = &ContextSingleton::getInstance();
			m_CurrentWindow = nullptr;
			m_CurrentCmdBuffer = 0;

			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.signalSemaphoreCount = 1;

			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.swapchainCount = 1;
			presentInfo.pResults = nullptr;

			initShaders();
		}
		Renderer::Renderer(Window* p_Window) 
			: m_DefaultRenderPass(RenderPassType::Graphics),
			//m_ShadowMapRenderPass(RenderPassType::Graphics),
			//m_GaussanBlurRenderPass(RenderPassType::Graphics),
			//colorRenderPass(RenderPassType::Graphics),
			m_CurrentWindowSize({ 0, 0 }), m_MainRenderPass(RenderPassType::Graphics), m_ThreadPool(std::thread::hardware_concurrency())
		{
			m_ContextPtr = &ContextSingleton::getInstance();
			bind(p_Window);

			m_CurrentWindowSize = p_Window->getWindowSize();

			m_CurrentWindow = p_Window;
			m_Camera = nullptr;
			m_CurrentCmdBuffer = 0;

			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.signalSemaphoreCount = 1;

			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.swapchainCount = 1;
			presentInfo.pResults = nullptr;

			initShaders();
			//imgui.init();
		}
		Renderer::Renderer(Window& p_Window) 
			: m_DefaultRenderPass(RenderPassType::Graphics),
			//m_ShadowMapRenderPass(RenderPassType::Graphics),
			//m_GaussanBlurRenderPass(RenderPassType::Graphics),
			//colorRenderPass(RenderPassType::Graphics),
			m_CurrentWindowSize({ 0, 0 }), m_ThreadPool(std::thread::hardware_concurrency())
			, imgui(&p_Window, &m_DefaultInheritanceInfo), m_MainRenderPass(RenderPassType::Graphics)
		{
			m_ContextPtr = &ContextSingleton::getInstance();
			bind(&p_Window);

			m_CurrentWindowSize = p_Window.getWindowSize();

			m_CurrentWindow = &p_Window;
			m_Camera = nullptr;
			m_CurrentCmdBuffer = 0;

			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.signalSemaphoreCount = 1;

			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.swapchainCount = 1;
			presentInfo.pResults = nullptr;

			initShaders();
		}
		Renderer::Renderer(Window& p_Window, Vector2i p_RenderResolution)
			: m_DefaultRenderPass(RenderPassType::Graphics),
			//m_ShadowMapRenderPass(RenderPassType::Graphics),
			//m_GaussanBlurRenderPass(RenderPassType::Graphics),
			//colorRenderPass(RenderPassType::Graphics),
			m_CurrentWindowSize({ 0, 0 }), m_ThreadPool(std::thread::hardware_concurrency())
			, imgui(&p_Window, &m_DefaultInheritanceInfo), m_MainRenderPass(RenderPassType::Graphics)
		{
			m_ContextPtr = &ContextSingleton::getInstance();

			bind(&p_Window);
			setRenderResolution(p_RenderResolution);
			createMainRenderPass();
			createMainResources();

			setAspectRatio(m_RenderResolution.getVec2f());

			m_CurrentWindowSize = p_Window.getWindowSize();

			m_CurrentWindow = &p_Window;
			m_Camera = nullptr;
			m_CurrentCmdBuffer = 0;

			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.signalSemaphoreCount = 1;

			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.swapchainCount = 1;
			presentInfo.pResults = nullptr;

			initShaders();
		}
		Renderer::~Renderer()
		{

		}

		void Renderer::initShaders()
		{
			m_RecRawVertices =
			{
				Vector2f(-1.0f,	-1.0f),	Vector2f(0.0f, 1.0f),
				Vector2f(1.0f,	-1.0f),	Vector2f(1.0f, 1.0f),
				Vector2f(1.0f,	 1.0f),	Vector2f(1.0f, 0.0f),
				Vector2f(-1.0f,	 1.0f),	Vector2f(0.0f, 0.0f)
			};

			m_RecRawIndices = { 0, 1, 2, 2, 3, 0 };

			exz::BufferLayout layout =
			{
				{
					{ exz::ShaderDataType::Vec2f, 0, "in_Position"  },
					{ exz::ShaderDataType::Vec2f, 1, "in_TexCoords" }
				},0
			};

			m_RecVertices.setLayout(layout);
			m_RecVertices.fill(m_RecRawVertices);
			m_RecIndices.fill(m_RecRawIndices);
			m_RecVao.fill(m_RecVertices, m_RecIndices);

			PipelineInfo pipelineInfo;
			//pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
			pipelineInfo.p_MSAASamples = m_CurrentWindow->maxMSAASamples;
			pipelineInfo.p_Viewport = Viewport(m_CurrentWindow->m_WindowSize, { 0, 0 });
			pipelineInfo.p_Scissor = Scissor(m_CurrentWindow->m_WindowSize, { 0, 0 });
			pipelineInfo.p_SampleRateShading = false;

			//=====================================================================================//
			// Create pipeline for 2D circles
			//=====================================================================================//
			m_2DCircleShader.setShader("resources/Shaders/2DCircle/vert.spv", "resources/Shaders/2DCircle/frag.spv");
			pipelineInfo.p_Shader = &m_2DCircleShader;
			pipelineInfo.p_RenderPass = &m_DefaultRenderPass;

			exz::BufferLayout _2DCircleLayout = { {
					{ exz::ShaderDataType::Vec2f, 0, "in_Position" },
					{ exz::ShaderDataType::Vec2f, 1, "in_Uv" }
				},0
			};

			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(1, 1, 0, 0));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_2DCircleLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_2DCircleLayout).second);

			m_2DCirclePipeline.create(pipelineInfo);

			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();

			//=====================================================================================//
			// Create pipeline for 2D shapes
			//=====================================================================================//
			m_2DShapeShader.setShader("resources/Shaders/2DShape/vert.spv", "resources/Shaders/2DShape/frag.spv");
			pipelineInfo.p_Shader = &m_2DShapeShader;
			pipelineInfo.p_AlphaBlending = true;

			exz::BufferLayout _2DShapeLayout = { {
					{ exz::ShaderDataType::Vec2f, 0, "in_Position" },
					{ exz::ShaderDataType::Vec2f, 1, "in_Uv" }
				},0
			};

			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(1, 0, 0, 0));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_2DShapeLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_2DShapeLayout).second);

			m_2DShapePipeline.create(pipelineInfo);

			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();
			
			//=====================================================================================//
			// Create pipeline for Textured quad
			//=====================================================================================//
			m_TexturedQuadShader.setShader("resources/Shaders/HDR/vert.spv", "resources/Shaders/HDR/frag.spv");
			pipelineInfo.p_Shader = &m_TexturedQuadShader;
			pipelineInfo.p_AlphaBlending = true;
			pipelineInfo.p_SampleRateShading = true;
			pipelineInfo.p_UsePushConstants = true;
			pipelineInfo.p_PushConstantOffset = 0;
			pipelineInfo.p_PushConstantShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
			pipelineInfo.p_PushConstantSize = sizeof(HDRFXAAInfo);

			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(0, 0, 0, 2));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_2DShapeLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_2DShapeLayout).second);

			m_TexturedQuadPipeline.create(pipelineInfo);

			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();

			pipelineInfo.p_UsePushConstants = false;

			//=====================================================================================//
			// Create pipeline for 2D text
			//=====================================================================================//
			m_2DTextBoxShader.setShader("resources/Shaders/TextBox/vert.spv", "resources/Shaders/TextBox/frag.spv");
			pipelineInfo.p_Shader = &m_2DTextBoxShader;
			pipelineInfo.p_AlphaBlending = true;

			exz::BufferLayout _2DTextBoxLayout = { {
					{ exz::ShaderDataType::Vec2f, 0, "in_Position" },
					{ exz::ShaderDataType::Vec2f, 1, "in_Uv" }
				},0
			};
			exz::BufferLayout _2DTextBoxInstancesLayout = { {
				{ exz::ShaderDataType::Int, 2, "in_InstanceGlyphIndex" },
				{ exz::ShaderDataType::Mat4f, 3, "in_InstancePosition" },
			},1};

			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(1, 0, 0, 0, 127));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_2DTextBoxLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_2DTextBoxLayout).second);
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_2DTextBoxInstancesLayout, InputRate::Instance).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_2DTextBoxInstancesLayout, InputRate::Instance).second);

			m_2DTextBoxPipeline.create(pipelineInfo);

			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();

			//=====================================================================================//
			// Create pipeline for shadow depth map
			//=====================================================================================//
			m_ShadowDepthMapShader.setShader("resources/Shaders/SM3D/vert.spv", "resources/Shaders/SM3D/frag.spv");
			pipelineInfo.p_Shader = &m_ShadowDepthMapShader;
			pipelineInfo.p_AlphaBlending = false;
			pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_DepthBias = true;
			pipelineInfo.p_DepthBiasConstantFactor = depthBiasConstant;
			pipelineInfo.p_DepthBiasSlopeFactor = depthBiasSlope;
			pipelineInfo.p_DepthBiasClamp = 0.0f;

			exz::BufferLayout m_ShadowDepthMapLayout = { {
					{ exz::ShaderDataType::Vec3f, 0, "in_Position"  },
					{ exz::ShaderDataType::Vec2f, 1, "in_Uv"		},
					{ exz::ShaderDataType::Vec3f, 2, "in_Normal"    },
					{ exz::ShaderDataType::Vec3f, 3, "in_Tangent"   },
					{ exz::ShaderDataType::Vec3f, 4, "in_Bitangent" }
				},0
			}; 
						
			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(1, 0, 0, 0));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&m_ShadowDepthMapLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&m_ShadowDepthMapLayout).second);
			pipelineInfo.p_MSAASamples = 1;

			m_ShadowMapSize = { 4096, 4096 };
			projection = Matrix4f::orthoRH_ZO(-3000, 3000, -3000, 3000, 1, 4800);
			m_SMMVP = view * projection;
						
			m_ShadowMapRenderFramebufferInfo.p_Size = m_ShadowMapSize;
			m_ShadowMapRenderFramebufferInfo.p_ColorType = ColorType::D16UN;
			m_ShadowMapRenderFramebufferInfo.p_BorderColor = BorderColor::TransparentBlack;
			m_ShadowMapRenderFramebufferInfo.p_SamplerMode = SamplerMode::ClampToEdge;
			m_ShadowMapRenderFramebufferInfo.p_RawIndices = m_RecRawIndices;
			m_ShadowMapRenderFramebufferInfo.p_RectangleData = &m_RecVao;
			m_ShadowMapRenderFramebufferInfo.p_GraphicsPipelineInfo = pipelineInfo;
			
			m_ShadowMapRenderFramebuffer.create(m_ShadowMapRenderFramebufferInfo);
						
			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();
			
			//=====================================================================================//
			// Create pipeline for Gaussan Blur
			//=====================================================================================//
			m_GaussanBlurShader.setShader("resources/Shaders/Gauss-Blur/vert.spv", "resources/Shaders/Gauss-Blur/frag.spv");
			pipelineInfo.p_Shader = &m_GaussanBlurShader;
			pipelineInfo.p_AlphaBlending = false;
			pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_DepthBias = false;
			pipelineInfo.p_UsePushConstants = true;
			pipelineInfo.p_PushConstantShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
			pipelineInfo.p_PushConstantOffset = 0;
			pipelineInfo.p_PushConstantSize = sizeof(Vector2f);
						
			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(0, 0, 0, 1));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_2DShapeLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_2DShapeLayout).second);
			pipelineInfo.p_MSAASamples = 1;
						
			colorRenderFramebufferInfo.p_UseGraphicsPipeline = false;
			colorRenderFramebufferInfo.p_Size = m_RenderResolution;
			colorRenderFramebufferInfo.p_SamplerMode = SamplerMode::ClampToBorder;
			colorRenderFramebufferInfo.p_RawIndices = m_RecRawIndices;
			colorRenderFramebufferInfo.p_RectangleData = &m_RecVao;
			colorRenderFramebufferInfo.p_ColorType = ColorType::C32SF;
			colorRenderFramebufferInfo.p_BorderColor = BorderColor::OpaqueBlack;
			colorRenderFramebuffer.create(colorRenderFramebufferInfo);

			m_GaussBlurFramebufferInfos[0].p_GraphicsPipelineInfo = pipelineInfo;
			m_GaussBlurFramebufferInfos[1].p_GraphicsPipelineInfo = pipelineInfo;

			createGaussBlurFramebuffer();

			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();

			blurScale[0] = { BlurScale / m_GaussBlurFramebuffers[0].m_Info.p_Size.x, 0 };
			blurScale[1] = { 0, BlurScale / m_GaussBlurFramebuffers[1].m_Info.p_Size.y};

			m_TexturedQaudUniformManager.setUniform("imageHDR", colorRenderFramebuffer.m_ImageAttachment.getImage(), 0);
			m_TexturedQaudUniformManager.setUniform("imageHDR", m_GaussBlurFramebuffers[1].m_ImageAttachment.getImage(), 1);
			m_TexturedQaudUniformManager.create();

			m_CommandBuffers.emplace_back(Level::Secondary);

			//=====================================================================================//
			// Create pipeline for skybox
			//=====================================================================================//
			m_SkyBoxShader.setShader("resources/Shaders/SkyBox/vert.spv", "resources/Shaders/SkyBox/frag.spv");

			pipelineInfo.p_Shader = &m_SkyBoxShader;
			pipelineInfo.p_CullMode = CullMode::FrontBit;
			pipelineInfo.p_DepthBuffering = false;
			pipelineInfo.p_RenderPass = &colorRenderFramebuffer.m_RenderPass;

			BufferLayout _SkyBoxLayout = { {
					{ exz::ShaderDataType::Vec3f, 0, "in_Position"  },
					{ exz::ShaderDataType::Vec2f, 1, "in_Uv"		},
					{ exz::ShaderDataType::Vec3f, 2, "in_Normal"    },
					{ exz::ShaderDataType::Vec3f, 3, "in_Tangent"   },
					{ exz::ShaderDataType::Vec3f, 4, "in_Bitangent" }
				},0
			};

			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(1, 0, 0, 1));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_SkyBoxLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_SkyBoxLayout).second);

			m_SkyBoxPipeline.create(pipelineInfo);

			pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_DepthBuffering = true;

			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();
			
			//=====================================================================================//
			// Create pipeline for Atmospheric Scattering
			//=====================================================================================//
			m_AtmosphericScatteringShader.setShader("resources/Shaders/AtmosphericScattering/vert.spv", "resources/Shaders/AtmosphericScattering/frag.spv");

			pipelineInfo.p_Shader = &m_AtmosphericScatteringShader;
			pipelineInfo.p_CullMode = CullMode::FrontBit;
			pipelineInfo.p_DepthBuffering = false;
			pipelineInfo.p_RenderPass = &colorRenderFramebuffer.m_RenderPass;

			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(1, 0, 0, 0));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_SkyBoxLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_SkyBoxLayout).second);

			m_AtmosphericScatteringPipeline.create(pipelineInfo);

			pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_DepthBuffering = true;

			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();

			m_AtmosphericScatteringUM = vk::UniformManager();

			// Plus 16 for alignment
			m_AtmosphericScatteringUM.setUniformBlock(vk::ShaderStage::VertexBit, "ubo", nullptr, 2 * sizeof(Matrix4f) + sizeof(AtmosphericScatteringInfo));
			m_AtmosphericScatteringUM.create();

			atmosphericScatteringInfo = {};
		}

		void Renderer::bind(Window* p_Window)
		{
			if (m_CurrentWindow != p_Window && p_Window != nullptr)
			{
				createSyncObjects();

				vkDeviceWaitIdle(m_ContextPtr->device);
				
				m_CurrentWindow = p_Window;

				createRenderPass();
				imgui.init(m_DefaultRenderPass);

				createDepthResources();

				m_CurrentWindow->m_SwapChain.initFramebuffers(m_DefaultRenderPass.m_RenderPass, m_DepthImageAttachment.m_ImageView, m_CurrentWindow->colorImageView);

				for (size_t i = 0; i < m_CurrentWindow->m_SwapChain.swapChainImages.size(); i++)
					m_PrimaryCommandBuffers.emplace_back(Level::Primary);

				presentInfo.pSwapchains = &m_CurrentWindow->m_SwapChain.swapChain;

				p_ProjectionSize = m_CurrentWindow->getWindowSize().getVec2f();

				m_RenderResolution = m_CurrentWindow->getWindowSize();
				setAspectRatio(m_RenderResolution.getVec2f());
			}
		}

		void Renderer::destroy()
		{
			
		}

		void Renderer::refreshRenderResolution(const Vector2i& p_Size)
		{
			//m_ContextPtr->deviceWaitIdle();

			//m_RenderResolution = p_Size;

			blurScale[0] = { BlurScale / p_Size.x, 0 };
			blurScale[1] = { 0, BlurScale / p_Size.y };

			//m_ShadowMapRenderFramebufferInfo.p_Size = m_RenderResolution;
			//m_ShadowMapRenderFramebuffer.destroy();
			//m_ShadowMapRenderFramebuffer.create(m_ShadowMapRenderFramebufferInfo);
			//
			//colorRenderFramebufferInfo.p_Size = m_RenderResolution;
			//colorRenderFramebuffer.destroy();
			//colorRenderFramebuffer.create(colorRenderFramebufferInfo);
			//
			//m_GaussBlurFramebufferInfos[0].p_Size = m_RenderResolution;
			//m_GaussBlurFramebuffers[0].destroy();
			//m_GaussBlurFramebuffers[0].create(m_GaussBlurFramebufferInfos[0]);
			//
			//m_GaussBlurFramebufferInfos[1].p_Size = m_RenderResolution;
			//m_GaussBlurFramebuffers[1].destroy();
			//m_GaussBlurFramebuffers[1].create(m_GaussBlurFramebufferInfos[1]);
		}

		void Renderer::setAspectRatio(Vector2f p_Size)
		{
			p_ProjectionSize = p_Size;
		}

		void Renderer::beginScene()
		{
			txtProj = Matrix4f::orthoRH_ZO(0, m_CurrentWindow->getWindowSize().x, 0, m_CurrentWindow->getWindowSize().y, 0.0f, 1.0f);

			static Matrix4f rot;

			if (Input::keyIsDown(Key::Left))
				rot.rotate(0.05, 0, 1, 0);
			else if (Input::keyIsDown(Key::Right))
				rot.rotate(-0.05, 0, 1, 0);
			if (Input::keyIsDown(Key::Up))
				rot.rotate(0.05, 0, 0, 1);
			else if (Input::keyIsDown(Key::Down))
				rot.rotate(-0.05, 0, 0, 1);

			m_LightDir = rot * Vector3f(0.33, 1, 0.33);

			atmosphericScatteringInfo.p_SunPos = m_LightDir;

			view = Matrix4f::lookAtRH(rot * Vector3f(1000, 3000, 1000), { 0, 0, 0 }, { 0, 1, 0 });
			m_SMMVP = view * projection;

			waitForFences();
			renderTexturedQuadHdr();
		}
		void Renderer::beginScene(CubeMap& p_IrradianceMap, CubeMap& p_PreFilteredMap, Image& p_BRDFLut)
		{
			txtProj = Matrix4f::orthoRH_ZO(0, m_CurrentWindow->getWindowSize().x, 0, m_CurrentWindow->getWindowSize().y, 0.0f, 1.0f);

			irradiance = &p_IrradianceMap;
			preFiltered = &p_PreFilteredMap;
			brdf = &p_BRDFLut;

			static Matrix4f rot;

			if (Input::keyIsDown(Key::Left))
				rot.rotate(0.05, 0, 1, 0);
			else if (Input::keyIsDown(Key::Right))
				rot.rotate(-0.05, 0, 1, 0);
			if (Input::keyIsDown(Key::Up))
				rot.rotate(0.05, 0, 0, 1);
			else if (Input::keyIsDown(Key::Down))
				rot.rotate(-0.05, 0, 0, 1);

			m_OldLightDir = { INFINITY, INFINITY, INFINITY };
			m_LightDir = rot * Vector3f(0.33, 1, 0.33);

			atmosphericScatteringInfo.p_SunPos = m_LightDir;

			Vector3f pos(0);
			if(m_Camera)
				pos = m_Camera->getPosition();
			
			view = Matrix4f::lookAtRH(rot * Vector3f(1000, 3000, 1000), { 0,0,0 }, { 0, 1, 0 });
			m_SMMVP = view * projection;

			waitForFences();
			renderTexturedQuadHdr();

			m_EnvData.p_ShadowMap = &m_ShadowMapRenderFramebuffer.m_ImageAttachment.getImage();
			m_EnvData.p_IrradianceMap = irradiance;
			m_EnvData.p_PreFilteredMap = preFiltered;
			m_EnvData.p_BRDFLut = brdf;
		}

		void Renderer::setCamera(Camera& p_Camera)
		{
			m_Camera = &p_Camera;
			m_Camera->setPerspectiveMatrix(p_ProjectionSize.x / p_ProjectionSize.y);
			m_Camera->setOrthographicMatrix(0, p_ProjectionSize.x, 0, p_ProjectionSize.y, 0.0f, 1.0f);
			txtProj = Matrix4f::orthoRH_ZO(0, m_CurrentWindow->getWindowSize().x, 0, m_CurrentWindow->getWindowSize().y, 0.0f, 1.0f);
		}
		void Renderer::setCamera(Camera* p_Camera)
		{
			m_Camera = p_Camera;
			m_Camera->setPerspectiveMatrix(p_ProjectionSize.x / p_ProjectionSize.y);
			m_Camera->setOrthographicMatrix(0, p_ProjectionSize.x, 0, p_ProjectionSize.y, 0.0f, 1.0f);
			txtProj = Matrix4f::orthoRH_ZO(0, m_CurrentWindow->getWindowSize().x, 0, m_CurrentWindow->getWindowSize().y, 0.0f, 1.0f);
		}

		void Renderer::submit(MeshData& p_MeshData, Matrix4f& p_ModelMatrix, RenderInfo& p_RenderInfo)
		{
			if (p_MeshData.newRT || p_MeshData.recreate)
			{
				initPipelines(p_MeshData, p_RenderInfo);
				m_OldLightDir = INFINITY;
			}

			mat = p_ModelMatrix * m_SMMVP;

			m_SUBM[m_CurrentShadowMapCmdBuffer].setUniform(ShaderStage::VertexBit, "ubo.mvp", mat, 0);

			//m_ThreadPool.enqueue([p_Model, this]{ submitModelThreaded(p_Model); });

			recordMeshCmdBuffer(p_MeshData, p_ModelMatrix);

			for (int i = 0; i < p_MeshData.m_Materials.size(); i++)
			{
				if (!p_MeshData.m_Materials[i].config.render)
					continue;
				if (m_LightDir != m_OldLightDir) {
					p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::FragmentBit, "ubo.ligthDir", m_LightDir, 0);
					p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::VertexBit, "ubo.lightSpaceMat", mat, MeshUBOAlignment::viewPosition_offset);
				}

				p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::FragmentBit, "ubo.albedo", p_MeshData.m_Materials[i].config.m_Albedo, 4 * sizeof(float));
				p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::FragmentBit, "ubo.mrao",
					Vector3f
					(
						p_MeshData.m_Materials[i].config.m_Metallic,
						p_MeshData.m_Materials[i].config.m_Roughness,
						p_MeshData.m_Materials[i].config.m_Ambient
					), 8 * sizeof(float)
				);
				p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::FragmentBit, "ubo.pdmDepth", p_MeshData.m_Materials[i].config.m_PDMDepth, 12 * sizeof(float));
				p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::FragmentBit, "ubo.lumaThreshold", lumaThreshold, 13 * sizeof(float));


				// MVP Matrix
				p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::VertexBit, "ubo.mvp", mvp, MeshUBOAlignment::mvp_offset);
				// Model Matrix
				p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::VertexBit, "ubo.model", p_ModelMatrix, MeshUBOAlignment::model_offset);

				// Camera position
				p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::VertexBit, "ubo.cameraPos", m_Camera->getPosition(), MeshUBOAlignment::viewPosition_offset + sizeof(Matrix4f));

				p_MeshData.m_Materials[i].m_PrevConfig = p_MeshData.m_Materials[i].config;
			}
		}
		void Renderer::submit(MeshData& p_MeshData, const uint32_t p_MaterialID, Matrix4f& p_ModelMatrix, RenderInfo& p_RenderInfo)
		{
			if (!p_MeshData.m_Materials[p_MaterialID].config.render)
				return;

			if (p_MeshData.newRT || p_MeshData.recreate)
			{
				initPipelines(p_MeshData, p_RenderInfo);
				m_OldLightDir = INFINITY;
			}

			mat = p_ModelMatrix * m_SMMVP;

			m_SUBM[m_CurrentShadowMapCmdBuffer].setUniform(ShaderStage::VertexBit, "ubo.mvp", mat, 0);

			//m_ThreadPool.enqueue([p_Model, this]{ submitModelThreaded(p_Model); });

			recordMeshCmdBuffer(p_MeshData, p_MaterialID, p_ModelMatrix);

			int i = 0;

			if (m_LightDir != m_OldLightDir) {
				p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::FragmentBit, "ubo.ligthDir", m_LightDir, 0);
				p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::VertexBit, "ubo.lightSpaceMat", mat, MeshUBOAlignment::viewPosition_offset);
			}

			p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::FragmentBit, "ubo.albedo", p_MeshData.m_Materials[i].config.m_Albedo, 4 * sizeof(float));
			p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::FragmentBit, "ubo.mrao",
				Vector3f
				(
					p_MeshData.m_Materials[i].config.m_Metallic,
					p_MeshData.m_Materials[i].config.m_Roughness,
					p_MeshData.m_Materials[i].config.m_Ambient
				), 8 * sizeof(float)
			);
			p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::FragmentBit, "ubo.pdmDepth", p_MeshData.m_Materials[i].config.m_PDMDepth, 12 * sizeof(float));
			p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::FragmentBit, "ubo.lumaThreshold", lumaThreshold, 13 * sizeof(float));


			// MVP Matrix
			p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::VertexBit, "ubo.mvp", mvp, MeshUBOAlignment::mvp_offset);
			// Model Matrix
			p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::VertexBit, "ubo.model", p_ModelMatrix, MeshUBOAlignment::model_offset);

			// Camera position
			p_MeshData.m_MtlUniformManagers[i].setUniform(ShaderStage::VertexBit, "ubo.cameraPos", m_Camera->getPosition(), MeshUBOAlignment::viewPosition_offset + sizeof(Matrix4f));

			p_MeshData.m_Materials[i].m_PrevConfig = p_MeshData.m_Materials[i].config;
		}

		void Renderer::recordMeshCmdBuffer(MeshData& p_MeshData, Matrix4f& p_ModelMatrix)
		{
			mvp = p_ModelMatrix * m_Camera->getViewMatrix() * m_Camera->getPerspectiveMatrix();

			// If swap chain needs to be recreated then recreate relevant objects
			unsigned int index = m_CurrentColorCmdBuffer;
			unsigned int cmdIndex = m_CurrentShadowMapCmdBuffer;

			// Shadow map rendering
			if (m_LightDir != m_OldLightDir)
			{
				m_ShadowMapCommandBuffers[cmdIndex].cmdBegin(m_ShadowMapRenderFramebuffer.m_InheritanceInfo);
				m_ShadowMapCommandBuffers[cmdIndex].cmdSetViewport(Viewport({ m_ShadowMapSize.x, -m_ShadowMapSize.y }, { 0, m_ShadowMapSize.y }));
				m_ShadowMapCommandBuffers[cmdIndex].cmdSetScissor(Scissor(Vector2i(m_ShadowMapSize.x, m_ShadowMapSize.y), { 0, 0 }));

				m_ShadowMapCommandBuffers[cmdIndex].cmdBindPipeline(m_ShadowMapRenderFramebuffer.m_GraphicsPipeline);
				m_ShadowMapCommandBuffers[cmdIndex].cmdBindUniformBuffer(m_SUBM[m_CurrentShadowMapCmdBuffer]);

				vkCmdSetDepthBias
				(
					m_ShadowMapCommandBuffers[cmdIndex].m_CommandBuffer,
					depthBiasConstant,
					0.00f,
					depthBiasSlope
				);

				m_ShadowMapCommandBuffers[cmdIndex].cmdBindVertexArray(p_MeshData.m_VertexArray);
				uint32_t prevIndex = 0;
				for (int i = 0; i < p_MeshData.m_Materials.size(); i++)
				{
					if (!p_MeshData.m_Materials[i].config.render) {
						m_ShadowMapCommandBuffers[cmdIndex].cmdDrawIndexed(prevIndex, p_MeshData.m_MaterialIndices[i].first);
						prevIndex = p_MeshData.m_MaterialIndices[i].second;
					}
				}

				m_ShadowMapCommandBuffers[cmdIndex].cmdDrawIndexed(prevIndex, p_MeshData.indices.size());

				m_ShadowMapCommandBuffers[cmdIndex].cmdEnd();
			}

			// Render normally
			m_ColorCommandBuffers[index].cmdBegin(colorRenderFramebuffer.m_InheritanceInfo);
			m_ColorCommandBuffers[index].cmdSetViewport(Viewport({ m_RenderResolution.x, -m_RenderResolution.y }, Vector2i(0, m_RenderResolution.y) + m_ViewportPos));
			m_ColorCommandBuffers[index].cmdSetScissor(Scissor({ m_RenderResolution.x, m_RenderResolution.y }, { 0, 0 }));

			for (int i = 0; i < p_MeshData.m_Materials.size(); i++)
			{
				if (!p_MeshData.m_Materials[i].config.render)
					continue;
				m_ColorCommandBuffers[index].cmdBindPipeline(pipelines[p_MeshData.m_Materials[i].m_PipelineIndex].first);
				m_ColorCommandBuffers[index].cmdBindUniformBuffer(p_MeshData.m_MtlUniformManagers[i]);

				m_ColorCommandBuffers[index].cmdBindVertexArray(p_MeshData.m_VertexArray);
				m_ColorCommandBuffers[index].cmdDrawIndexed(p_MeshData.m_MaterialIndices[i].first, p_MeshData.m_MaterialIndices[i].second);
			}

			m_ColorCommandBuffers[index].cmdEnd();
			m_CurrentShadowMapCmdBuffer++;
			m_CurrentColorCmdBuffer++;
		}
		void Renderer::recordMeshCmdBuffer(MeshData& p_MeshData, const uint32_t p_MaterialID, Matrix4f& p_ModelMatrix)
		{
			//mvp = p_ModelMatrix * m_Camera->getViewMatrix() * m_Camera->getPerspectiveMatrix();
			//
			//// If swap chain needs to be recreated then recreate relevant objects
			//unsigned int index = m_CurrentColorCmdBuffer;
			//unsigned int cmdIndex = m_CurrentShadowMapCmdBuffer;
			//
			//// Shadow map rendering
			//if (m_LightDir != m_OldLightDir)
			//{
			//	m_ShadowMapCommandBuffers[cmdIndex].cmdBegin(m_ShadowMapRenderFramebuffer.m_InheritanceInfo);
			//	m_ShadowMapCommandBuffers[cmdIndex].cmdSetViewport(Viewport({ m_ShadowMapSize.x, -m_ShadowMapSize.y }, { 0, m_ShadowMapSize.y }));
			//	m_ShadowMapCommandBuffers[cmdIndex].cmdSetScissor(Scissor(Vector2i(m_ShadowMapSize.x, m_ShadowMapSize.y), { 0, 0 }));
			//
			//	m_ShadowMapCommandBuffers[cmdIndex].cmdBindPipeline(m_ShadowMapRenderFramebuffer.m_GraphicsPipeline);
			//	m_ShadowMapCommandBuffers[cmdIndex].cmdBindUniformBuffer(m_SUBM[m_CurrentShadowMapCmdBuffer]);
			//
			//	vkCmdSetDepthBias
			//	(
			//		m_ShadowMapCommandBuffers[cmdIndex].m_CommandBuffer,
			//		depthBiasConstant,
			//		0.00f,
			//		depthBiasSlope
			//	);
			//
			//	m_ShadowMapCommandBuffers[cmdIndex].cmdBindVertexArray(p_MeshData.m_VertexArray);
			//
			//	m_ShadowMapCommandBuffers[cmdIndex].cmdDrawIndexed(p_MeshData.m_MaterialIndices[p_MaterialID].first, p_MeshData.m_MaterialIndices[p_MaterialID].second);
			//
			//	m_ShadowMapCommandBuffers[cmdIndex].cmdEnd();
			//}
			//
			//// Render normally
			//m_ColorCommandBuffers[index].cmdBegin(colorRenderFramebuffer.m_InheritanceInfo);
			//m_ColorCommandBuffers[index].cmdSetViewport(Viewport({ m_RenderResolution.x, -m_RenderResolution.y }, Vector2i(0, m_RenderResolution.y) + m_ViewportPos));
			//m_ColorCommandBuffers[index].cmdSetScissor(Scissor({ m_RenderResolution.x, m_RenderResolution.y }, { 0, 0 }));
			//
			//if (!p_MeshData.m_Materials[p_MaterialID].config.render)
			//	continue;
			//m_ColorCommandBuffers[index].cmdBindPipeline(pipelines[p_MeshData.m_Materials[i].m_PipelineIndex].first);
			//m_ColorCommandBuffers[index].cmdBindUniformBuffer(p_MeshData.m_MtlUniformManagers[i]);
			//
			//m_ColorCommandBuffers[index].cmdBindVertexArray(p_MeshData.m_VertexArray);
			//m_ColorCommandBuffers[index].cmdDrawIndexed(p_MeshData.m_MaterialIndices[i].first, p_MeshData.m_MaterialIndices[i].second);
			//
			//m_ColorCommandBuffers[index].cmdEnd();
			//m_CurrentShadowMapCmdBuffer++;
			//m_CurrentColorCmdBuffer++;
		}

		void Renderer::initPipelines(MeshData& p_MeshData, RenderInfo& p_RenderInfo)
		{
			bool useBump = false;
			int indexTUM = 0;

			if (p_MeshData.recreate)
			{
				for (auto& mtl : p_MeshData.m_MtlUniformManagers)
					mtl.destroyDscLayout();
			}

			if (!p_MeshData.m_MTLCompleted || p_MeshData.recreate || p_MeshData.newRT)
				p_MeshData.initMaterials(m_EnvData);

			if (p_MeshData.newRT)
			{
				m_SUBM.resize(m_ShadowMapCommandBuffers.size() + 1);
				m_SUBM[m_ShadowMapCommandBuffers.size()].setUniformBlock(ShaderStage::VertexBit, "ubo", &m_SMMVP, sizeof(Matrix4f));
				m_SUBM[m_ShadowMapCommandBuffers.size()].createDescriptorSetLayout();
				m_SUBM[m_ShadowMapCommandBuffers.size()].initDescriptorPool();
				m_SUBM[m_ShadowMapCommandBuffers.size()].createDescriptorSet();

				m_ShadowMapCommandBuffers.emplace_back(Level::Secondary);
				m_ColorCommandBuffers.emplace_back(Level::Secondary);
			}

			PipelineInfo pipelineInfo;
			pipelineInfo.p_CullMode = p_RenderInfo.p_CullMode;
			pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
			pipelineInfo.p_PolygonMode = p_RenderInfo.p_PolygonMode;
			pipelineInfo.p_MSAASamples = 1;
			pipelineInfo.p_Viewport = Viewport(m_CurrentWindow->m_WindowSize, { 0, 0 });
			pipelineInfo.p_Scissor = Scissor(m_CurrentWindow->m_WindowSize, { 0, 0 });
			pipelineInfo.p_SampleRateShading = true;
			pipelineInfo.p_RenderPass = &colorRenderFramebuffer.m_RenderPass;
			pipelineInfo.p_AlphaBlending = p_RenderInfo.p_AlphaBlending;

			pipelineInfo.p_AttributeDescription = p_MeshData.m_VertexArray.getAttributeDescriptions();
			pipelineInfo.p_BindingDescription = p_MeshData.m_VertexArray.getBindingDescription();

			for (int i = 0; i < p_MeshData.m_Materials.size(); i++)
			{
				bool c = false;

				for (int k = 0; k < pipelines.size(); k++)
				{
					if (p_MeshData.m_Materials[i].m_ShaderInfo == pipelines[k].second)
					{
						c = true;
						p_MeshData.m_Materials[i].m_PipelineIndex = k;
					}
				}

				if (c)
					continue;

				Shader shader;
				shader.compile(ShaderPermutationsGenerator::generateShader(p_MeshData.m_Materials[i].m_ShaderInfo));
				shaders.push_back(shader);

				Pipeline pipeline;

				pipelineInfo.p_Shader = &shaders[shaders.size() - 1];

				pipelineInfo.p_DescriptorSetLayout = p_MeshData.m_MtlUniformManagers[i].m_DescriptorSetLayout;

				pipeline.create(pipelineInfo);

				pipelines.emplace_back(pipeline, p_MeshData.m_Materials[i].m_ShaderInfo);
				p_MeshData.m_Materials[i].m_PipelineIndex = pipelines.size() - 1;
			}

			p_MeshData.recreate = false;
			p_MeshData.newRT = false;
		}

		void Renderer::submit(Skybox& p_SkyBox, bool renderAtmosphere)
		{
			submit(&p_SkyBox, renderAtmosphere);
		}
		void Renderer::submit(Skybox* p_SkyBox, bool renderAtmosphere)
		{
			if (p_SkyBox->newRT) {
				m_ColorCommandBuffers.emplace_back(Level::Secondary);
				p_SkyBox->newRT = false;
			}
			
			const int index = m_CurrentColorCmdBuffer;
			
			m_ColorCommandBuffers[index].cmdBegin(colorRenderFramebuffer.m_InheritanceInfo);
			m_ColorCommandBuffers[index].cmdSetViewport(Viewport({ m_RenderResolution.x, -m_RenderResolution.y }, { 0, m_RenderResolution.y }));
			m_ColorCommandBuffers[index].cmdSetScissor(Scissor({ m_RenderResolution.x, m_RenderResolution.y }, { 0, 0 }));
			
			if (renderAtmosphere) {
				m_ColorCommandBuffers[index].cmdBindPipeline(m_AtmosphericScatteringPipeline);
				m_ColorCommandBuffers[index].cmdBindUniformBuffer(m_AtmosphericScatteringUM);
			}
			else{
				m_ColorCommandBuffers[index].cmdBindPipeline(m_SkyBoxPipeline);
				m_ColorCommandBuffers[index].cmdBindUniformBuffer(p_SkyBox->m_MtlUniformManagers[0]);
			}
			
			m_ColorCommandBuffers[index].cmdBindVertexArray(p_SkyBox->m_VertexArray);
			m_ColorCommandBuffers[index].cmdDrawIndexed(p_SkyBox->m_MaterialIndices[0].first, p_SkyBox->m_MaterialIndices[0].second);
			
			m_ColorCommandBuffers[index].cmdEnd();
			
			Vector3f viewPos = m_Camera->getPosition();
			m_Camera->setPosition({ 0, 0, 0 });
			mvp = p_SkyBox->model * m_Camera->getViewMatrix() * m_Camera->getPerspectiveMatrix();
			m_Camera->setPosition(viewPos);
			
			if (renderAtmosphere) {
				//atmosphericScatteringInfo.p_RayOrigin = m_Camera->getPosition();

				// MVP Matrix
				m_AtmosphericScatteringUM.setUniform(ShaderStage::VertexBit, "ubo.mvp", mvp, 0);
				// Model Matrix
				m_AtmosphericScatteringUM.setUniform(ShaderStage::VertexBit, "ubo.model", p_SkyBox->model, sizeof(Matrix4f));
				m_AtmosphericScatteringUM.setUniform(ShaderStage::VertexBit, "ubo...", &atmosphericScatteringInfo, sizeof(atmosphericScatteringInfo), 2*sizeof(Matrix4f));

				m_CurrentColorCmdBuffer++;
				return;
			}

			// MVP Matrix
			p_SkyBox->m_MtlUniformManagers[0].setUniform(ShaderStage::VertexBit, "ubo.mvp", mvp, 0);
			// Model Matrix
			p_SkyBox->m_MtlUniformManagers[0].setUniform(ShaderStage::VertexBit, "ubo.model", p_SkyBox->model, sizeof(Matrix4f));
			
			m_CurrentColorCmdBuffer++;
		}

		void Renderer::submit(TextBox& p_TextBox)
		{
			submit(&p_TextBox);
		}
		void Renderer::submit(TextBox* p_TextBox)
		{			
			if (p_TextBox->NewRT)
			{
				p_TextBox->rec.m_UniformManager.setUniformBlock(ShaderStage::VertexBit, "ubo", nullptr, sizeof(Matrix4f) + sizeof(Vector4f));
				p_TextBox->rec.m_UniformManager.setUniform("glyphs", p_TextBox->getFont().glyphs, 1);
				p_TextBox->rec.m_UniformManager.create();

				p_TextBox->m_Vao.fill({ &m_RecVertices, &p_TextBox->m_InstanceTransforms }, m_RecIndices);
			
				m_TxtCommandBuffers.emplace_back(Level::Secondary);
			
				p_TextBox->NewRT = false;
			}

			p_TextBox->m_Vao.update(1);

			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBegin(m_DefaultInheritanceInfo);
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdSetViewport(Viewport({ m_RenderResolution.x, -m_RenderResolution.y }, { 0, m_RenderResolution.y }));
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdSetScissor(Scissor({ m_RenderResolution.x, m_RenderResolution.y }, { 0, 0 }));
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBindPipeline(m_2DTextBoxPipeline);
			
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBindUniformBuffer(p_TextBox->rec.m_UniformManager);
			
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBindVertexArray(p_TextBox->m_Vao);
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdDrawIndexedInstanced(0, m_RecRawIndices.size(), p_TextBox->characterTransforms.size());
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdEnd();
			
			p_TextBox->rec.m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", txtProj, 0);
			p_TextBox->rec.m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", p_TextBox->rec.m_Color, sizeof(Matrix4f));

			m_TxtCommandBufferCount++;
		}
		void Renderer::submit(TextBox* p_TextBox, Scissor p_Scissor)
		{			
			if (p_TextBox->NewRT)
			{
				p_TextBox->rec.m_UniformManager.setUniformBlock(ShaderStage::VertexBit, "ubo", nullptr, sizeof(Matrix4f) + sizeof(Vector4f));
				p_TextBox->rec.m_UniformManager.setUniform("glyphs", p_TextBox->getFont().glyphs, 1);
				p_TextBox->rec.m_UniformManager.create();

				p_TextBox->m_Vao.fill({ &m_RecVertices, &p_TextBox->m_InstanceTransforms }, m_RecIndices);

				m_TxtCommandBuffers.emplace_back(Level::Secondary);

				p_TextBox->NewRT = false;
			}

			p_TextBox->m_Vao.update(1);

			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBegin(m_DefaultInheritanceInfo);
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdSetViewport(Viewport({ m_RenderResolution.x, -m_RenderResolution.y }, { 0, m_RenderResolution.y }));
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdSetScissor(p_Scissor);
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBindPipeline(m_2DTextBoxPipeline);

			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBindUniformBuffer(p_TextBox->rec.m_UniformManager);

			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBindVertexArray(p_TextBox->m_Vao);
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdDrawIndexedInstanced(0, m_RecRawIndices.size(), p_TextBox->characterTransforms.size());
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdEnd();

			p_TextBox->rec.m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", txtProj, 0);
			p_TextBox->rec.m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", p_TextBox->rec.m_Color, sizeof(Matrix4f));

			m_TxtCommandBufferCount++;
		}
		void Renderer::submit(TextBox* p_TextBox, Viewport p_Viewport)
		{			
			if (p_TextBox->NewRT)
			{
				p_TextBox->rec.m_UniformManager.setUniformBlock(ShaderStage::VertexBit, "ubo", nullptr, sizeof(Matrix4f) + sizeof(Vector4f));
				p_TextBox->rec.m_UniformManager.setUniform("glyphs", p_TextBox->getFont().glyphs, 1);
				p_TextBox->rec.m_UniformManager.create();

				p_TextBox->m_Vao.fill({ &m_RecVertices, &p_TextBox->m_InstanceTransforms }, m_RecIndices);

				m_TxtCommandBuffers.emplace_back(Level::Secondary);

				p_TextBox->NewRT = false;
			}

			// Updates the vertex buffer with instance info
			p_TextBox->m_Vao.update(1);

			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBegin(m_DefaultInheritanceInfo);
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdSetViewport(p_Viewport);
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdSetScissor(Scissor({ m_RenderResolution.x, m_RenderResolution.y }, { 0, 0 }));
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBindPipeline(m_2DTextBoxPipeline);

			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBindUniformBuffer(p_TextBox->rec.m_UniformManager);

			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdBindVertexArray(p_TextBox->m_Vao);
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdDrawIndexedInstanced(0, m_RecRawIndices.size(), p_TextBox->characterTransforms.size());
			m_TxtCommandBuffers[m_TxtCommandBufferCount].cmdEnd();

			p_TextBox->rec.m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", txtProj, 0);
			p_TextBox->rec.m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", p_TextBox->rec.m_Color, sizeof(Matrix4f));

			m_TxtCommandBufferCount++;
		}
		void Renderer::submit(Rectangle& p_Rectangle)
		{
			submit(&p_Rectangle);
		}
		void Renderer::submit(Rectangle* p_Rectangle)
		{
			recMvp = p_Rectangle->model * m_Camera->getOrthographicMatrix();

			if (p_Rectangle->NewRT)
			{
				p_Rectangle->m_UniformManager.setUniformBlock(ShaderStage::VertexBit, "ubo", &recMvp, sizeof(Matrix4f) + 2 * sizeof(Vector4f) + sizeof(Vector2f));
				p_Rectangle->m_UniformManager.create();
			
				// Previous size is the index for the new command buffer
				const int index = m_CommandBuffers.size();

				m_CommandBuffers.emplace_back(Level::Secondary);

				p_Rectangle->NewRT = false;
			}
			const int index = m_CurrentCmdBuffer;

			m_CommandBuffers[index].cmdBegin(m_MainInheritanceInfo);
			m_CommandBuffers[index].cmdSetViewport(Viewport({ m_RenderResolution.x, -m_RenderResolution.y }, { 0, m_RenderResolution.y }));
			m_CommandBuffers[index].cmdSetScissor(Scissor({ m_RenderResolution.x, m_RenderResolution.y }, { 0, 0 }));

			m_CommandBuffers[index].cmdBindPipeline(m_2DShapePipeline);
			m_CommandBuffers[index].cmdBindUniformBuffer(p_Rectangle->m_UniformManager);

			m_CommandBuffers[index].cmdBindVertexArray(m_RecVao);
			m_CommandBuffers[index].cmdDrawIndexed(0, m_RecRawIndices.size());

			m_CommandBuffers[index].cmdEnd();
			
			Vector2f thickness = Vector2f
			(
				(p_Rectangle->m_OutlineThickness * m_CurrentWindow->m_PPi) / p_Rectangle->m_Size.x,
				(p_Rectangle->m_OutlineThickness * m_CurrentWindow->m_PPi) / p_Rectangle->m_Size.y
			);
			
			Vector4f color(p_Rectangle->m_Color.x, p_Rectangle->m_Color.y, p_Rectangle->m_Color.z, p_Rectangle->m_Opacity);

			// MVP Matrix
			p_Rectangle->m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", recMvp, 0);
			p_Rectangle->m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", color, sizeof(Matrix4f));
			p_Rectangle->m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", p_Rectangle->m_OutlineColor, sizeof(Matrix4f) + sizeof(Vector4f));
			p_Rectangle->m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", thickness, sizeof(Matrix4f) + 2 * sizeof(Vector4f));

			m_CurrentCmdBuffer++;
		}

		void Renderer::submit(Circle& p_Circle)
		{
			submit(&p_Circle);
		}

		void Renderer::submit(Circle* p_Circle)
		{
			Matrix4f mvp = p_Circle->model * m_Camera->getOrthographicMatrix();

			if (renderSame)
			{
				p_Circle->m_UniformManager.setUniformBlock(ShaderStage::VertexBit, "ubo", &mvp, sizeof(Matrix4f) + 2 * sizeof(Vector4f) + sizeof(Vector2f));
				p_Circle->m_UniformManager.setUniformBlock(ShaderStage::FragmentBit, "ubo", nullptr, sizeof(Vector4f));
			
				p_Circle->m_UniformManager.m_SwapChainImageCount = m_CurrentWindow->m_SwapChain.swapChainImages.size();
				p_Circle->m_UniformManager.createDescriptorSetLayout();
				p_Circle->m_UniformManager.initDescriptorPool();
				p_Circle->m_UniformManager.createDescriptorSet();
			
				// Previous size is the index for the new command buffer
				const int index = m_CommandBuffers.size();

				m_CommandBuffers.emplace_back(Level::Secondary);
				m_CommandBuffers[index].cmdBegin(m_MainInheritanceInfo);
				m_CommandBuffers[index].cmdSetViewport(Viewport({ m_RenderResolution.x, -m_RenderResolution.y }, { 0, m_RenderResolution.y }));
				m_CommandBuffers[index].cmdSetScissor(Scissor({ m_RenderResolution.x, m_RenderResolution.y }, { 0, 0 }));

				m_CommandBuffers[index].cmdBindPipeline(m_2DCirclePipeline);
				m_CommandBuffers[index].cmdBindUniformBuffer(p_Circle->m_UniformManager);

				m_CommandBuffers[index].cmdBindVertexArray(m_RecVao);
				m_CommandBuffers[index].cmdDrawIndexed(0, m_RecRawIndices.size());

				m_CommandBuffers[index].cmdEnd();
			}
			else
			{
				const int index = m_CurrentCmdBuffer;

				m_CommandBuffers[index].cmdBegin(m_MainInheritanceInfo);
				m_CommandBuffers[index].cmdSetViewport(Viewport({ m_RenderResolution.x, -m_RenderResolution.y }, { 0, m_RenderResolution.y }));
				m_CommandBuffers[index].cmdSetScissor(Scissor({ m_RenderResolution.x, m_RenderResolution.y }, { 0, 0 }));

				m_CommandBuffers[index].cmdBindPipeline(m_2DCirclePipeline);
				m_CommandBuffers[index].cmdBindUniformBuffer(p_Circle->m_UniformManager);

				m_CommandBuffers[index].cmdBindVertexArray(m_RecVao);
				m_CommandBuffers[index].cmdDrawIndexed(0, m_RecRawIndices.size());

				m_CommandBuffers[index].cmdEnd();
			}
			
			Vector2f thickness = Vector2f
			(
				p_Circle->m_OutlineThickness / p_Circle->m_Radius,
				p_Circle->m_OutlineThickness / p_Circle->m_Radius
			);
			
			Vector4f color(p_Circle->getColor().x, p_Circle->getColor().y, p_Circle->getColor().z, p_Circle->getOpacity());

			// MVP Matrix
			p_Circle->m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", mvp, 0);
			p_Circle->m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", color, sizeof(Matrix4f));
			p_Circle->m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", p_Circle->m_OutlineColor, sizeof(Matrix4f) + sizeof(Vector4f));
			p_Circle->m_UniformManager.setUniform(ShaderStage::VertexBit, "ubo", thickness, sizeof(Matrix4f) + 2 * sizeof(Vector4f));

			m_CurrentCmdBuffer++;
		}

		void Renderer::endScene()
		{
			submitCommands();

			renderSame = false;
		}

		void Renderer::submitCommands()
		{
			//m_ThreadPool.wait();

			for (size_t i = 0; i < m_PrimaryCommandBuffers.size(); i++)
				recordPrimaryCmdBuffer(m_CurrentWindow->m_SwapChain.swapChainFramebuffers[i], i);

			m_RecordCmdBuffers = true;

			result = vkAcquireNextImageKHR(m_ContextPtr->device, m_CurrentWindow->m_SwapChain.swapChain, UINT64_MAX,
				imageAvailableSemaphore[current_frame], VK_NULL_HANDLE, &imageIndex);

			//Check if a previous frame is using this image (i.e. there is its fence to wait on)
			if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
				vkWaitForFences(m_ContextPtr->device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

			imagesInFlight[imageIndex] = inFlightFences[current_frame];

			// If swapchain needs recreation
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
				recreateSwapChain();
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
				throw std::runtime_error("failed to acquire swap chain image!");

			submitInfo.pWaitSemaphores = &imageAvailableSemaphore[current_frame];
			submitInfo.pCommandBuffers = &m_PrimaryCommandBuffers[current_frame].m_CommandBuffer;
			submitInfo.pSignalSemaphores = &renderFinishedSemaphore[current_frame];

			vkResetFences(m_ContextPtr->device, 1, &inFlightFences[current_frame]);

			// Submit the commands
			if (vkQueueSubmit(m_ContextPtr->graphicsQueue, 1, &submitInfo, inFlightFences[current_frame]) != VK_SUCCESS)
				std::cout << "failed to submit command buffer" << std::endl;

			presentInfo.pWaitSemaphores = &renderFinishedSemaphore[current_frame];
			presentInfo.pImageIndices = &imageIndex;

			imgui.updateViewports();

			result = vkQueuePresentKHR(m_ContextPtr->presentQueue, &presentInfo);

			// If swapchain needs recreation
			if (result == VK_ERROR_OUT_OF_DATE_KHR || m_CurrentWindow->windowResized())
				recreateSwapChain();

			else if (result != VK_SUCCESS)
				throw std::runtime_error("failed to present swap chain image!");

			waitForFences();

			m_CurrentCmdBuffer = 0;
			m_CurrentShadowMapCmdBuffer = 0;
			m_CurrentColorCmdBuffer = 0;
			m_TxtCommandBufferCount = 0;

			current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

			vkResetCommandPool(m_ContextPtr->device, m_ContextPtr->commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		}

		void Renderer::renderTexturedQuadHdr()
		{
			const int& index = m_CurrentCmdBuffer;
			m_CommandBuffers[index].cmdBegin(m_MainInheritanceInfo);
			m_CommandBuffers[index].cmdSetViewport(Viewport({ m_RenderResolution.x, -m_RenderResolution.y }, { 0, m_RenderResolution.y }));
			m_CommandBuffers[index].cmdSetScissor(Scissor({ m_RenderResolution.x, m_RenderResolution.y }, { 0, 0 }));

			m_CommandBuffers[index].cmdBindPipeline(m_TexturedQuadPipeline);
			m_CommandBuffers[index].cmdBindUniformBuffer(m_TexturedQaudUniformManager);

			vkCmdPushConstants(
				m_CommandBuffers[index].m_CommandBuffer,
				m_TexturedQuadPipeline.m_PipelineLayout,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(HDRFXAAInfo),
				&hdrInfo
			);

			m_CommandBuffers[index].cmdBindVertexArray(m_RecVao);
			m_CommandBuffers[index].cmdDrawIndexed(0, m_RecRawIndices.size());

			m_CommandBuffers[index].cmdEnd();

			m_CurrentCmdBuffer++;
		}

		void Renderer::recordPrimaryCmdBuffer(VkFramebuffer& p_FrameBuffer, const int p_Index)
		{
			m_PrimaryCommandBuffers[p_Index].cmdBegin();

			// Shadow map render pass
			if (m_LightDir != m_OldLightDir)
			{
				//.cmdBeginRenderPass(m_ShadowMapRenderPass, SubpassContents::Secondary, m_ShadowMapFramebuffer);

				m_ShadowMapRenderFramebuffer.cmdBeginRenderPass(m_PrimaryCommandBuffers[p_Index]);

				m_PrimaryCommandBuffers[p_Index].cmdExecuteCommands(m_ShadowMapCommandBuffers, m_CurrentShadowMapCmdBuffer);

				m_ShadowMapRenderFramebuffer.cmdEndRenderPass();

				//m_PrimaryCommandBuffers[p_Index].cmdEndRenderPass();
				m_OldLightDir = m_LightDir;
			}

			// Color for Hdr
			//.cmdBeginRenderPass(colorRenderPass, SubpassContents::Secondary, colorRenderFramebuffer.m_Framebuffer, colorRenderFramebuffer.m_Info.p_Size);

			colorRenderFramebuffer.cmdBeginRenderPass(m_PrimaryCommandBuffers[p_Index]);

			m_PrimaryCommandBuffers[p_Index].cmdExecuteCommands(m_ColorCommandBuffers, m_CurrentColorCmdBuffer);

			colorRenderFramebuffer.cmdEndRenderPass();

			m_GaussBlurFramebuffers[0].recordCmdBuffer(m_PrimaryCommandBuffers[p_Index]);
			m_GaussBlurFramebuffers[1].recordCmdBuffer(m_PrimaryCommandBuffers[p_Index]);

			// Main
			m_PrimaryCommandBuffers[p_Index].cmdBeginRenderPass(m_MainRenderPass, SubpassContents::Secondary, m_MainFramebuffer.m_Framebuffer, m_MainFramebuffer.m_Size);
			m_PrimaryCommandBuffers[p_Index].cmdExecuteCommands(m_CommandBuffers, m_CurrentCmdBuffer);
			m_PrimaryCommandBuffers[p_Index].cmdEndRenderPass();

			// Default
			m_PrimaryCommandBuffers[p_Index].cmdBeginRenderPass(m_DefaultRenderPass, SubpassContents::Secondary, p_FrameBuffer, m_CurrentWindowSize);
			m_PrimaryCommandBuffers[p_Index].cmdExecuteCommands(imgui.commandBuffer);
			m_PrimaryCommandBuffers[p_Index].cmdExecuteCommands(m_TxtCommandBuffers, m_TxtCommandBufferCount);
			//imgui.genCmdBuffers(m_PrimaryCommandBuffers[p_Index]);
			m_PrimaryCommandBuffers[p_Index].cmdEndRenderPass();
			m_PrimaryCommandBuffers[p_Index].cmdEnd();
		}
		
		void Renderer::waitForFences()
		{
			vkWaitForFences(m_ContextPtr->device, 1, &inFlightFences[current_frame], VK_TRUE, UINT64_MAX);
		}

		void Renderer::createGaussBlurFramebuffer()
		{
			m_GaussBlurFramebufferInfos[0].p_BorderColor	  = m_GaussBlurFramebufferInfos[1].p_BorderColor		= BorderColor::TransparentBlack;
			m_GaussBlurFramebufferInfos[0].p_SamplerMode	  = m_GaussBlurFramebufferInfos[1].p_SamplerMode		= SamplerMode::ClampToBorder;
			m_GaussBlurFramebufferInfos[0].p_ColorType		  = m_GaussBlurFramebufferInfos[1].p_ColorType			= ColorType::Custom;
			m_GaussBlurFramebufferInfos[0].p_CustomFormat	  = m_GaussBlurFramebufferInfos[1].p_CustomFormat		= m_CurrentWindow->m_SwapChain.swapChainImageFormat;
			m_GaussBlurFramebufferInfos[0].p_NoDepth		  = m_GaussBlurFramebufferInfos[1].p_NoDepth			= true;
			m_GaussBlurFramebufferInfos[0].p_PushConstantSize = m_GaussBlurFramebufferInfos[1].p_PushConstantSize	= sizeof(Vector2f);
			m_GaussBlurFramebufferInfos[0].p_Size			  = m_GaussBlurFramebufferInfos[1].p_Size				= m_RenderResolution;
			m_GaussBlurFramebufferInfos[0].p_RawIndices		  = m_GaussBlurFramebufferInfos[1].p_RawIndices			= m_RecRawIndices;
			m_GaussBlurFramebufferInfos[0].p_RectangleData	  = m_GaussBlurFramebufferInfos[1].p_RectangleData		= &m_RecVao;

			m_GaussBlurFramebufferInfos[0].p_UniformMananger	= &m_GaussBlurUniformMngr[0];
			m_GaussBlurFramebufferInfos[1].p_UniformMananger	= &m_GaussBlurUniformMngr[1];
			m_GaussBlurFramebufferInfos[0].p_PushConstantData	= &blurScale[0];
			m_GaussBlurFramebufferInfos[1].p_PushConstantData	= &blurScale[1];

			m_GaussBlurFramebuffers[0].create(m_GaussBlurFramebufferInfos[0]);
			m_GaussBlurFramebuffers[1].create(m_GaussBlurFramebufferInfos[1]);

			m_GaussBlurUniformMngr[0].setUniform("image", colorRenderFramebuffer.colorImgDiscarded.getImage(), 0);
			m_GaussBlurUniformMngr[0].create();
			
			m_GaussBlurUniformMngr[1].setUniform("image", m_GaussBlurFramebuffers[0].m_ImageAttachment.getImage(), 0);
			m_GaussBlurUniformMngr[1].create();
		}

		CubeMap Renderer::generateAtmosphericScatteringCubeMap(Vector2i p_Size)
		{
			Shader shader;
			shader.setShader("resources/Shaders/AtmosphericScatteringToCubeMap/vert.spv", "resources/Shaders/AtmosphericScatteringToCubeMap/frag.spv");

			CubeMap cube;
			cube.createEmpty(p_Size, VK_FORMAT_R32G32B32A32_SFLOAT);

			m_AtmosphericScatteringUM.setUniform(ShaderStage::VertexBit, "ubo...", &atmosphericScatteringInfo, sizeof(atmosphericScatteringInfo), 2 * sizeof(Matrix4f));

			RenderPass renderPass(RenderPassType::Graphics);
			AttachmentInfo color;
			color.p_AttachmentType = AttachmentType::Color;
			color.p_Format = VK_FORMAT_R32G32B32A32_SFLOAT;
			color.p_SampleCount = VK_SAMPLE_COUNT_1_BIT;
			color.p_LoadOp = LoadOp::Clear;
			color.p_StoreOp = StoreOp::Store;
			color.p_StencilLoadOp = LoadOp::Null;
			color.p_StencilStoreOp = StoreOp::Null;
			color.p_InitialLayout = Layout::Undefined;
			color.p_FinalLayout = Layout::Color;

			renderPass.addAttachment(color);

			AttachmentInfo depthAttachment;
			depthAttachment.p_AttachmentType = AttachmentType::Depth;
			depthAttachment.p_Format = m_ContextPtr->findDepthFormat();
			depthAttachment.p_SampleCount = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.p_LoadOp = LoadOp::Clear;
			depthAttachment.p_StoreOp = StoreOp::Null;
			depthAttachment.p_StencilLoadOp = LoadOp::Null;
			depthAttachment.p_StencilStoreOp = StoreOp::Null;
			depthAttachment.p_InitialLayout = Layout::Undefined;
			depthAttachment.p_FinalLayout = Layout::Depth;

			renderPass.addAttachment(depthAttachment);

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			renderPass.m_Dependencies.push_back(dependency);

			dependency.srcSubpass = 0;
			dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			renderPass.m_Dependencies.push_back(dependency);
			renderPass.create();

			Pipeline pipeline;
			PipelineInfo pipelineInfo;
			pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
			pipelineInfo.p_MSAASamples = 1;
			pipelineInfo.p_Viewport = Viewport({ p_Size.x, -p_Size.y }, { 0, p_Size.y });
			pipelineInfo.p_Scissor = Scissor(p_Size, { 0, 0 });
			pipelineInfo.p_SampleRateShading = false;
			pipelineInfo.p_RenderPass = &renderPass;
			pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_Shader = &shader;
			pipelineInfo.p_UsePushConstants = true;
			pipelineInfo.p_PushConstantShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
			pipelineInfo.p_PushConstantOffset = 0;
			pipelineInfo.p_PushConstantSize = sizeof(Matrix4f);

			exz::BufferLayout Layout = { {
					{ exz::ShaderDataType::Vec2f, 0, "in_Position" },
					{ exz::ShaderDataType::Vec2f, 1, "in_Uv" }
				},0
			};

			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(1, 0, 0, 0));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&Layout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&Layout).second);

			pipeline.create(pipelineInfo);

			ImageAttachment image;
			ImageAttachmentInfo imageInfo;
			imageInfo.p_AttachmentInfo = &color;
			imageInfo.p_BorderColor = BorderColor::TransparentBlack;
			imageInfo.p_Size = p_Size;
			imageInfo.p_TransitionLayoutImage = false;
			image.create(imageInfo);

			ImageAttachment imageDepth;
			ImageAttachmentInfo imageInfoDepth;
			imageInfoDepth.p_AttachmentInfo = &depthAttachment;
			imageInfoDepth.p_BorderColor = BorderColor::TransparentBlack;
			imageInfoDepth.p_Size = p_Size;
			imageInfoDepth.p_TransitionLayoutImage = false;
			imageDepth.create(imageInfoDepth);

			Framebuffer framebuffer;
			FramebufferInfo info;
			info.p_RenderPass = &renderPass;
			info.p_Size = p_Size;
			info.p_ImageAttachments = { image, imageDepth };
			framebuffer.create(info);

			CommandBuffer cmd(Level::Primary);

			Transform3D transform;
			Matrix4f push[6];

			transform.rotate(90, { 0, 1, 0 });
			push[0] = transform.model;

			transform.model.identity();

			transform.rotate(-90, { 0, 1, 0 });
			push[1] = transform.model;

			transform.model.identity();

			push[4] = transform.model;

			transform.model.identity();

			transform.setScale(-1, 1, -1);
			push[5] = transform.model;

			transform.model.identity();

			transform.rotate(90, { 1.0f, 0.0f, 0.0f });
			push[3] = transform.model;

			transform.model.identity();

			transform.rotate(-90, { 1.0f, 0.0f, 0.0f });
			push[2] = transform.model;

			cmd.cmdBegin();
			for (int i = 0; i < 6; i++)
			{

				cmd.cmdBeginRenderPass(renderPass, exz::vk::SubpassContents::Inline, framebuffer);
				cmd.cmdSetViewport(Viewport({ p_Size.x, -p_Size.y }, { 0, p_Size.y }));
				cmd.cmdSetScissor(Scissor(p_Size, { 0, 0 }));

				vkCmdPushConstants(
					cmd.m_CommandBuffer,
					pipeline.m_PipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT,
					0,
					sizeof(Matrix4f),
					&push[i]
				);

				cmd.cmdBindPipeline(pipeline);
				cmd.cmdBindUniformBuffer(m_AtmosphericScatteringUM);
				cmd.cmdBindVertexArray(m_RecVao);
				cmd.cmdDrawIndexed(0, m_RecRawIndices.size());

				cmd.cmdEndRenderPass();

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_Image,
					VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
				);

				VkImageSubresourceRange cubeFaceSubresourceRange = {};
				cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				cubeFaceSubresourceRange.baseMipLevel = 0;
				cubeFaceSubresourceRange.levelCount = 1;
				cubeFaceSubresourceRange.baseArrayLayer = i;
				cubeFaceSubresourceRange.layerCount = 1;

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_Image,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					cubeFaceSubresourceRange
				);

				VkImageCopy copy;
				copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copy.srcSubresource.baseArrayLayer = 0;
				copy.srcSubresource.mipLevel = 0;
				copy.srcSubresource.layerCount = 1;
				copy.srcOffset = { 0, 0, 0 };

				copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copy.dstSubresource.baseArrayLayer = i;
				copy.dstSubresource.mipLevel = 0;
				copy.dstSubresource.layerCount = 1;
				copy.dstOffset = { 0, 0, 0 };

				copy.extent.width = p_Size.x;
				copy.extent.height = p_Size.y;
				copy.extent.depth = 1;

				vkCmdCopyImage(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_Image,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					cube.m_Image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&copy
				);

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_Image,
					VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				);

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_Image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					cubeFaceSubresourceRange
				);
			}

			cmd.cmdEnd();

			submitCommandBuffer(cmd);

			cmd.destroy();
			renderPass.destroy();
			imageDepth.destroy();
			image.destroy();
			shader.destroy();
			pipeline.destroy();

			return cube;
		}

		CubeMap Renderer::generateIrradianceMap(CubeMap& p_CubeMapImage, Vector2i p_Size)
		{
			Shader shader;
			shader.setShader("resources/Shaders/GenIrradiance/vert.spv", "resources/Shaders/GenIrradiance/frag.spv");

			CubeMap cube;
			cube.createEmpty(p_Size, VK_FORMAT_R32G32B32A32_SFLOAT);

			UniformManager mng;
			mng.setUniform("hdrCubeMap", p_CubeMapImage, 0);
			mng.create();

			RenderPass renderPass(RenderPassType::Graphics);
			AttachmentInfo color;
			color.p_AttachmentType = AttachmentType::Color;
			color.p_Format = VK_FORMAT_R32G32B32A32_SFLOAT;
			color.p_SampleCount = VK_SAMPLE_COUNT_1_BIT;
			color.p_LoadOp = LoadOp::Clear;
			color.p_StoreOp = StoreOp::Store;
			color.p_StencilLoadOp = LoadOp::Null;
			color.p_StencilStoreOp = StoreOp::Null;
			color.p_InitialLayout = Layout::Undefined;
			color.p_FinalLayout = Layout::Color;

			renderPass.addAttachment(color);

			AttachmentInfo depthAttachment;
			depthAttachment.p_AttachmentType = AttachmentType::Depth;
			depthAttachment.p_Format = m_ContextPtr->findDepthFormat();
			depthAttachment.p_SampleCount = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.p_LoadOp = LoadOp::Clear;
			depthAttachment.p_StoreOp = StoreOp::Null;
			depthAttachment.p_StencilLoadOp = LoadOp::Null;
			depthAttachment.p_StencilStoreOp = StoreOp::Null;
			depthAttachment.p_InitialLayout = Layout::Undefined;
			depthAttachment.p_FinalLayout = Layout::Depth;

			renderPass.addAttachment(depthAttachment);

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			renderPass.m_Dependencies.push_back(dependency);

			dependency.srcSubpass = 0;
			dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			renderPass.m_Dependencies.push_back(dependency);
			renderPass.create();

			Pipeline pipeline;
			PipelineInfo pipelineInfo;
			pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
			pipelineInfo.p_MSAASamples = 1;
			pipelineInfo.p_Viewport = Viewport({ p_Size.x, -p_Size.y }, { 0, p_Size.y });
			pipelineInfo.p_Scissor = Scissor(p_Size, { 0, 0 });
			pipelineInfo.p_SampleRateShading = false;
			pipelineInfo.p_RenderPass = &renderPass;
			pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_Shader = &shader;
			pipelineInfo.p_UsePushConstants = true;
			pipelineInfo.p_PushConstantShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
			pipelineInfo.p_PushConstantOffset = 0;
			pipelineInfo.p_PushConstantSize = sizeof(Matrix4f);

			exz::BufferLayout Layout = { {
					{ exz::ShaderDataType::Vec2f, 0, "in_Position" },
					{ exz::ShaderDataType::Vec2f, 1, "in_Uv" }
				},0
			};

			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(0, 0, 0, 1));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&Layout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&Layout).second);

			pipeline.create(pipelineInfo);

			ImageAttachment image;
			ImageAttachmentInfo imageInfo;
			imageInfo.p_AttachmentInfo = &color;
			imageInfo.p_BorderColor = BorderColor::OpaqueBlack;
			imageInfo.p_Size = p_Size;
			imageInfo.p_TransitionLayoutImage = false;
			image.create(imageInfo);

			ImageAttachment imageDepth;
			ImageAttachmentInfo imageInfoDepth;
			imageInfoDepth.p_AttachmentInfo = &depthAttachment;
			imageInfoDepth.p_BorderColor = BorderColor::OpaqueBlack;
			imageInfoDepth.p_Size = p_Size;
			imageInfoDepth.p_TransitionLayoutImage = false;
			imageDepth.create(imageInfoDepth);

			Framebuffer framebuffer;
			FramebufferInfo info;
			info.p_RenderPass = &renderPass;
			info.p_Size = p_Size;
			info.p_ImageAttachments = { image, imageDepth };
			framebuffer.create(info);

			CommandBuffer cmd(Level::Primary);

			Transform3D transform;
			Matrix4f push[6];

			transform.rotate(90, { 0, 1, 0 });
			push[0] = transform.model;

			transform.model.identity();

			transform.rotate(-90, { 0, 1, 0 });
			push[1] = transform.model;

			transform.model.identity();

			push[4] = transform.model;

			transform.model.identity();

			transform.setScale(-1, 1, -1);
			push[5] = transform.model;

			transform.model.identity();

			transform.rotate(90, { 1.0f, 0.0f, 0.0f });
			push[3] = transform.model;

			transform.model.identity();

			transform.rotate(-90, { 1.0f, 0.0f, 0.0f });
			push[2] = transform.model;

			for (int i = 0; i < 6; i++)
			{
				cmd.cmdBegin();
				cmd.cmdBeginRenderPass(renderPass, exz::vk::SubpassContents::Inline, framebuffer);
				cmd.cmdSetViewport(Viewport({ p_Size.x, -p_Size.y }, { 0, p_Size.y }));
				cmd.cmdSetScissor(Scissor(p_Size, { 0, 0 }));

				vkCmdPushConstants(
					cmd.m_CommandBuffer,
					pipeline.m_PipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT,
					0,
					sizeof(Matrix4f),
					&push[i]
				);

				cmd.cmdBindPipeline(pipeline);
				cmd.cmdBindUniformBuffer(mng);
				cmd.cmdBindVertexArray(m_RecVao);
				cmd.cmdDrawIndexed(0, m_RecRawIndices.size());

				cmd.cmdEndRenderPass();

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_Image,
					VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
				);

				VkImageSubresourceRange cubeFaceSubresourceRange = {};
				cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				cubeFaceSubresourceRange.baseMipLevel = 0;
				cubeFaceSubresourceRange.levelCount = 1;
				cubeFaceSubresourceRange.baseArrayLayer = i;
				cubeFaceSubresourceRange.layerCount = 1;

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_Image,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					cubeFaceSubresourceRange
				);

				VkImageCopy copy;
				copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copy.srcSubresource.baseArrayLayer = 0;
				copy.srcSubresource.mipLevel = 0;
				copy.srcSubresource.layerCount = 1;
				copy.srcOffset = { 0, 0, 0 };

				copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copy.dstSubresource.baseArrayLayer = i;
				copy.dstSubresource.mipLevel = 0;
				copy.dstSubresource.layerCount = 1;
				copy.dstOffset = { 0, 0, 0 };

				copy.extent.width = p_Size.x;
				copy.extent.height = p_Size.y;
				copy.extent.depth = 1;

				vkCmdCopyImage(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_Image,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					cube.m_Image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&copy
				);

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_Image,
					VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				);

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_Image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					cubeFaceSubresourceRange
				);

				cmd.cmdEnd();

				submitCommandBuffer(cmd);

				cmd.reset();
			}

			cmd.destroy();
			renderPass.destroy();
			imageDepth.destroy();
			image.destroy();
			shader.destroy();
			mng.destroy();
			pipeline.destroy();

			return cube;
		}

		CubeMap Renderer::getCubeMapFromHDR(Image& p_HDRImage, Vector2i p_Size)
		{
			Shader shader;
			shader.setShader("resources/Shaders/HDRTOCUBE/vert.spv", "resources/Shaders/HDRTOCUBE/frag.spv");

			CubeMap cube;
			cube.createEmpty(p_Size, VK_FORMAT_R32G32B32A32_SFLOAT);

			UniformManager mng;
			mng.setUniform("hdrMap", p_HDRImage, 0);
			mng.create();

			RenderPass renderPass(RenderPassType::Graphics);
			AttachmentInfo color;
			color.p_AttachmentType = AttachmentType::Color;
			color.p_Format = VK_FORMAT_R32G32B32A32_SFLOAT;
			color.p_SampleCount = VK_SAMPLE_COUNT_1_BIT;
			color.p_LoadOp = LoadOp::Clear;
			color.p_StoreOp = StoreOp::Store;
			color.p_StencilLoadOp = LoadOp::Null;
			color.p_StencilStoreOp = StoreOp::Null;
			color.p_InitialLayout = Layout::Undefined;
			color.p_FinalLayout = Layout::Color;

			renderPass.addAttachment(color);

			AttachmentInfo depthAttachment;
			depthAttachment.p_AttachmentType = AttachmentType::Depth;
			depthAttachment.p_Format = m_ContextPtr->findDepthFormat();
			depthAttachment.p_SampleCount = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.p_LoadOp = LoadOp::Clear;
			depthAttachment.p_StoreOp = StoreOp::Null;
			depthAttachment.p_StencilLoadOp = LoadOp::Null;
			depthAttachment.p_StencilStoreOp = StoreOp::Null;
			depthAttachment.p_InitialLayout = Layout::Undefined;
			depthAttachment.p_FinalLayout = Layout::Depth;

			renderPass.addAttachment(depthAttachment);

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			renderPass.m_Dependencies.push_back(dependency);

			dependency.srcSubpass = 0;
			dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			renderPass.m_Dependencies.push_back(dependency);
			renderPass.create();

			Pipeline pipeline;
			PipelineInfo pipelineInfo;
			pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
			pipelineInfo.p_MSAASamples = 1;
			pipelineInfo.p_Viewport = Viewport({ p_Size.x, -p_Size.y }, { 0, p_Size.y });
			pipelineInfo.p_Scissor = Scissor(p_Size, { 0, 0 });
			pipelineInfo.p_SampleRateShading = false;
			pipelineInfo.p_RenderPass = &renderPass;
			pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_Shader = &shader;
			pipelineInfo.p_UsePushConstants = true;
			pipelineInfo.p_PushConstantShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
			pipelineInfo.p_PushConstantOffset = 0;
			pipelineInfo.p_PushConstantSize = sizeof(Matrix4f);

			exz::BufferLayout Layout = { {
					{ exz::ShaderDataType::Vec2f, 0, "in_Position" },
					{ exz::ShaderDataType::Vec2f, 1, "in_Uv" }
				},0
			};

			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(0, 0, 0, 1));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&Layout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&Layout).second);

			pipeline.create(pipelineInfo);

			ImageAttachment image;
			ImageAttachmentInfo imageInfo;
			imageInfo.p_AttachmentInfo = &color;
			imageInfo.p_BorderColor = BorderColor::OpaqueBlack;
			imageInfo.p_Size = p_Size;
			imageInfo.p_TransitionLayoutImage = false;
			image.create(imageInfo);

			ImageAttachment imageDepth;
			ImageAttachmentInfo imageInfoDepth;
			imageInfoDepth.p_AttachmentInfo = &depthAttachment;
			imageInfoDepth.p_BorderColor = BorderColor::OpaqueBlack;
			imageInfoDepth.p_Size = p_Size;
			imageInfoDepth.p_TransitionLayoutImage = false;
			imageDepth.create(imageInfoDepth);

			Framebuffer framebuffer;
			FramebufferInfo info;
			info.p_RenderPass = &renderPass;
			info.p_Size = p_Size;
			info.p_ImageAttachments = { image, imageDepth };
			framebuffer.create(info);

			CommandBuffer cmd(Level::Primary);

			Transform3D transform;
			Matrix4f push[6];

			transform.rotate(90, { 0, 1, 0 });
			push[0] = transform.model;

			transform.model.identity();

			transform.rotate(-90, { 0, 1, 0 });
			push[1] = transform.model;

			transform.model.identity();

			push[4] = transform.model;

			transform.model.identity();

			transform.setScale(-1, 1, -1);
			push[5] = transform.model;

			transform.model.identity();

			transform.rotate(90, { 1.0f, 0.0f, 0.0f });
			push[3] = transform.model;

			transform.model.identity();

			transform.rotate(-90, { 1.0f, 0.0f, 0.0f });
			push[2] = transform.model;

			cmd.cmdBegin();
			for (int i = 0; i < 6; i++)
			{

				cmd.cmdBeginRenderPass(renderPass, exz::vk::SubpassContents::Inline, framebuffer);
				cmd.cmdSetViewport(Viewport({ p_Size.x, -p_Size.y }, { 0, p_Size.y }));
				cmd.cmdSetScissor(Scissor(p_Size, { 0, 0 }));

				vkCmdPushConstants(
					cmd.m_CommandBuffer,
					pipeline.m_PipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT,
					0,
					sizeof(Matrix4f),
					&push[i]
				);

				cmd.cmdBindPipeline(pipeline);
				cmd.cmdBindUniformBuffer(mng);
				cmd.cmdBindVertexArray(m_RecVao);
				cmd.cmdDrawIndexed(0, m_RecRawIndices.size());

				cmd.cmdEndRenderPass();

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_Image,
					VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
				);

				VkImageSubresourceRange cubeFaceSubresourceRange = {};
				cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				cubeFaceSubresourceRange.baseMipLevel = 0;
				cubeFaceSubresourceRange.levelCount = 1;
				cubeFaceSubresourceRange.baseArrayLayer = i;
				cubeFaceSubresourceRange.layerCount = 1;

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_Image,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					cubeFaceSubresourceRange
				);

				VkImageCopy copy;
				copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copy.srcSubresource.baseArrayLayer = 0;
				copy.srcSubresource.mipLevel = 0;
				copy.srcSubresource.layerCount = 1;
				copy.srcOffset = { 0, 0, 0 };

				copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copy.dstSubresource.baseArrayLayer = i;
				copy.dstSubresource.mipLevel = 0;
				copy.dstSubresource.layerCount = 1;
				copy.dstOffset = { 0, 0, 0 };

				copy.extent.width = p_Size.x;
				copy.extent.height = p_Size.y;
				copy.extent.depth = 1;

				vkCmdCopyImage(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_Image,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					cube.m_Image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&copy
				);

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_Image,
					VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				);

				m_ContextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_Image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					cubeFaceSubresourceRange
				);
			}

			cmd.cmdEnd();

			submitCommandBuffer(cmd);

			cmd.destroy();
			renderPass.destroy();
			imageDepth.destroy();
			image.destroy();
			shader.destroy();
			mng.destroy();
			pipeline.destroy();

			return cube;
		}

		CubeMap Renderer::generatePreFilteredMaps(CubeMap& p_CubeMapImage, Vector2i p_InitialSize)
		{
			Shader shader;
			shader.setShader("resources/Shaders/GenPreFiltered/vert.spv", "resources/Shaders/GenPreFiltered/frag.spv");

			const int mipCount = static_cast<uint32_t>(std::floor(std::log2(std::max(p_InitialSize.x, p_InitialSize.y)))) + 1;

			CubeMap cube;
			cube.createEmpty(p_InitialSize, VK_FORMAT_R16G16B16A16_SFLOAT, mipCount);

			UniformManager mng;
			mng.setUniform("hdrMap", p_CubeMapImage, 0);
			mng.create();

			RenderPass renderPass(RenderPassType::Graphics);
			AttachmentInfo color;
			color.p_AttachmentType = AttachmentType::Color;
			color.p_Format = VK_FORMAT_R16G16B16A16_SFLOAT;
			color.p_SampleCount = VK_SAMPLE_COUNT_1_BIT;
			color.p_LoadOp = LoadOp::Clear;
			color.p_StoreOp = StoreOp::Store;
			color.p_StencilLoadOp = LoadOp::Null;
			color.p_StencilStoreOp = StoreOp::Null;
			color.p_InitialLayout = Layout::Undefined;
			color.p_FinalLayout = Layout::Color;

			renderPass.addAttachment(color);

			AttachmentInfo depthAttachment;
			depthAttachment.p_AttachmentType = AttachmentType::Depth;
			depthAttachment.p_Format = m_ContextPtr->findDepthFormat();
			depthAttachment.p_SampleCount = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.p_LoadOp = LoadOp::Clear;
			depthAttachment.p_StoreOp = StoreOp::Null;
			depthAttachment.p_StencilLoadOp = LoadOp::Null;
			depthAttachment.p_StencilStoreOp = StoreOp::Null;
			depthAttachment.p_InitialLayout = Layout::Undefined;
			depthAttachment.p_FinalLayout = Layout::Depth;

			renderPass.addAttachment(depthAttachment);

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			renderPass.m_Dependencies.push_back(dependency);

			dependency.srcSubpass = 0;
			dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			renderPass.m_Dependencies.push_back(dependency);
			renderPass.create();

			struct PushC
			{
				Matrix4f model;
				float roughness;
			};

			Pipeline pipeline;
			PipelineInfo pipelineInfo;
			pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
			pipelineInfo.p_MSAASamples = 1;
			pipelineInfo.p_Viewport = Viewport({ p_InitialSize.x, -p_InitialSize.y }, { 0, p_InitialSize.y });
			pipelineInfo.p_Scissor = Scissor(p_InitialSize, { 0, 0 });
			pipelineInfo.p_SampleRateShading = false;
			pipelineInfo.p_RenderPass = &renderPass;
			pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_Shader = &shader;
			pipelineInfo.p_UsePushConstants = true;
			pipelineInfo.p_PushConstantShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
			pipelineInfo.p_PushConstantOffset = 0;
			pipelineInfo.p_PushConstantSize = sizeof(PushC);

			exz::BufferLayout Layout = { {
					{ exz::ShaderDataType::Vec2f, 0, "in_Position" },
					{ exz::ShaderDataType::Vec2f, 1, "in_Uv" }
				},0
			};

			pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(0, 0, 0, 1));
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&Layout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&Layout).second);

			pipeline.create(pipelineInfo);

			ImageAttachment image;
			ImageAttachmentInfo imageInfo;
			imageInfo.p_AttachmentInfo = &color;
			imageInfo.p_BorderColor = BorderColor::OpaqueBlack;
			imageInfo.p_Size = p_InitialSize;
			imageInfo.p_TransitionLayoutImage = false;
			image.create(imageInfo);

			ImageAttachment imageDepth;
			ImageAttachmentInfo imageInfoDepth;
			imageInfoDepth.p_AttachmentInfo = &depthAttachment;
			imageInfoDepth.p_BorderColor = BorderColor::OpaqueBlack;
			imageInfoDepth.p_Size = p_InitialSize;
			imageInfoDepth.p_TransitionLayoutImage = false;
			imageDepth.create(imageInfoDepth);

			Framebuffer framebuffer;
			FramebufferInfo info;
			info.p_RenderPass = &renderPass;
			info.p_Size = p_InitialSize;
			info.p_ImageAttachments = { image, imageDepth };
			framebuffer.create(info);

			CommandBuffer cmd(Level::Primary);

			Transform3D transform;
			PushC push[6];

			transform.rotate(90, { 0, 1, 0 });
			push[0].model = transform.model;

			transform.model.identity();

			transform.rotate(-90, { 0, 1, 0 });
			push[1].model = transform.model;

			transform.model.identity();

			push[4].model = transform.model;

			transform.model.identity();

			transform.setScale(-1, 1, -1);
			push[5].model = transform.model;

			transform.model.identity();

			transform.rotate(90, { 1.0f, 0.0f, 0.0f });
			push[3].model = transform.model;

			transform.model.identity();

			transform.rotate(-90, { 1.0f, 0.0f, 0.0f });
			push[2].model = transform.model;


			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = mipCount;
			subresourceRange.layerCount = 6;

			cmd.cmdBegin();
			cmd.cmdSetViewport(Viewport({ p_InitialSize.x, -p_InitialSize.y }, { 0, p_InitialSize.y }));
			cmd.cmdSetScissor(Scissor(p_InitialSize, { 0, 0 }));

			// Change image layout for all cubemap faces to transfer destination
			m_ContextPtr->setImageLayout(
				cmd.m_CommandBuffer,
				cube.m_Image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange
			);

			for (int m = 0; m < mipCount; m++)
			{
				for (int i = 0; i < 6; i++)
				{
					cmd.cmdBeginRenderPass(renderPass, exz::vk::SubpassContents::Inline, framebuffer);

					int w = static_cast<int>(p_InitialSize.x * std::pow(0.5f, m));
					int h = static_cast<int>(p_InitialSize.y * std::pow(0.5f, m));

					cmd.cmdSetViewport(Viewport({ w, -h }, { 0, h }));
					cmd.cmdSetScissor(Scissor({ w, h }, { 0, 0 }));

					push[i].roughness = (float)m / (float)mipCount;

					vkCmdPushConstants(
						cmd.m_CommandBuffer,
						pipeline.m_PipelineLayout,
						VK_SHADER_STAGE_VERTEX_BIT,
						0,
						sizeof(PushC),
						&push[i]
					);

					cmd.cmdBindPipeline(pipeline);
					cmd.cmdBindUniformBuffer(mng);
					cmd.cmdBindVertexArray(m_RecVao);
					cmd.cmdDrawIndexed(0, m_RecRawIndices.size());

					cmd.cmdEndRenderPass();

					m_ContextPtr->setImageLayout(
						cmd.m_CommandBuffer,
						info.p_ImageAttachments[0].m_Image,
						VK_IMAGE_ASPECT_COLOR_BIT,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
					);

					VkImageCopy copy;
					copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					copy.srcSubresource.baseArrayLayer = 0;
					copy.srcSubresource.mipLevel = 0;
					copy.srcSubresource.layerCount = 1;
					copy.srcOffset = { 0, 0, 0 };

					copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					copy.dstSubresource.baseArrayLayer = i;
					copy.dstSubresource.mipLevel = m;
					copy.dstSubresource.layerCount = 1;
					copy.dstOffset = { 0, 0, 0 };

					copy.extent.width = w;
					copy.extent.height = h;
					copy.extent.depth = 1;

					vkCmdCopyImage(
						cmd.m_CommandBuffer,
						info.p_ImageAttachments[0].m_Image,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						cube.m_Image,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1,
						&copy
					);

					m_ContextPtr->setImageLayout(
						cmd.m_CommandBuffer,
						info.p_ImageAttachments[0].m_Image,
						VK_IMAGE_ASPECT_COLOR_BIT,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					);
				}
			}

			m_ContextPtr->setImageLayout(
				cmd.m_CommandBuffer,
				cube.m_Image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				subresourceRange
			);

			cmd.cmdEnd();

			submitCommandBuffer(cmd);

			cmd.destroy();
			renderPass.destroy();
			imageDepth.destroy();
			image.destroy();
			shader.destroy();
			mng.destroy();
			pipeline.destroy();

			return cube;
		}
		
		Image Renderer::generateBRDFLut(Vector2f p_Size)
		{
			Image img;
			return img;
		}

		void Renderer::submitCommandBuffer(CommandBuffer& p_CmdBuffer)
		{
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &p_CmdBuffer.m_CommandBuffer;

			vkQueueSubmit(m_ContextPtr->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(m_ContextPtr->graphicsQueue);
		}

		void Renderer::createRenderPass()
		{
			AttachmentInfo colorAttachment;
			colorAttachment.p_AttachmentType	= AttachmentType::Color;
			colorAttachment.p_Format			= m_CurrentWindow->m_SwapChain.swapChainImageFormat;
			colorAttachment.p_SampleCount		= m_CurrentWindow->maxMSAASamples;
			colorAttachment.p_LoadOp			= LoadOp::Clear;
			colorAttachment.p_StoreOp			= StoreOp::Store;
			colorAttachment.p_StencilLoadOp		= LoadOp::Null;
			colorAttachment.p_StencilStoreOp	= StoreOp::Null;
			colorAttachment.p_InitialLayout		= Layout::Undefined;
			colorAttachment.p_FinalLayout		= Layout::Color;

			m_DefaultRenderPass.addAttachment(colorAttachment);

			AttachmentInfo depthAttachment;
			depthAttachment.p_AttachmentType	= AttachmentType::Depth;
			depthAttachment.p_Format			= m_ContextPtr->findDepthFormat();
			depthAttachment.p_SampleCount		= m_CurrentWindow->maxMSAASamples;
			depthAttachment.p_LoadOp			= LoadOp::Clear;
			depthAttachment.p_StoreOp			= StoreOp::Null;
			depthAttachment.p_StencilLoadOp		= LoadOp::Null;
			depthAttachment.p_StencilStoreOp	= StoreOp::Null;
			depthAttachment.p_InitialLayout		= Layout::Undefined;
			depthAttachment.p_FinalLayout		= Layout::Depth;

			m_DefaultRenderPass.addAttachment(depthAttachment);

			AttachmentInfo resolveAttachment;
			resolveAttachment.p_AttachmentType	= AttachmentType::Resolve;
			resolveAttachment.p_Format			= m_CurrentWindow->m_SwapChain.swapChainImageFormat;
			resolveAttachment.p_SampleCount		= 1;
			resolveAttachment.p_LoadOp			= LoadOp::Null;
			resolveAttachment.p_StoreOp			= StoreOp::Store;
			resolveAttachment.p_StencilLoadOp	= LoadOp::Null;
			resolveAttachment.p_StencilStoreOp	= StoreOp::Null;
			resolveAttachment.p_InitialLayout	= Layout::Undefined;
			resolveAttachment.p_FinalLayout		= Layout::Present;

			m_DefaultRenderPass.addAttachment(resolveAttachment);

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			m_DefaultRenderPass.m_Dependencies.push_back(dependency);

			dependency.srcSubpass = 0;
			dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			m_DefaultRenderPass.m_Dependencies.push_back(dependency);

			m_DefaultRenderPass.create();

			m_DefaultInheritanceInfo = {};
			m_DefaultInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			m_DefaultInheritanceInfo.renderPass = m_DefaultRenderPass.m_RenderPass;
			m_DefaultInheritanceInfo.framebuffer = VK_NULL_HANDLE;
			m_DefaultInheritanceInfo.subpass = 0;
		}

		void Renderer::createMainRenderPass()
		{
			AttachmentInfo colorAttachment;
			colorAttachment.p_AttachmentType	= AttachmentType::Color;
			colorAttachment.p_Format			= m_CurrentWindow->m_SwapChain.swapChainImageFormat;
			colorAttachment.p_SampleCount		= m_CurrentWindow->maxMSAASamples;
			colorAttachment.p_LoadOp			= LoadOp::Clear;
			colorAttachment.p_StoreOp			= StoreOp::Store;
			colorAttachment.p_StencilLoadOp		= LoadOp::Null;
			colorAttachment.p_StencilStoreOp	= StoreOp::Null;
			colorAttachment.p_InitialLayout		= Layout::Undefined;
			colorAttachment.p_FinalLayout		= Layout::Color;

			m_MainRenderPass.addAttachment(colorAttachment);

			AttachmentInfo depthAttachment;
			depthAttachment.p_AttachmentType	= AttachmentType::Depth;
			depthAttachment.p_Format			= m_ContextPtr->findDepthFormat();
			depthAttachment.p_SampleCount		= m_CurrentWindow->maxMSAASamples;
			depthAttachment.p_LoadOp			= LoadOp::Clear;
			depthAttachment.p_StoreOp			= StoreOp::Null;
			depthAttachment.p_StencilLoadOp		= LoadOp::Null;
			depthAttachment.p_StencilStoreOp	= StoreOp::Null;
			depthAttachment.p_InitialLayout		= Layout::Undefined;
			depthAttachment.p_FinalLayout		= Layout::Depth;

			m_MainRenderPass.addAttachment(depthAttachment);

			AttachmentInfo resolveAttachment;
			resolveAttachment.p_AttachmentType	= AttachmentType::Resolve;
			resolveAttachment.p_Format			= m_CurrentWindow->m_SwapChain.swapChainImageFormat;
			resolveAttachment.p_SampleCount		= 1;
			resolveAttachment.p_LoadOp			= LoadOp::Null;
			resolveAttachment.p_StoreOp			= StoreOp::Store;
			resolveAttachment.p_StencilLoadOp	= LoadOp::Null;
			resolveAttachment.p_StencilStoreOp	= StoreOp::Null;
			resolveAttachment.p_InitialLayout	= Layout::Undefined;
			resolveAttachment.p_FinalLayout		= Layout::ShaderR;

			m_MainRenderPass.addAttachment(resolveAttachment);

			VkSubpassDependency dependency = {};
			dependency.srcSubpass		= VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass		= 0;
			dependency.srcStageMask		= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask	= VK_ACCESS_MEMORY_READ_BIT;
			dependency.dstAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags	= VK_DEPENDENCY_BY_REGION_BIT;

			m_MainRenderPass.m_Dependencies.push_back(dependency);

			dependency.srcSubpass		= 0;
			dependency.dstSubpass		= VK_SUBPASS_EXTERNAL;
			dependency.srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask		= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependency.srcAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask	= VK_ACCESS_MEMORY_READ_BIT;
			dependency.dependencyFlags	= VK_DEPENDENCY_BY_REGION_BIT;

			m_MainRenderPass.m_Dependencies.push_back(dependency);

			m_MainRenderPass.create();

			m_MainInheritanceInfo = {};
			m_MainInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			m_MainInheritanceInfo.renderPass = m_MainRenderPass.m_RenderPass;
			m_MainInheritanceInfo.framebuffer = VK_NULL_HANDLE;
			m_MainInheritanceInfo.subpass = 0;
		}

		void Renderer::createMainResources()
		{
			ImageAttachmentInfo info;
			info.p_AttachmentInfo = &m_MainRenderPass.m_AttachmentInfos[2];
			info.p_CreateSampler = true;
			info.p_BorderColor = BorderColor::OpaqueBlack;
			info.p_SamplerMode = SamplerMode::ClampToEdge;
			info.p_Size = m_RenderResolution;
			info.p_TransitionLayoutImage = false;
			m_MainImage.create(info);

			ImageAttachment depth;
			info.p_AttachmentInfo = &m_MainRenderPass.m_AttachmentInfos[1];
			info.p_CreateSampler = false;
			depth.create(info);

			ImageAttachment color;
			info.p_AttachmentInfo = &m_MainRenderPass.m_AttachmentInfos[0];
			color.create(info);

			FramebufferInfo framebufferInfo;
			framebufferInfo.p_ImageAttachments = { color, depth, m_MainImage };
			framebufferInfo.p_RenderPass = &m_MainRenderPass;
			framebufferInfo.p_Size = m_RenderResolution;
			m_MainFramebuffer.create(framebufferInfo);
		}

		void Renderer::createDepthResources()
		{
			ImageAttachmentInfo info;
			info.p_Size = m_CurrentWindow->m_WindowSize;
			info.p_AttachmentInfo = &m_DefaultRenderPass.m_AttachmentInfos[1];

			m_DepthImageAttachment.create(info);
		}

		void Renderer::createSyncObjects()
		{
			imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
			renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
			inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
			imagesInFlight.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.pNext = nullptr;
			semaphoreInfo.flags = 0;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				if (vkCreateSemaphore(m_ContextPtr->device, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
					vkCreateSemaphore(m_ContextPtr->device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
					vkCreateFence(m_ContextPtr->device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
					Utils::Logger::logMSG("Failed to create synchronization objects for frame", "Render Sync Objects", Utils::Severity::Error);
			}
		}

		void Renderer::recreateSwapChain()
		{
			waitForFences();

			if (m_CurrentWindow->getWindowSize().x <= 1)
			{
				glfwWaitEvents();
				m_CurrentWindow->setWindowSize(2, m_CurrentWindow->getWindowSize().y);
			}
			if (m_CurrentWindow->getWindowSize().y <= 1)
			{
				glfwWaitEvents();
				m_CurrentWindow->setWindowSize(m_CurrentWindow->getWindowSize().x, 2);
			}

			m_CurrentWindowSize = m_CurrentWindow->getWindowSize();

			cleanUpSwapChain();
			
			m_CurrentWindow->m_SwapChain.initSwapChain(m_CurrentWindowSize);
			
			createRenderPass();
			m_CurrentWindow->createColorResources();
			createDepthResources();

			m_CurrentWindow->m_SwapChain.initFramebuffers(m_DefaultRenderPass.m_RenderPass, m_DepthImageAttachment.m_ImageView, m_CurrentWindow->colorImageView);

			m_CurrentWindow->m_DeltaTime = 0;
			m_CurrentWindow->m_PreviousTime = Utils::Logger::getTimePoint();

			//createColorFramebuffer();
			//
			//createGaussBlurFramebuffer();
			//
			//createMainResources();
			//
			//blurScale[0] = { BlurScale / m_GaussBlurFramebuffer[0].m_Size.x, 0 };
			//blurScale[1] = { 0, BlurScale / m_GaussBlurFramebuffer[1].m_Size.y };
			//
			//m_TexturedQaudUniformManager.setUniform("imageHDR", colorImg.getImage(), 0);
			//m_TexturedQaudUniformManager.setUniform("imageHDR", m_GaussBlurImageAttachment[1].getImage(), 1);
			//m_TexturedQaudUniformManager.create();
		}

		void Renderer::cleanUpSwapChain()
		{
			//vkWaitForFences(m_ContextPtr->device, 1, &inFlightFences[current_frame], VK_TRUE, UINT64_MAX);

			m_CurrentWindow->m_SwapChain.destroy();

			m_DefaultRenderPass.destroy();

			m_CurrentWindow->destroyMSAAColorImage();

			m_DepthImageAttachment.destroy();

			//colorFramebuffer.destroy();
			//colorImg.destroy();
			//colorImgDiscarded.destroy();
			//
			//m_GaussBlurFramebuffer->destroy();
			//m_GaussBlurImageAttachment[0].destroy();
			//m_GaussBlurImageAttachment[1].destroy();
			//m_GaussBlurUniformMngr[0].destroy();
			//m_GaussBlurUniformMngr[1].destroy();
			//m_TexturedQaudUniformManager.destroy();
			//m_MainImage.destroy();
			//m_MainFramebuffer.destroy();
			
		}
	}
}
