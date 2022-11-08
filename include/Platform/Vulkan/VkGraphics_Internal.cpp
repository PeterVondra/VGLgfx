#include "VkGraphics_Internal.h"

namespace vgl
{
	namespace vk
	{
		void GraphicsContext::init() {
			m_RecRawVertices =
			{
				Vector2f(-1.0f,	-1.0f),	Vector2f(0.0f, 1.0f),
				Vector2f(1.0f,	-1.0f),	Vector2f(1.0f, 1.0f),
				Vector2f(1.0f,	 1.0f),	Vector2f(1.0f, 0.0f),
				Vector2f(-1.0f,	 1.0f),	Vector2f(0.0f, 0.0f)
			};

			m_RecRawIndices = { 0, 1, 2, 2, 3, 0 };

			m_RecLayout = { {
					{ ShaderDataType::Vec2f, 0, "in_Position"  },
					{ ShaderDataType::Vec2f, 1, "in_TexCoords" }
				},0
			};

			m_RecVertices.setLayout(m_RecLayout);
			m_RecVertices.fill(m_RecRawVertices);
			m_RecIndices.fill(m_RecRawIndices);
			m_RecVao.fill(m_RecVertices, m_RecIndices);

			getDShadowMapShader().setShader("data/Shaders/SM3D/vert.spv", "data/Shaders/SM3D/frag.spv");
			getDShadowMapAlbedoShader().setShader("data/Shaders/SM3D/Alpha/vert.spv", "data/Shaders/SM3D/Alpha/frag.spv");

			//m_DShadowMapDescriptorSetLayout = ContextSingleton::getInstance().m_DescriptorSetLayoutCache.createLayout({
			//		DLC::Binding(DLC::DescriptorType::Uniform_Buffer, ShaderStage::VertexBit, 0)
			//	}
			//);

			m_DShadowMapAlbedoDescriptorSetLayout = ContextSingleton::getInstance().m_DescriptorSetLayoutCache.createLayout({
				//DLC::Binding(DLC::DescriptorType::Uniform_Buffer, ShaderStage::VertexBit, 0),
				DLC::Binding(DLC::DescriptorType::Combined_Image_Sampler, ShaderStage::FragmentBit, 0)
				}
			);

			vgl::BufferLayout layout =
			{
				{
					{ vgl::ShaderDataType::Vec2f, 0, "in_Position"  },
					{ vgl::ShaderDataType::Vec2f, 1, "in_TexCoords" }
				},0
			};

			m_RecVertices.setLayout(layout);
			m_RecVertices.fill(m_RecRawVertices);
			m_RecIndices.fill(m_RecRawIndices);
			m_RecVao.fill(m_RecVertices, m_RecIndices);

			{
				AttachmentInfo colorAttachment;
				colorAttachment.p_AttachmentType = AttachmentType::Depth;
				colorAttachment.p_Format = VK_FORMAT_D32_SFLOAT;
				colorAttachment.p_SampleCount = 1;
				colorAttachment.p_LoadOp = LoadOp::Clear;
				colorAttachment.p_StoreOp = StoreOp::Store;
				colorAttachment.p_StencilLoadOp = LoadOp::Null;
				colorAttachment.p_StencilStoreOp = StoreOp::Null;
				colorAttachment.p_InitialLayout = Layout::Undefined;
				colorAttachment.p_FinalLayout = Layout::DepthR;

				getDShadowMapRenderPass().addAttachment(colorAttachment);

				VkSubpassDependency dependency = {};
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

				getDShadowMapRenderPass().m_Dependencies.push_back(dependency);

				dependency.srcSubpass = 0;
				dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
				dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

				getDShadowMapRenderPass().m_Dependencies.push_back(dependency);

				getDShadowMapRenderPass().create();

				g_PipelineInfo pipelineInfo;
				pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
				pipelineInfo.p_SampleRateShading = false;
				pipelineInfo.p_AlphaBlending = false;
				pipelineInfo.p_CullMode = CullMode::BackBit;
				pipelineInfo.p_DepthBias = true;
				pipelineInfo.p_DepthBiasClamp = 0.0f;
				pipelineInfo.p_RenderPass = &getDShadowMapRenderPass();
				pipelineInfo.p_DepthBuffering = true;
				pipelineInfo.p_MSAASamples = 1;
				pipelineInfo.p_PushConstantShaderStage = getShaderStageVkH(ShaderStage::VertexBit);
				pipelineInfo.p_PushConstantSize = sizeof(Matrix4f);
				pipelineInfo.p_UsePushConstants = true;

				BufferLayout m_ShadowDepthMapLayout = { {
						{ ShaderDataType::Vec3f, 0, "in_Position"  },
						{ ShaderDataType::Vec2f, 1, "in_Uv"		},
						{ ShaderDataType::Vec3f, 2, "in_Normal"    },
						{ ShaderDataType::Vec3f, 3, "in_Tangent"   },
						{ ShaderDataType::Vec3f, 4, "in_Bitangent" }
					},0
				};

				pipelineInfo.p_Shader = &getDShadowMapShader();
				//pipelineInfo.p_DescriptorSetLayout = getDShadowMapDescriptorSetLayout();
				pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&m_ShadowDepthMapLayout).first);
				pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&m_ShadowDepthMapLayout).second);

				getDShadowMapPipeline().create(pipelineInfo);

				pipelineInfo.p_Shader = &getDShadowMapAlbedoShader();
				pipelineInfo.p_DescriptorSetLayout = getDShadowMapAlbedoDescriptorSetLayout();

				getDShadowMapAlbedoPipeline().create(pipelineInfo);
			}
		}

		g_Pipeline& GraphicsContext::getAtmosphericScatteringPipeline() 
		{ 
			return m_AtmosphericScatteringPipeline; 
		}
		g_Pipeline& GraphicsContext::getSkyBoxPipeline()
		{ 
			return m_SkyBoxPipeline;
		}
		Shader& GraphicsContext::getSkyBoxShader() 
		{ 
			return m_SkyBoxShader;
		}
		//static g_Pipeline& getAtmosphericScatteringPipelineC16SF() { static g_Pipeline m_AtmosphericScatteringPipeline; return m_AtmosphericScatteringPipeline; }
		//static g_Pipeline& getAtmosphericScatteringPipelineC16SFD() { static g_Pipeline m_AtmosphericScatteringPipeline; return m_AtmosphericScatteringPipeline; }


		Shader& GraphicsContext::getAtmosphericScatteringShader() 
		{ 
			return m_AtmosphericScatteringShader;
		}
		//inline static g_Pipeline& getSkyBoxPipeline() { static g_Pipeline m_SkyBoxPipeline; return m_SkyBoxPipeline; }
		//inline static Shader& getSkyBoxShader() { static Shader m_SkyBoxShader; return m_SkyBoxShader; }

		Shader& GraphicsContext::getDShadowMapShader() 
		{ 
			return m_DShadowMapShader;
		}
		Shader& GraphicsContext::getDShadowMapAlbedoShader() 
		{
			return m_DShadowMapAlbedoShader;
		}
		g_Pipeline& GraphicsContext::getDShadowMapPipeline() 
		{ 
			return m_DShadowMapPipeline;
		}
		g_Pipeline& GraphicsContext::getDShadowMapAlbedoPipeline() 
		{ 
			return m_DShadowMapAlbedoPipeline;
		}
		RenderPass& GraphicsContext::getDShadowMapRenderPass() 
		{ 
			return m_DShadowMapRenderPass;
		}
		VkDescriptorSetLayout& GraphicsContext::getDShadowMapDescriptorSetLayout() {
			return m_DShadowMapDescriptorSetLayout;
		}
		VkDescriptorSetLayout& GraphicsContext::getDShadowMapAlbedoDescriptorSetLayout() {
			return m_DShadowMapAlbedoDescriptorSetLayout;
		}

		const VertexArray& GraphicsContext::getRecVao() { return m_RecVao; }

		VertexBuffer& GraphicsContext::getRecVertexBuffer()
		{
			return m_RecVertices;
		}

		IndexBuffer& GraphicsContext::getRecIndexBuffer()
		{
			return m_RecIndices; 
		}

		void GraphicsContext::createSkyboxPipelines(RenderPass& p_RenderPass) {
			Context* contextPtr = &ContextSingleton::getInstance();

			g_PipelineInfo pipelineInfo;
			pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
			pipelineInfo.p_SampleRateShading = false;
			pipelineInfo.p_Shader = &getDShadowMapShader();
			pipelineInfo.p_AlphaBlending = false;
			pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_MSAASamples = 1;

			//=====================================================================================//
			// Create pipeline for skybox
			//=====================================================================================//
			getSkyBoxShader().setShader("data/Shaders/SkyBox/vert.spv", "data/Shaders/SkyBox/frag.spv");

			pipelineInfo.p_Shader = &getSkyBoxShader();
			pipelineInfo.p_CullMode = CullMode::FrontBit;
			pipelineInfo.p_DepthBuffering = false;
			pipelineInfo.p_RenderPass = &p_RenderPass;

			BufferLayout _SkyBoxLayout = { {
					{ ShaderDataType::Vec3f, 0, "in_Position"  },
					{ ShaderDataType::Vec2f, 1, "in_Uv"		},
					{ ShaderDataType::Vec3f, 2, "in_Normal"    },
					{ ShaderDataType::Vec3f, 3, "in_Tangent"   },
					{ ShaderDataType::Vec3f, 4, "in_Bitangent" }
				},0
			};

			auto layout = contextPtr->m_DescriptorSetLayoutCache.createLayout({
				DLC::Binding(DLC::DescriptorType::Uniform_Buffer, ShaderStage::VertexBit, 0),
				DLC::Binding(DLC::DescriptorType::Combined_Image_Sampler, ShaderStage::FragmentBit, 1)
				}
			);
			pipelineInfo.p_DescriptorSetLayout = layout;

			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_SkyBoxLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_SkyBoxLayout).second);

			getSkyBoxPipeline().create(pipelineInfo);

			//=====================================================================================//
			// Create pipeline for Atmospheric Scattering
			//=====================================================================================//
			getAtmosphericScatteringShader().setShader("data/Shaders/AtmosphericScattering/vert.spv", "data/Shaders/AtmosphericScattering/frag.spv");

			contextPtr->m_DescriptorSetLayoutCache.createLayout({
				DLC::Binding(DLC::DescriptorType::Uniform_Buffer, ShaderStage::FragmentBit, 0)
				}
			);
			pipelineInfo.p_DescriptorSetLayout = layout;
			pipelineInfo.p_Shader = &getAtmosphericScatteringShader();
			pipelineInfo.p_PushConstantSize = sizeof(AtmosphericScatteringInfo);
			pipelineInfo.p_PushConstantShaderStage = getShaderStageVkH(ShaderStage::FragmentBit);
			pipelineInfo.p_UsePushConstants = true;

			pipelineInfo.p_RenderPass = &p_RenderPass;
			getAtmosphericScatteringPipeline().create(pipelineInfo);
			pipelineInfo.p_RenderPass = &p_RenderPass;
			getAtmosphericScatteringPipeline().create(pipelineInfo);
		}

		void GraphicsContext::createShapesPipelines(RenderPass& p_RenderPass)
		{
			Context* contextPtr = &ContextSingleton::getInstance();

			g_PipelineInfo pipelineInfo;
			//pipelineInfo.p_CullMode = CullMode::BackBit;
			pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
			pipelineInfo.p_MSAASamples = 1;
			pipelineInfo.p_SampleRateShading = false;
			pipelineInfo.p_AlphaBlending = true;

			/*
			//=====================================================================================//
			// Create pipeline for 2D circles
			//=====================================================================================//
			m_Circle2DShader.setShader("resources/Shaders/Circle2D/vert.spv", "resources/Shaders/Circle2D/frag.spv");
			pipelineInfo.p_Shader = &m_Circle2DShader;
			pipelineInfo.p_RenderPass = &p_RenderPass;

			vgl::BufferLayout _2DCircleLayout = { {
					{ vgl::ShaderDataType::Vec2f, 0, "in_Position" },
					{ vgl::ShaderDataType::Vec2f, 1, "in_Uv" }
				},0
			};

			auto layout_circle = contextPtr->m_DescriptorSetLayoutCache.createDescriptorSetLayout({ {0} }, { {1} });
			pipelineInfo.p_DescriptorSetLayout = &layout_circle;
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_2DCircleLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_2DCircleLayout).second);

			m_Circle2DPipeline.create(pipelineInfo);

			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();

			//=====================================================================================//
			// Create pipeline for 2D shapes
			//=====================================================================================//
			m_Shape2DShader.setShader("resources/Shaders/Shape2D/vert.spv", "resources/Shaders/Shape2D/frag.spv");
			pipelineInfo.p_Shader = &m_Shape2DShader;
			pipelineInfo.p_AlphaBlending = true;

			vgl::BufferLayout _Shape2DLayout = { {
					{ vgl::ShaderDataType::Vec2f, 0, "in_Position" },
					{ vgl::ShaderDataType::Vec2f, 1, "in_Uv" }
				},0
			};

			auto layout_shape = contextPtr->m_DescriptorSetLayoutCache.createDescriptorSetLayout({ {0} });
			pipelineInfo.p_DescriptorSetLayout = &layout_shape;
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_Shape2DLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_Shape2DLayout).second);

			m_Shape2DPipeline.create(pipelineInfo);

			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();

			pipelineInfo.p_UsePushConstants = false;

			//=====================================================================================//
			// Create pipeline for 2D text
			//=====================================================================================//
			m_TextBoxShader.setShader("resources/Shaders/TextBox/vert.spv", "resources/Shaders/TextBox/frag.spv");
			pipelineInfo.p_Shader = &m_TextBoxShader;
			pipelineInfo.p_AlphaBlending = true;

			vgl::BufferLayout _2DTextBoxLayout = { {
					{ vgl::ShaderDataType::Vec2f, 0, "in_Position" },
					{ vgl::ShaderDataType::Vec2f, 1, "in_Uv" }
				},0
			};
			vgl::BufferLayout _2DTextBoxInstancesLayout = { {
				{ vgl::ShaderDataType::Int, 2, "in_InstanceGlyphIndex" },
				{ vgl::ShaderDataType::Mat4f, 3, "in_InstancePosition" },
			},1 };

			auto layout_text = contextPtr->m_DescriptorSetLayoutCache.createDescriptorSetLayout({ {0} }, {}, { {1, 127} });
			pipelineInfo.p_DescriptorSetLayout = &layout_text;
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_2DTextBoxLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_2DTextBoxLayout).second);
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_2DTextBoxInstancesLayout, InputRate::Instance).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_2DTextBoxInstancesLayout, InputRate::Instance).second);

			m_TextBoxPipeline.create(pipelineInfo);

			pipelineInfo.p_AttributeDescription.clear();
			pipelineInfo.p_BindingDescription.clear();
			*/
			//=====================================================================================//
			// Create pipeline for 3D Particles
			//=====================================================================================//
			m_ParticleShader.setShader("data/Shaders/Particles/vert.spv", "data/Shaders/Particles/frag.spv");
			pipelineInfo.p_Shader = &m_ParticleShader;
			pipelineInfo.p_AlphaBlending = true;
			pipelineInfo.p_DepthBuffering = false;
			pipelineInfo.p_UsePushConstants = true;
			pipelineInfo.p_PushConstantOffset = 0;
			pipelineInfo.p_PushConstantShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
			pipelineInfo.p_PushConstantSize = sizeof(Matrix4f);
			pipelineInfo.p_RenderPass = &p_RenderPass;

			vgl::BufferLayout _ParticleLayout = { {
					{ vgl::ShaderDataType::Vec2f, 0, "inPosition" },
					{ vgl::ShaderDataType::Vec2f, 1, "inUv" }
				},0
			};
			vgl::BufferLayout _ParticleInstancesLayout = { {
				{ vgl::ShaderDataType::Float, 2, "inInstanceLife" },
				{ vgl::ShaderDataType::Vec4f, 3, "inInstanceColor" },
				{ vgl::ShaderDataType::Vec4f, 4, "inInstanceOutlineColor" },
				{ vgl::ShaderDataType::Vec2f, 5, "inInstanceOutlineThickness" },
				{ vgl::ShaderDataType::Mat4f, 6, "in_InstanceTransform" },
			},1 };

			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_ParticleLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_ParticleLayout).second);
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_ParticleInstancesLayout, InputRate::Instance).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_ParticleInstancesLayout, InputRate::Instance).second);

			m_ParticlePipeline.create(pipelineInfo);
		}

		ImageCube GraphicsContext::generateAtmosphericScatteringCubeMap(Vector2i p_Size, AtmosphericScatteringInfo& p_Info)
		{
			Context* contextPtr = &ContextSingleton::getInstance();

			Shader shader;
			shader.setShader("data/Shaders/AtmosphericScatteringToCubeMap/vert.spv", "data/Shaders/AtmosphericScatteringToCubeMap/frag.spv");

			ImageCube cube;
			cube.createEmpty(p_Size, VK_FORMAT_R32G32B32A32_SFLOAT);

			DescriptorSetInfo descriptorSetInfo;
			descriptorSetInfo.p_VertexUniformBuffer = UniformBuffer(sizeof(p_Info), 0);
			DescriptorSetManager mng;
			mng.create(descriptorSetInfo);

			mng.copy(ShaderStage::VertexBit, p_Info, 0);

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
			depthAttachment.p_Format = contextPtr->findDepthFormat();
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

			g_Pipeline pipeline;
			g_PipelineInfo pipelineInfo;
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

			auto layout = contextPtr->m_DescriptorSetLayoutCache.createLayout({
				DLC::Binding(DLC::DescriptorType::Uniform_Buffer, ShaderStage::VertexBit, 0)
				}
			);
			pipelineInfo.p_DescriptorSetLayout = layout;
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&m_RecLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&m_RecLayout).second);

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

				cmd.cmdBeginRenderPass(renderPass, SubpassContents::Inline, framebuffer);
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
				cmd.cmdBindDescriptorSets(mng, 0);
				cmd.cmdBindVertexArray(m_RecVao);
				cmd.cmdDrawIndexed(0, m_RecRawIndices.size());

				cmd.cmdEndRenderPass();

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_VkImageHandle,
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

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_VkImageHandle,
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
					info.p_ImageAttachments[0].m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					cube.m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&copy
				);

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_VkImageHandle,
					VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				);

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					cubeFaceSubresourceRange
				);
			}

			cmd.cmdEnd();

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmd.m_CommandBuffer;

			vkQueueSubmit(contextPtr->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(contextPtr->m_GraphicsQueue);

			cmd.destroy();
			renderPass.destroy();
			imageDepth.destroy();
			image.destroy();
			shader.destroy();
			pipeline.destroy();

			return cube;
		}

		ImageCube GraphicsContext::generateIrradianceMap(ImageCube& p_CubeMapImage, Vector2i p_Size)
		{
			Context* contextPtr = &ContextSingleton::getInstance();

			Shader shader;
			shader.setShader("data/Shaders/GenIrradiance/vert.spv", "data/Shaders/GenIrradiance/frag.spv");

			ImageCube cube;
			cube.createEmpty(p_Size, VK_FORMAT_R32G32B32A32_SFLOAT);

			DescriptorSetInfo descriptorSetInfo;
			descriptorSetInfo.p_CubeMapDescriptors.emplace_back(&p_CubeMapImage, 0);
			DescriptorSetManager mng;
			mng.create(descriptorSetInfo);

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
			depthAttachment.p_Format = contextPtr->findDepthFormat();
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

			g_Pipeline pipeline;
			g_PipelineInfo pipelineInfo;
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

			auto layout = contextPtr->m_DescriptorSetLayoutCache.createLayout({
				DLC::Binding(DLC::DescriptorType::Combined_Image_Sampler, ShaderStage::FragmentBit, 0)
				}
			);
			pipelineInfo.p_DescriptorSetLayout = layout;
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&m_RecLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&m_RecLayout).second);

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
				cmd.cmdBeginRenderPass(renderPass, SubpassContents::Inline, framebuffer);
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
				cmd.cmdBindDescriptorSets(mng, 0);
				cmd.cmdBindVertexArray(m_RecVao);
				cmd.cmdDrawIndexed(0, m_RecRawIndices.size());

				cmd.cmdEndRenderPass();

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_VkImageHandle,
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

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_VkImageHandle,
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
					info.p_ImageAttachments[0].m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					cube.m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&copy
				);

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_VkImageHandle,
					VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				);

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					cubeFaceSubresourceRange
				);

				cmd.cmdEnd();

				VkSubmitInfo submitInfo = {};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &cmd.m_CommandBuffer;

				vkQueueSubmit(contextPtr->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
				vkQueueWaitIdle(contextPtr->m_GraphicsQueue);


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

		ImageCube GraphicsContext::getCubeMapFromHDR(Image& p_HDRImage, Vector2i p_Size)
		{
			Context* contextPtr = &ContextSingleton::getInstance();

			Shader shader;
			shader.setShader("data/Shaders/HDRTOCUBE/vert.spv", "data/Shaders/HDRTOCUBE/frag.spv");

			ImageCube cube;
			cube.createEmpty(p_Size, VK_FORMAT_R32G32B32A32_SFLOAT);

			DescriptorSetInfo descriptorSetInfo;
			descriptorSetInfo.p_ImageDescriptors.emplace_back(&p_HDRImage, 0);
			DescriptorSetManager mng;
			mng.create(descriptorSetInfo);

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
			depthAttachment.p_Format = contextPtr->findDepthFormat();
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

			g_Pipeline pipeline;
			g_PipelineInfo pipelineInfo;
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

			auto layout = contextPtr->m_DescriptorSetLayoutCache.createLayout({
				DLC::Binding(DLC::DescriptorType::Combined_Image_Sampler, ShaderStage::FragmentBit, 0)
				}
			);
			pipelineInfo.p_DescriptorSetLayout = layout;
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&m_RecLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&m_RecLayout).second);

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

				cmd.cmdBeginRenderPass(renderPass, SubpassContents::Inline, framebuffer);
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
				cmd.cmdBindDescriptorSets(mng, 0);
				cmd.cmdBindVertexArray(m_RecVao);
				cmd.cmdDrawIndexed(0, m_RecRawIndices.size());

				cmd.cmdEndRenderPass();

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_VkImageHandle,
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

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_VkImageHandle,
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
					info.p_ImageAttachments[0].m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					cube.m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&copy
				);

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					info.p_ImageAttachments[0].m_VkImageHandle,
					VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				);

				contextPtr->setImageLayout(
					cmd.m_CommandBuffer,
					cube.m_VkImageHandle,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					cubeFaceSubresourceRange
				);
			}

			cmd.cmdEnd();

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmd.m_CommandBuffer;

			vkQueueSubmit(contextPtr->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(contextPtr->m_GraphicsQueue);

			cmd.destroy();
			renderPass.destroy();
			imageDepth.destroy();
			image.destroy();
			shader.destroy();
			mng.destroy();
			pipeline.destroy();

			return cube;
		}

		ImageCube GraphicsContext::generatePreFilteredMaps(ImageCube& p_CubeMapImage, Vector2i p_InitialSize)
		{
			Context* contextPtr = &ContextSingleton::getInstance();

			Shader shader;
			shader.setShader("data/Shaders/GenPreFiltered/vert.spv", "data/Shaders/GenPreFiltered/frag.spv");
#undef max
			const int mipCount = static_cast<uint32_t>(std::floor(std::log2(std::max(p_InitialSize.x, p_InitialSize.y)))) + 1;

			ImageCube cube;
			cube.createEmpty(p_InitialSize, VK_FORMAT_R16G16B16A16_SFLOAT, mipCount);

			DescriptorSetInfo descriptorSetInfo;
			descriptorSetInfo.p_CubeMapDescriptors.emplace_back(&p_CubeMapImage, 0);
			DescriptorSetManager mng;
			mng.create(descriptorSetInfo);

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
			depthAttachment.p_Format = contextPtr->findDepthFormat();
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

			g_Pipeline pipeline;
			g_PipelineInfo pipelineInfo;
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

			auto layout = contextPtr->m_DescriptorSetLayoutCache.createLayout({
				DLC::Binding(DLC::DescriptorType::Combined_Image_Sampler, ShaderStage::FragmentBit, 0)
				}
			);
			pipelineInfo.p_DescriptorSetLayout = layout;
			pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&m_RecLayout).first);
			pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&m_RecLayout).second);

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
			contextPtr->setImageLayout(
				cmd.m_CommandBuffer,
				cube.m_VkImageHandle,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange
			);

			for (int m = 0; m < mipCount; m++)
			{
				for (int i = 0; i < 6; i++)
				{
					cmd.cmdBeginRenderPass(renderPass, SubpassContents::Inline, framebuffer);

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
					cmd.cmdBindDescriptorSets(mng, 0);
					cmd.cmdBindVertexArray(m_RecVao);
					cmd.cmdDrawIndexed(0, m_RecRawIndices.size());

					cmd.cmdEndRenderPass();

					contextPtr->setImageLayout(
						cmd.m_CommandBuffer,
						info.p_ImageAttachments[0].m_VkImageHandle,
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
						info.p_ImageAttachments[0].m_VkImageHandle,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						cube.m_VkImageHandle,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1,
						&copy
					);

					contextPtr->setImageLayout(
						cmd.m_CommandBuffer,
						info.p_ImageAttachments[0].m_VkImageHandle,
						VK_IMAGE_ASPECT_COLOR_BIT,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					);
				}
			}

			contextPtr->setImageLayout(
				cmd.m_CommandBuffer,
				cube.m_VkImageHandle,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				subresourceRange
			);

			cmd.cmdEnd();

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmd.m_CommandBuffer;

			vkQueueSubmit(contextPtr->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(contextPtr->m_GraphicsQueue);

			cmd.destroy();
			renderPass.destroy();
			imageDepth.destroy();
			image.destroy();
			shader.destroy();
			mng.destroy();
			pipeline.destroy();

			return cube;
		}
	}
}