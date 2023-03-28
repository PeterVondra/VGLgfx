#include "VkCommandBuffer.h"

namespace vgl
{
	namespace vk
	{
		CommandBuffer::CommandBuffer(Level p_Level) : m_ContextPtr(&ContextSingleton::getInstance()), m_Level(p_Level), m_PipelinePtr(nullptr)
		{
			m_Recording = false;

			m_AllocationInfo = {};
			m_AllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			m_AllocationInfo.commandPool = m_ContextPtr->m_DefaultCommandPool;
			m_AllocationInfo.level = (VkCommandBufferLevel)p_Level;
			m_AllocationInfo.commandBufferCount = 1;

			if (p_Level == Level::Primary){
				VkResult result = vkAllocateCommandBuffers(m_ContextPtr->m_Device, &m_AllocationInfo, &m_CommandBuffer);
				VGL_INTERNAL_ASSERT_ERROR(result == VK_SUCCESS, "[vk::CommandBuffer]Failed to allocate PRIMARY command buffer, VkResult: %i", (uint64_t)result);
				if (result != VK_SUCCESS) m_Allocated = false;
				else m_Allocated = true;
				return;
			}

			VkResult result = vkAllocateCommandBuffers(m_ContextPtr->m_Device, &m_AllocationInfo, &m_CommandBuffer);
			VGL_INTERNAL_ASSERT_ERROR(result == VK_SUCCESS, "[vk::CommandBuffer]Failed to allocate SECONDARY command buffer, VkResult: %i", (uint64_t)result);

			if (result != VK_SUCCESS) m_Allocated = false;
			else m_Allocated = true;
		}
		CommandBuffer::~CommandBuffer()
		{
			//destroy();
		}

		void CommandBuffer::allocate(Level p_Level, uint32_t p_CommandBufferCount, std::vector<CommandBuffer>& p_CommandBuffers)
		{
			Context* context_ptr = &ContextSingleton::getInstance();

			bool allocated = false;

			VkCommandBufferAllocateInfo alloc_info = {};
			alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			alloc_info.commandPool = context_ptr->m_DefaultCommandPool;
			alloc_info.level = (VkCommandBufferLevel)p_Level;
			alloc_info.commandBufferCount = p_CommandBufferCount;

			std::vector<VkCommandBuffer> command_buffers(p_CommandBufferCount);
			p_CommandBuffers.resize(p_CommandBufferCount);

			if (p_Level == Level::Primary) {
				VkResult result = vkAllocateCommandBuffers(context_ptr->m_Device, &alloc_info, command_buffers.data());
				VGL_INTERNAL_ASSERT_ERROR(
					result == VK_SUCCESS,
					"[vk::CommandBuffer]Failed to allocate PRIMARY command buffers, buffer_count == %i, VkResult: %i",
					p_CommandBufferCount,
					(uint64_t)result
				);
				if (result != VK_SUCCESS) return;
			}
			else if (p_Level == Level::Secondary) {
				VkResult result = vkAllocateCommandBuffers(context_ptr->m_Device, &alloc_info, command_buffers.data());
				VGL_INTERNAL_ASSERT_ERROR(
					result == VK_SUCCESS,
					"[vk::CommandBuffer]Failed to allocate SECONDARY command buffers, buffer_count == %i, VkResult: %i",
					p_CommandBufferCount,
					(uint64_t)result
				);
				if (result != VK_SUCCESS) return;
			}

			for (uint32_t i = 0; i < p_CommandBufferCount; i++) {
				p_CommandBuffers[i].m_Level = p_Level;
				p_CommandBuffers[i].m_Allocated = true;
				p_CommandBuffers[i].m_AllocationInfo = alloc_info;
				p_CommandBuffers[i].m_CommandBuffer = command_buffers[i];
				p_CommandBuffers[i].m_ContextPtr = context_ptr;
				p_CommandBuffers[i].m_Recording = false;
			}
		}

