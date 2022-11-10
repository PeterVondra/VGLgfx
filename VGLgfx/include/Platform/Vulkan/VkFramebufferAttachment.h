#pragma once

#define NOMINMAX

#include "VkFramebuffer.h"
#include "VkRenderPass.h"
#include "VkShader.h"
#include "VkCommandBuffer.h"
#include "VkWindow.h"

#include "../../VGL_Logger.h"

namespace vgl
{
	namespace vk
	{
		struct AttachmentDescriptor
		{
			Vector2i p_Size;
			ImageFormat p_ImageFormat;
			Layout p_Layout;

			bool p_AllowMipMapping = false;
			bool p_ClearFrame = true;

			// AttachmentInfo from RenderPass 
			AttachmentInfo p_AttachmentInfo; 

			BorderColor p_BorderColor = BorderColor::OpaqueBlack;
			SamplerMode p_SamplerMode = SamplerMode::ClampToBorder;

			// p_ClearFrame = false, image will not be cleared after image write.
			AttachmentDescriptor(
				Vector2i p_Size,
				ImageFormat p_ImageFormat,
				Layout p_Layout,
				bool p_ClearFrame = true,
				bool p_AllowMipMapping = false,
				BorderColor p_BorderColor = BorderColor::OpaqueBlack,
				SamplerMode p_SamplerMode = SamplerMode::ClampToBorder
			){
				m_CreateSampler = p_Layout == Layout::ShaderR || p_Layout == Layout::DepthR;
				
				p_AttachmentInfo.p_Format = (VkFormat)p_ImageFormat;
				p_AttachmentInfo.p_SampleCount = 1;
				p_AttachmentInfo.p_LoadOp = p_ClearFrame ? LoadOp::Clear : LoadOp::Load;
				p_AttachmentInfo.p_StoreOp = StoreOp::Store;
				p_AttachmentInfo.p_StencilLoadOp = LoadOp::Null;
				p_AttachmentInfo.p_StencilStoreOp = StoreOp::Null;
				p_AttachmentInfo.p_InitialLayout = Layout::Undefined;
				p_AttachmentInfo.p_FinalLayout = p_Layout;
				p_AttachmentInfo.p_AttachmentType = 
					((p_ImageFormat == ImageFormat::D16UN || p_ImageFormat == ImageFormat::D32SF) ? AttachmentType::Depth : AttachmentType::Color);

				m_IsValid = true;
			}
			private:
				friend class FramebufferAttachment;
				friend class GraphicsContext;

				bool m_IsValid = false;
				bool m_CreateSampler = false;
		};

		struct FramebufferAttachmentInfo
		{
			FramebufferAttachmentInfo() {};
			~FramebufferAttachmentInfo() {};

			Vector2i p_Size;
			std::vector<AttachmentDescriptor> p_AttachmentDescriptors;

			Shader* p_Shader = nullptr;
			RenderPipelineInfo p_RenderPipelineInfo = {};
			Descriptor* p_Descriptors = nullptr;

			// Only works in Vulkan
			ShaderStage p_PushConstantShaderStage;
			void* p_PushConstantData;
			// Max size = 128 bytes
			size_t p_PushConstantSize = 0;
		};

		class FramebufferAttachment
		{
			public:
				FramebufferAttachment() 
					: m_ContextPtr(&ContextSingleton::getInstance()),
					m_Framebuffer(m_ContextPtr->m_SwapchainImageCount),
					m_ImageAttachments(m_ContextPtr->m_SwapchainImageCount),
					m_RenderPass(RenderPassType::Graphics)
				{
					m_RecVertices =
					{
					    Vector2f(-1.0f, -1.0f), Vector2f(0.0f, 1.0f),
					    Vector2f(1.0f,  -1.0f), Vector2f(1.0f, 1.0f),
					    Vector2f(1.0f,   1.0f), Vector2f(1.0f, 0.0f),
					    Vector2f(-1.0f,  1.0f), Vector2f(0.0f, 0.0f)
					};

					m_RecIndices = { 0, 1, 2, 2, 3, 0 };

					m_RecBufferLayout = { {
					        { ShaderDataType::Vec2f, 0, "in_Position"  },
					        { ShaderDataType::Vec2f, 1, "in_TexCoords" }
					    },0
					};

					m_RecVertexBuffer.setLayout(m_RecBufferLayout);
					m_RecVertexBuffer.fill(m_RecVertices);
					m_RecIndexBuffer.fill(m_RecIndices);
					m_RecVao.fill(m_RecVertexBuffer, m_RecIndexBuffer);

				};
				~FramebufferAttachment() {};

				void create();
				std::vector<std::vector<ImageAttachment>>& getImageAttachments();
				std::vector<ImageAttachment>& getCurrentImageAttachments();

				const std::vector<VkCommandBufferInheritanceInfo>& getInheritanceInfo();

				Descriptor& getDescriptors(); 
				void destroy();

				AttachmentDescriptor addAttachment(
					Vector2i p_Size,
					ImageFormat p_ImageFormat,
					Layout p_Layout,
					bool p_ClearFrame = true,
					bool p_AllowMipMapping = false,
					BorderColor p_BorderColor = BorderColor::OpaqueBlack,
					SamplerMode p_SamplerMode = SamplerMode::ClampToBorder
				){ 
					return m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
						p_Size,
						p_ImageFormat,
						p_Layout,
						p_ClearFrame,
						p_AllowMipMapping,
						p_BorderColor,
						p_SamplerMode
					);
				}

				FramebufferAttachmentInfo m_FramebufferAttachmentInfo;

			protected:
			private:
				friend class Renderer;
				friend class GraphicsContext;

				bool m_AllowMipMapping = false;

				void cmdBeginRenderPass(CommandBuffer& p_CommandBuffer, SubpassContents p_SubpassContents, const uint32_t p_ImageIndex);
				void cmdEndRenderPass();
				void recordCmdBuffer(CommandBuffer& p_CommandBuffer, const uint32_t p_ImageIndex);

				Context* m_ContextPtr;

				std::vector<std::vector<ImageAttachment>> m_ImageAttachments;
				std::vector<Framebuffer> m_Framebuffer;

				RenderPass m_RenderPass;
				VkRenderPassAttachmentBeginInfo m_AttachmentBeginInfo = {};
				g_Pipeline m_Pipeline;

				Descriptor m_Descriptors;

				BufferLayout m_RecBufferLayout;
				VertexArray m_RecVao;
				VertexBuffer m_RecVertexBuffer;
				IndexBuffer m_RecIndexBuffer;
				std::vector<Vector2f> m_RecVertices;
				std::vector<uint32_t> m_RecIndices;

				CommandBuffer* m_CurrentCommandBuffer;
				std::vector<VkCommandBufferInheritanceInfo> m_InheritanceInfo;
		};
	}
}
