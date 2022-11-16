#include "VkRenderPass.h"

#include "../../VGL_Logger.h"

namespace vgl
{
	namespace vk
	{
		RenderPass::RenderPass(RenderPassType p_Type) : m_ContextPtr(&ContextSingleton::getInstance()), m_Type(p_Type), m_IsValid(false)//, m_ClearCount(m_ContextPtr->m_SwapchainImageCount)
		{
		
		}
		RenderPass::~RenderPass()
		{
	
		}

		void RenderPass::addAttachment(AttachmentInfo& p_AttachmentInfo)
		{
			VkAttachmentDescription attachment = {};
			attachment.format				= p_AttachmentInfo.p_Format;
			attachment.samples				= (VkSampleCountFlagBits)p_AttachmentInfo.p_SampleCount;
			attachment.loadOp				= (VkAttachmentLoadOp)p_AttachmentInfo.p_LoadOp;
			attachment.storeOp				= (VkAttachmentStoreOp)p_AttachmentInfo.p_StoreOp;
			attachment.stencilLoadOp		= (VkAttachmentLoadOp)p_AttachmentInfo.p_StencilLoadOp;
			attachment.stencilStoreOp		= (VkAttachmentStoreOp)p_AttachmentInfo.p_StencilStoreOp;
			attachment.initialLayout		= (VkImageLayout)p_AttachmentInfo.p_InitialLayout;
			attachment.finalLayout			= (VkImageLayout)p_AttachmentInfo.p_FinalLayout;

			//if (p_AttachmentInfo.p_LoadOp == LoadOp::Clear || p_AttachmentInfo.p_StencilLoadOp == LoadOp::Clear)
				m_ClearCount++;

			VkAttachmentReference attachmentRef = {};
			attachmentRef.attachment = m_Attachments.size(); //  addAttachment() Size 0... addAttachment() Size 1... addAttachment() Size 2
			attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			if (p_AttachmentInfo.p_AttachmentType == AttachmentType::Depth)
				attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// Size += 1
			m_Attachments.push_back(attachment);
			m_AttachmentRefs.push_back(attachmentRef);
			m_AttachmentInfo.push_back(p_AttachmentInfo);
		}

		bool RenderPass::create(void* p_Next)
		{
			if (m_Attachments.empty())
			{
				VGL_INTERNAL_WARNING("[vk::RenderPass]Failed to create render pass, no attachments submitted");
				m_IsValid = false;
				return false;
			}

			m_Subpass = {};
			m_Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			for (int i = 0; i < m_Attachments.size(); i++)
			{
				if (m_AttachmentInfo[i].p_AttachmentType == AttachmentType::Color)
				{
					m_ColorAttachments.push_back(m_AttachmentRefs[i]);
					m_Subpass.colorAttachmentCount++;
				}
				else if (m_AttachmentInfo[i].p_AttachmentType == AttachmentType::Input)
				{
					m_InputAttachments.push_back(m_AttachmentRefs[i]);
					m_Subpass.inputAttachmentCount++;
				}
				else if (m_AttachmentInfo[i].p_AttachmentType == AttachmentType::Depth)
					m_Subpass.pDepthStencilAttachment = &m_AttachmentRefs[i];

				else if (m_AttachmentInfo[i].p_AttachmentType == AttachmentType::Resolve)
					m_ResolveAttachments.push_back(m_AttachmentRefs[i]);
			}

			m_Subpass.pColorAttachments = m_ColorAttachments.data();
			m_Subpass.pResolveAttachments = m_ResolveAttachments.data();
			m_Subpass.pInputAttachments = m_InputAttachments.data();

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.pNext = p_Next;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(m_Attachments.size());
			renderPassInfo.pAttachments = m_Attachments.data();
			renderPassInfo.subpassCount = 1;

			renderPassInfo.pSubpasses = &m_Subpass;
			renderPassInfo.dependencyCount = static_cast<uint32_t>(m_Dependencies.size());
			renderPassInfo.pDependencies = m_Dependencies.data();

			VkResult result = vkCreateRenderPass(m_ContextPtr->m_Device, &renderPassInfo, nullptr, &m_RenderPass);
			VGL_INTERNAL_ASSERT_ERROR(result == VK_SUCCESS,"vk::kRenderPass]Failed to create Render pass, VkResult: %i", (uint64_t)result);

			if (result != VK_SUCCESS){
				m_IsValid = false;
				return false;
			}


			VGL_INTERNAL_TRACE("[vk::RenderPass]Succesfully created renderpass");

			m_IsValid = true;
			return true;
		}