		void CommandBuffer::cmdBegin()
		{
			if (m_Allocated){
				if (m_Level == Level::Primary){
					m_BeginInfo = {};
					m_BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
					m_BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
					m_BeginInfo.pInheritanceInfo = nullptr;

					VkResult result = vkBeginCommandBuffer(m_CommandBuffer, &m_BeginInfo);
					VGL_INTERNAL_ASSERT_ERROR(result == VK_SUCCESS, "[vk::CommandBuffer]Failed to begin command buffer recording, VkResult: %i", (uint64_t)result);

					if (result == VK_SUCCESS) m_Recording = true;
				}else VGL_INTERNAL_WARNING("[vk::CommandBuffer]Cannot record SECONDARY command buffer without inheritance info, command buffer did not begin recording");
			}
		}
		void CommandBuffer::cmdBegin(VkCommandBufferInheritanceInfo& p_InheritanceInfo)
		{
			if (m_Allocated){
				m_BeginInfo = {};
				m_BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				m_BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				m_BeginInfo.pInheritanceInfo = nullptr;

				if (m_Level == Level::Secondary){
					m_BeginInfo.pInheritanceInfo = &p_InheritanceInfo;
					m_InheritanceInfo = p_InheritanceInfo;
				}else
					VGL_INTERNAL_WARNING("[vk::CommandBuffer]Passed inheritance info when recording PRIMARY command buffer, recording will continue without it");

				VkResult result = vkBeginCommandBuffer(m_CommandBuffer, &m_BeginInfo);
				VGL_INTERNAL_ASSERT_ERROR(result == VK_SUCCESS, "[vk::CommandBuffer]Failed to begin command buffer recording, VkResult: %i", (uint64_t)result);
				if(result == VK_SUCCESS) m_Recording = true;
			}
		}

		void CommandBuffer::cmdEnd()
		{
			m_PipelinePtr = nullptr;

			VkResult result = vkEndCommandBuffer(m_CommandBuffer);
			VGL_INTERNAL_ASSERT_ERROR(result == VK_SUCCESS, "[vk::CommandBuffer]Failed to end command buffer recording, VkResult: %i", (uint64_t)result);

			if (result == VK_SUCCESS) m_Recording = false;
		}

