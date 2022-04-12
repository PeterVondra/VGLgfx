#pragma once

#include "VkContext.h"
#include "VkDescriptorSetManager.h"
#include "VkRenderPass.h"
#include "VkBuffers.h"
#include "../Definitions.h"
#include "VkFramebuffer.h"

namespace vgl
{
	namespace vk
	{
		enum class Level
		{
			Primary = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			Secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY
		};

		enum class SubpassContents
		{
			Inline = VK_SUBPASS_CONTENTS_INLINE,
			Secondary = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
		};

		class CommandBuffer
		{
			public:
				CommandBuffer(Level p_Level);
				~CommandBuffer();

				void cmdBegin();
				void cmdBegin(VkCommandBufferInheritanceInfo& p_InheritanceInfo);
				void cmdEnd();

				void cmdBeginRenderPass(RenderPass& p_RenderPass, SubpassContents p_SubPassContents, Framebuffer& p_Framebuffer);
				void cmdBeginRenderPass(RenderPass& p_RenderPass, SubpassContents p_SubPassContents, Framebuffer& p_Framebuffer, 
					VkRenderPassAttachmentBeginInfo* p_Next);
				void cmdBeginRenderPass(RenderPass& p_RenderPass, SubpassContents p_SubPassContents, VkFramebuffer& p_Framebuffer, Vector2i p_Size);
				void cmdEndRenderPass();

				// Execute secondary command buffers, if this command buffer level is set to primary
				void cmdExecuteCommands(std::vector<CommandBuffer>& p_CommandBuffers);
				void cmdExecuteCommands(CommandBuffer& p_CommandBuffer);
				void cmdExecuteCommands(std::vector<CommandBuffer>& p_CommandBuffers, size_t p_Size);

				void cmdBindPipeline(g_Pipeline& p_Pipeline);
				void cmdBindDescriptorSets(DescriptorSetManager& p_DescriptorSetManager, const uint32_t p_ImageIndex);
				void cmdBindVertexArray(VertexArray& p_Vao);
				void cmdBindVertexArray16BitIdx(VertexArray& p_Vao);

				void cmdSetViewport(Viewport p_Viewport);
				void cmdSetScissor(Scissor p_Scissor);

				void cmdDrawIndexed(uint32_t p_FirstIndex, uint32_t p_LastIndex);
				void cmdDrawIndexed(uint32_t p_VertexOffset, uint32_t p_FirstIndex, uint32_t p_LastIndex);
				void cmdDrawIndexedInstanced(uint32_t p_FirstIndex, uint32_t p_LastIndex, uint32_t p_Instances);
				void cmdDraw(const uint32_t p_VertexCount);

				void cmdTransitionImageLayout(VkImage p_Image, VkFormat p_Format, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout, uint32_t p_MipLevels, VkImageAspectFlagBits p_AspectFlags);

				void destroy();
				void reset();

				static void destroy(std::vector<CommandBuffer>& p_CommandBuffers);
				static void reset(std::vector<CommandBuffer>& p_CommandBuffers);

				static CommandBuffer& beginSingleTimeCmds();
				static void endSingleTimeCmds(VkCommandBuffer& p_CommandBuffer);
				static void endSingleTimeCmds(CommandBuffer& p_CommandBuffer);

				VkCommandBuffer& vkHandle();

			protected:
			private:
				friend class Renderer;
				friend class BaseRenderer;
				friend class ForwardRenderer;
				friend class DeferredRenderer;
				friend class FramebufferAttachment;
				friend class ImGuiContext;

				Context* m_ContextPtr;
				VkCommandBuffer m_CommandBuffer;	

				const Level m_Level;
				bool m_Allocated;
				bool m_Recording;


				VkCommandBufferBeginInfo m_BeginInfo;
				VkCommandBufferAllocateInfo m_AllocationInfo;
				VkCommandBufferInheritanceInfo m_InheritanceInfo;

				g_Pipeline* m_PipelinePtr;
		};
	}
}