		void RenderPass::destroy()
		{
			m_IsValid = false;

			m_Attachments.clear();
			m_AttachmentRefs.clear();

			m_ColorAttachments.clear();
			m_ResolveAttachments.clear();
			m_InputAttachments.clear();

			m_AttachmentInfo.clear();

			vkDestroyRenderPass(m_ContextPtr->m_Device, m_RenderPass, nullptr);
		}

		g_Pipeline::g_Pipeline() : m_ContextPtr(&ContextSingleton::getInstance())
		{
			m_BindPoint = PipelineBindPoint::Graphics;

			m_DynamicStates =
			{
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_LINE_WIDTH,
				VK_DYNAMIC_STATE_SCISSOR,
				VK_DYNAMIC_STATE_DEPTH_BIAS
			};
		}

		g_Pipeline::~g_Pipeline()
		{

		}

		bool g_Pipeline::create(g_PipelineInfo p_PipelineInfo)
		{
			if (m_IsValid) {
				VGL_INTERNAL_WARNING("[vk::g_Pipeline]Graphics Pipeline was already created, destroy it first!");
				return false;
			}

			m_BindPoint = PipelineBindPoint::Graphics;

			m_DynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_LINE_WIDTH,
				VK_DYNAMIC_STATE_SCISSOR,
				VK_DYNAMIC_STATE_DEPTH_BIAS
			};

			m_Info = p_PipelineInfo;

			VkPipelineShaderStageCreateInfo shaderStages[] = {
				p_PipelineInfo.p_Shader->m_VertShaderStageInfo,
				p_PipelineInfo.p_Shader->m_FragShaderStageInfo
			};

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			unsigned int vertexAttributeDescriptionCount = 0;
			std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
			for (auto& vertexAttributeDescription : p_PipelineInfo.p_AttributeDescription){
				vertexAttributeDescriptionCount += vertexAttributeDescription.size();
				for (auto& attribute : vertexAttributeDescription)
					vertexInputAttributes.push_back(attribute);
			}

			//Binding and attribute descriptions on how vertex data should be passed trough the pipeline
			vertexInputInfo.vertexBindingDescriptionCount = p_PipelineInfo.p_BindingDescription.size();
			vertexInputInfo.pVertexBindingDescriptions = p_PipelineInfo.p_BindingDescription.data();
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptionCount);
			vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

			//Get info for graphics pipeline creation
			m_InputAssemblyInfo = {};
			m_InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			m_InputAssemblyInfo.topology = (VkPrimitiveTopology)p_PipelineInfo.p_IATopology;
			m_InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

			m_Viewport.width = p_PipelineInfo.p_Viewport.m_Size.x;
			m_Viewport.height = p_PipelineInfo.p_Viewport.m_Size.y;
			m_Viewport.x = p_PipelineInfo.p_Viewport.m_Position.x;
			m_Viewport.y = p_PipelineInfo.p_Viewport.m_Position.y;

			m_Scissor.extent.width = p_PipelineInfo.p_Scissor.m_Size.x;
			m_Scissor.extent.height = p_PipelineInfo.p_Scissor.m_Size.y;
			m_Scissor.offset.x = p_PipelineInfo.p_Scissor.m_Position.x;
			m_Scissor.offset.y = p_PipelineInfo.p_Scissor.m_Position.y;

			m_ViewportInfo = {};
			m_ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			m_ViewportInfo.viewportCount = 1;
			m_ViewportInfo.pViewports = &m_Viewport;
			m_ViewportInfo.scissorCount = 1;
			m_ViewportInfo.pScissors = &m_Scissor;