		void CommandBuffer::cmdBeginRenderPass(RenderPass& p_RenderPass, SubpassContents p_SubPassContents, Framebuffer& p_Framebuffer, VkRenderPassAttachmentBeginInfo* p_Next)
		{
			if (m_Level == Level::Primary){
				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.pNext = p_Next;
				renderPassInfo.renderPass = p_RenderPass.m_RenderPass;
				renderPassInfo.framebuffer = p_Framebuffer.m_Framebuffer;
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent.width = p_Framebuffer.m_Size.x;
				renderPassInfo.renderArea.extent.height = p_Framebuffer.m_Size.y;

				VkClearValue clearValue;
				clearValue.color = { 1, 0, 1, 1 };
				clearValue.depthStencil = { 1, 0 };
				std::vector<VkClearValue> clearValues(p_RenderPass.m_ClearCount, clearValue);

				renderPassInfo.clearValueCount = clearValues.size();
				renderPassInfo.pClearValues = clearValues.data();

				vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, (VkSubpassContents)p_SubPassContents);
			}else VGL_INTERNAL_WARNING("[vk::CommandBuffer]Renderpass can't begin, command buffer level is SECONDARY");
		}
		void CommandBuffer::cmdBeginRenderPass(RenderPass& p_RenderPass, SubpassContents p_SubPassContents, VkFramebuffer& p_Framebuffer, Vector2i p_Size)
		{
			if (m_Level == Level::Primary)
			{
				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = p_RenderPass.m_RenderPass;
				renderPassInfo.framebuffer = p_Framebuffer;
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent.width = p_Size.x;
				renderPassInfo.renderArea.extent.height = p_Size.y;

				VkClearValue clearValue;
				clearValue.color = { 1, 0, 1, 1 };
				clearValue.depthStencil = { 1, 0 };
				std::vector<VkClearValue> clearValues(p_RenderPass.m_ClearCount, clearValue);

				renderPassInfo.clearValueCount = clearValues.size();
				renderPassInfo.pClearValues = clearValues.data();

				vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, (VkSubpassContents)p_SubPassContents);
			}else VGL_INTERNAL_WARNING("[vk::CommandBuffer]Renderpass can't begin, command buffer level is SECONDARY");
		}
		void CommandBuffer::cmdEndRenderPass()
		{
			if (m_Level == Level::Primary)
				vkCmdEndRenderPass(m_CommandBuffer);
			else VGL_INTERNAL_WARNING("[vk::CommandBuffer]Renderpass can't end, command buffer level is SECONDARY");
		}

		// Execute secondary command buffers, if this command buffer level is set to primary
		void CommandBuffer::cmdExecuteCommands(std::vector<CommandBuffer>& p_CommandBuffers)
		{
			if (p_CommandBuffers.size() > 0){
				std::vector<VkCommandBuffer> commandBuffers;
				for (int i = 0; i < p_CommandBuffers.size(); i++){
					if (p_CommandBuffers[i].m_Level == Level::Secondary)
						commandBuffers.push_back(p_CommandBuffers[i].m_CommandBuffer);
					else
						VGL_INTERNAL_WARNING("[vk:CommandBuffer]Secondary command buffer at index %i is not a secondary command buffer, therefore will not be executed", i);
				}
				vkCmdExecuteCommands(m_CommandBuffer, commandBuffers.size(), commandBuffers.data());
			}
		}
		// Execute secondary command buffers, if this command buffer level is set to primary
		void CommandBuffer::cmdExecuteCommands(CommandBuffer& p_CommandBuffer)
		{
			vkCmdExecuteCommands(m_CommandBuffer, 1, &p_CommandBuffer.m_CommandBuffer);
		}
		// Execute secondary command buffers, if this command buffer level is set to primary
		void CommandBuffer::cmdExecuteCommands(std::vector<CommandBuffer>& p_CommandBuffers, uint32_t p_Size)
		{
			if (p_Size > 0){
				std::vector<VkCommandBuffer> commandBuffers;
				for (int i = 0; i < p_Size; i++){
					if (p_CommandBuffers[i].m_Level == Level::Secondary)
						commandBuffers.push_back(p_CommandBuffers[i].m_CommandBuffer);
					else 
						VGL_INTERNAL_WARNING("[vk:CommandBuffer]Secondary command buffer at index %i is not a secondary command buffer, therefore will not be executed", i);
				}
				vkCmdExecuteCommands(m_CommandBuffer, p_Size, commandBuffers.data());
			}
		}

		void CommandBuffer::cmdBindPipeline(g_Pipeline& p_Pipeline)
		{
			m_PipelinePtr = &p_Pipeline;
			vkCmdBindPipeline(m_CommandBuffer, (VkPipelineBindPoint)p_Pipeline.m_BindPoint, p_Pipeline.m_Pipeline);
		}
		void CommandBuffer::cmdBindDescriptorSets(Descriptor& p_Descriptor, const uint32_t p_ImageIndex)
		{
			if (m_PipelinePtr != nullptr) {
				vkCmdBindDescriptorSets(
					m_CommandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_PipelinePtr->m_PipelineLayout,
					0, 1, &p_Descriptor.m_DescriptorSets[p_ImageIndex].m_DescriptorSet,
					0, nullptr
				);
				return;
			}
			VGL_INTERNAL_ERROR("[vk::CommandBuffer]Can't bind descriptor set, pipeline is not bound");
		}
		void CommandBuffer::cmdBindVertexArray(VertexArray& p_Vao)
		{
			std::vector<VkDeviceSize> offsets;
			for (int i = 0; i < p_Vao.buffers.size(); i++)
				offsets.push_back(0);

			if (!p_Vao.buffers.empty())
				vkCmdBindVertexBuffers(m_CommandBuffer, 0, p_Vao.m_BindingDescriptions.size(), p_Vao.buffers.data(), offsets.data());

			if (p_Vao.indexBuffer != nullptr)
				vkCmdBindIndexBuffer(m_CommandBuffer, p_Vao.indexBuffer->m_Buffer, 0, VK_INDEX_TYPE_UINT32);
		}
		void CommandBuffer::cmdBindVertexArray16BitIdx(VertexArray& p_Vao)
		{
			std::vector<VkDeviceSize> offsets;
			for (int i = 0; i < p_Vao.buffers.size(); i++)
				offsets.push_back(0);

			if (!p_Vao.buffers.empty())
				vkCmdBindVertexBuffers(m_CommandBuffer, 0, p_Vao.m_BindingDescriptions.size(), p_Vao.buffers.data(), offsets.data());

			if (p_Vao.indexBuffer->isValid())
				vkCmdBindIndexBuffer(m_CommandBuffer, p_Vao.indexBuffer->m_Buffer, 0, VK_INDEX_TYPE_UINT16);
		}

		void CommandBuffer::cmdSetViewport(Viewport p_Viewport)
		{
			VkViewport viewport;
			viewport.width = p_Viewport.m_Size.x;
			viewport.height = p_Viewport.m_Size.y;
			viewport.maxDepth = 1.0f;
			viewport.minDepth = 0.0f;
			viewport.x = p_Viewport.m_Position.x;
			viewport.y = p_Viewport.m_Position.y;

			vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);
		}
		void CommandBuffer::cmdSetScissor(Scissor p_Scissor)
		{
			VkRect2D scissor;
			scissor.extent.width = p_Scissor.m_Size.x;
			scissor.extent.height = p_Scissor.m_Size.y;
			scissor.offset.x = p_Scissor.m_Position.x;
			scissor.offset.y = p_Scissor.m_Position.y;

			vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
		}

		void CommandBuffer::cmdDrawIndexed(uint32_t p_FirstIndex, uint32_t p_LastIndex)
		{
			if (p_FirstIndex == p_LastIndex) return;
			vkCmdDrawIndexed(m_CommandBuffer, p_LastIndex - p_FirstIndex, 1, p_FirstIndex, 0, 0);
		}
		void CommandBuffer::cmdDrawIndexed(uint32_t p_VertexOffset, uint32_t p_FirstIndex, uint32_t p_LastIndex)
		{
			if (p_FirstIndex == p_LastIndex) return;
			vkCmdDrawIndexed(m_CommandBuffer, p_LastIndex - p_FirstIndex, 1, p_FirstIndex, p_VertexOffset, 0);
		}
		void CommandBuffer::cmdDrawIndexedInstanced(uint32_t p_FirstIndex, uint32_t p_LastIndex, uint32_t p_Instances)
		{
			if (p_FirstIndex == p_LastIndex) return;
			vkCmdDrawIndexed(m_CommandBuffer, p_LastIndex - p_FirstIndex, p_Instances, p_FirstIndex, 0, 0);
		}
		void CommandBuffer::cmdDraw(const uint32_t p_VertexCount)
		{
			vkCmdDraw(m_CommandBuffer, p_VertexCount, 1, 0, 0);
		}

		void CommandBuffer::cmdTransitionImageLayout(VkImage p_Image, VkFormat p_Format, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout, uint32_t p_MipLevels, VkImageAspectFlagBits p_AspectFlags)
		{
			VkPipelineStageFlags srcStage;
			VkPipelineStageFlags dstStage;

			VkImageMemoryBarrier imageBarrier = {};
			imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageBarrier.oldLayout = p_OldLayout;
			imageBarrier.newLayout = p_NewLayout;
			imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.image = p_Image;
			imageBarrier.subresourceRange.aspectMask = p_AspectFlags;
			imageBarrier.subresourceRange.baseMipLevel = 0;
			imageBarrier.subresourceRange.levelCount = p_MipLevels;
			imageBarrier.subresourceRange.baseArrayLayer = 0;
			imageBarrier.subresourceRange.layerCount = 1;
			imageBarrier.dstAccessMask = 0;
			imageBarrier.srcAccessMask = 0;

			if (p_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
				imageBarrier.srcAccessMask = 0;
				imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (p_OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
				imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (p_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
				imageBarrier.srcAccessMask = 0;
				imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else if (p_OldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
				imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (p_OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
				imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dstStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			else
				VGL_INTERNAL_ERROR("[vk::CommandBuffer/VkPipelineBarrier]Unsupported layout transition: p_OldLayout is %i and p_NewLayout is %i", p_OldLayout, p_NewLayout);

			if (p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
				imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (m_ContextPtr->hasStencilComponent(p_Format))
					imageBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}else{
				imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}


			vkCmdPipelineBarrier(m_CommandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
		}

		void CommandBuffer::destroy()
		{
			vkFreeCommandBuffers(m_ContextPtr->m_Device, m_ContextPtr->m_DefaultCommandPool, 1, &m_CommandBuffer);

			m_Recording = false;
			m_Allocated = false;

			if (m_Level == Level::Primary){
				VGL_INTERNAL_TRACE("[vk::CommandBuffer]Destroyed PRIMARY command buffer");
				return;
			}

			VGL_INTERNAL_TRACE("[vk::CommandBuffer]Destroyed SECONDARY command buffer");
		}
		void CommandBuffer::reset()
		{
			vkResetCommandBuffer(m_CommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		}

		void CommandBuffer::destroy(std::vector<CommandBuffer>& p_CommandBuffers)
		{
			std::vector<VkCommandBuffer> commandBuffers;
			for (int i = 0; i < p_CommandBuffers.size(); i++)
				commandBuffers.push_back(p_CommandBuffers[i].m_CommandBuffer);

			vkFreeCommandBuffers(ContextSingleton::getInstance().m_Device, ContextSingleton::getInstance().m_DefaultCommandPool, p_CommandBuffers.size(), commandBuffers.data());
		}
		void CommandBuffer::reset(std::vector<CommandBuffer>& p_CommandBuffers)
		{
			for (CommandBuffer& commandBuffer : p_CommandBuffers)
				commandBuffer.reset();
		}
		CommandBuffer& CommandBuffer::beginSingleTimeCmds()
		{
			CommandBuffer cmd(Level::Primary);
			cmd.cmdBegin();
			return cmd;
		}
		void CommandBuffer::endSingleTimeCmds(VkCommandBuffer& p_CommandBuffer)
		{
			ContextSingleton::getInstance().endSingleTimeCmds(p_CommandBuffer);
		}
		void CommandBuffer::endSingleTimeCmds(CommandBuffer& p_CommandBuffer)
		{
			p_CommandBuffer.cmdEnd();
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &p_CommandBuffer.vkHandle();

			Context* context_ptr = &ContextSingleton::getInstance();
			vkQueueSubmit(context_ptr->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(context_ptr->m_GraphicsQueue);
			p_CommandBuffer.destroy();
		}
		VkCommandBuffer& CommandBuffer::vkHandle()
		{
			return m_CommandBuffer;
		}
	}
}