			m_RasterizerInfo = {};
			m_RasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			m_RasterizerInfo.depthClampEnable = VK_FALSE;
			m_RasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
			m_RasterizerInfo.polygonMode = (VkPolygonMode)p_PipelineInfo.p_PolygonMode;
			m_RasterizerInfo.lineWidth = 1.0f;
			m_RasterizerInfo.cullMode = (VkCullModeFlags)p_PipelineInfo.p_CullMode;
			m_RasterizerInfo.frontFace = (VkFrontFace)p_PipelineInfo.p_FrontFace;
			m_RasterizerInfo.depthBiasEnable = p_PipelineInfo.p_DepthBias;
			m_RasterizerInfo.depthBiasConstantFactor = 0.0f;
			m_RasterizerInfo.depthBiasClamp = 0.0f;
			m_RasterizerInfo.depthBiasSlopeFactor = 0.0f;

			if (p_PipelineInfo.p_DepthBias){
				m_RasterizerInfo.depthBiasConstantFactor = p_PipelineInfo.p_DepthBiasConstantFactor;
				m_RasterizerInfo.depthBiasClamp = p_PipelineInfo.p_DepthBiasClamp;
				m_RasterizerInfo.depthBiasSlopeFactor = p_PipelineInfo.p_DepthBiasSlopeFactor;
			}

			m_MultiSampling = {};
			m_MultiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			m_MultiSampling.sampleShadingEnable = VK_FALSE;
			m_MultiSampling.rasterizationSamples = (VkSampleCountFlagBits)p_PipelineInfo.p_MSAASamples;
			m_MultiSampling.minSampleShading = p_PipelineInfo.p_SampleRateShading ? 0.5 : 1.0f;
			m_MultiSampling.pSampleMask = nullptr;
			m_MultiSampling.alphaToCoverageEnable = VK_FALSE;
			m_MultiSampling.alphaToCoverageEnable = VK_FALSE;


			for (int i = 0; i < p_PipelineInfo.p_RenderPass->m_ColorAttachments.size(); i++) {
				VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
				colorBlendAttachment.colorWriteMask =
					VK_COLOR_COMPONENT_R_BIT |
					VK_COLOR_COMPONENT_G_BIT |
					VK_COLOR_COMPONENT_B_BIT |
					VK_COLOR_COMPONENT_A_BIT;
				colorBlendAttachment.blendEnable = VK_FALSE;
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
				colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

				m_ColorBlendAttachments.push_back(colorBlendAttachment);
			}

			m_ColorBlending = {};
			m_ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			m_ColorBlending.logicOpEnable = VK_FALSE;
			m_ColorBlending.logicOp = VK_LOGIC_OP_COPY;
			m_ColorBlending.attachmentCount = p_PipelineInfo.p_RenderPass->m_ColorAttachments.size();
			m_ColorBlending.pAttachments = m_ColorBlendAttachments.data();
			m_ColorBlending.blendConstants[0] = 0.0f;
			m_ColorBlending.blendConstants[1] = 0.0f;
			m_ColorBlending.blendConstants[2] = 0.0f;
			m_ColorBlending.blendConstants[3] = 0.0f;

			if (p_PipelineInfo.p_AlphaBlending){
				m_ColorBlendAttachments.clear();

				for (int i = 0; i < p_PipelineInfo.p_RenderPass->m_ColorAttachments.size(); i++) {
					VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
					colorBlendAttachment = {};
					colorBlendAttachment.colorWriteMask =
						VK_COLOR_COMPONENT_R_BIT |
						VK_COLOR_COMPONENT_G_BIT |
						VK_COLOR_COMPONENT_B_BIT |
						VK_COLOR_COMPONENT_A_BIT;
					colorBlendAttachment.blendEnable = VK_TRUE;
					colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
					colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_SUBTRACT;

					m_ColorBlendAttachments.push_back(colorBlendAttachment);
				}

				m_ColorBlending = {};
				m_ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				m_ColorBlending.logicOpEnable = VK_FALSE;
				m_ColorBlending.logicOp = VK_LOGIC_OP_COPY;
				m_ColorBlending.attachmentCount = m_ColorBlendAttachments.size();
				m_ColorBlending.pAttachments = m_ColorBlendAttachments.data();
				m_ColorBlending.blendConstants[0] = 0.0f;
				m_ColorBlending.blendConstants[1] = 0.0f;
				m_ColorBlending.blendConstants[2] = 0.0f;
				m_ColorBlending.blendConstants[3] = 0.0f;
			}

			m_DynamicState = {};
			m_DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			m_DynamicState.dynamicStateCount = m_DynamicStates.size();
			m_DynamicState.pDynamicStates = m_DynamicStates.data();

			m_DepthInfo = {};
			m_DepthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			m_DepthInfo.depthTestEnable = VK_TRUE;
			m_DepthInfo.depthWriteEnable = VK_TRUE;
			m_DepthInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			m_DepthInfo.depthBoundsTestEnable = VK_FALSE;
			m_DepthInfo.maxDepthBounds = 1.0f;
			m_DepthInfo.minDepthBounds = 0.0f;
			m_DepthInfo.stencilTestEnable = VK_FALSE;
			m_DepthInfo.front = m_DepthInfo.back;
			m_DepthInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

			VkPushConstantRange pcRange = {};
			pcRange.offset = p_PipelineInfo.p_PushConstantOffset;
			pcRange.size = p_PipelineInfo.p_PushConstantSize;
			pcRange.stageFlags = p_PipelineInfo.p_PushConstantShaderStage;

			VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = p_PipelineInfo.p_DescriptorSetLayouts.size();

			if (p_PipelineInfo.p_DescriptorSetLayouts.size() > 0)
				pipelineLayoutInfo.pSetLayouts = p_PipelineInfo.p_DescriptorSetLayouts.data();
			else pipelineLayoutInfo.setLayoutCount = 0;
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;
			
			if (p_PipelineInfo.p_UsePushConstants){
				pipelineLayoutInfo.pushConstantRangeCount = 1;
				pipelineLayoutInfo.pPushConstantRanges = &pcRange;
			}

			VkResult result = vkCreatePipelineLayout(m_ContextPtr->m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
			VGL_INTERNAL_ASSERT_ERROR(result == VK_SUCCESS, "[vk::g_Pipeline]Failed to create pipeline layout, VkResult: %i", (uint64_t)result);

			if (result != VK_SUCCESS) return false;

			VkGraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &m_InputAssemblyInfo;
			pipelineInfo.pViewportState = &m_ViewportInfo;
			pipelineInfo.pRasterizationState = &m_RasterizerInfo;
			pipelineInfo.pMultisampleState = &m_MultiSampling;
			pipelineInfo.pDepthStencilState = &m_DepthInfo;
			pipelineInfo.pColorBlendState = &m_ColorBlending;
			pipelineInfo.pDynamicState = &m_DynamicState;
			pipelineInfo.layout = m_PipelineLayout;
			pipelineInfo.renderPass = p_PipelineInfo.p_RenderPass->m_RenderPass;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex = -1;
			pipelineInfo.subpass = 0;

			VGL_INTERNAL_ASSERT_ERROR(p_PipelineInfo.p_RenderPass != nullptr, "[vk::g_Pipeline]Failed to create graphics pipeline, p_RenderPass == nullptr");
			if (p_PipelineInfo.p_RenderPass == nullptr) return false;
			result = vkCreateGraphicsPipelines(m_ContextPtr->m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline);
			VGL_INTERNAL_ASSERT_ERROR(result == VK_SUCCESS, "[vk::g_Pipeline]Failed to create graphics pipeline, VkResult: %i", (uint64_t)result);
			if (result != VK_SUCCESS) return false;

			VGL_INTERNAL_TRACE("[vk::g_Pipeline]Succesfully created graphics pipeline");

			m_IsValid = true;

			return true;
		}

		void g_Pipeline::destroy()
		{
			m_IsValid = false;

			vkDestroyPipeline(m_ContextPtr->m_Device, m_Pipeline, nullptr);
			vkDestroyPipelineLayout(m_ContextPtr->m_Device, m_PipelineLayout, nullptr);
		}
	}
}