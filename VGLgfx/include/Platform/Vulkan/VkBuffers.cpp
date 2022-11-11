#include "VkBuffers.h"
#include "../../VGL_Logger.h"

namespace vgl
{
	namespace vk
	{
		IndexBuffer::IndexBuffer() : m_ContextPtr(&ContextSingleton::getInstance())
		{

		}

		IndexBuffer::~IndexBuffer()
		{
			//destroy();
		}

		void IndexBuffer::destroy()
		{
			VGL_INTERNAL_TRACE("[vk::IndexBuffer]Destroyed Index buffer %i", m_Buffer);

			m_ContextPtr->deviceWaitIdle();
			m_ContextPtr->destroyBuffer(m_Buffer, m_AllocInfo.p_Alloc);
			m_AlreadyDestroyed = true;
			m_Data = nullptr;
			m_IsValid = false;
		}

		IndexBuffer::IndexBuffer(std::vector<uint32_t>* p_BufferData) : m_ContextPtr(&ContextSingleton::getInstance())
		{
			m_Count = p_BufferData->size();
			fill(p_BufferData);
		}

		IndexBuffer::IndexBuffer(std::vector<uint32_t>& p_BufferData) : m_ContextPtr(&ContextSingleton::getInstance())
		{
			m_Count = p_BufferData.size();
			fill(p_BufferData);
		}

		void IndexBuffer::fill(std::vector<uint32_t>& p_BufferData)
		{
			m_Count = p_BufferData.size();
			fill(&p_BufferData);
		}

		void IndexBuffer::fill(std::vector<uint32_t>* p_BufferData)
		{
			VGL_INTERNAL_ASSERT_WARNING(p_BufferData != nullptr, "[vk::Buffer]Attempted to allocate Index buffer with 'p_BufferData' == nullptr, buffer will not be allocated");
			if (!p_BufferData) return;

			m_Count = p_BufferData->size();

			m_BufferSize = sizeof(p_BufferData->operator[](0)) * p_BufferData->size();

			m_StagingBufferAlloc = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_StagingBuffer).p_Alloc;

			auto data = m_ContextPtr->mapMemory(m_StagingBufferAlloc);
			memcpy(data, p_BufferData->data(), m_BufferSize);
			m_ContextPtr->unmapMemory(m_StagingBufferAlloc);

			m_AllocInfo = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);

			m_ContextPtr->copyBuffer(m_StagingBuffer, m_Buffer, m_BufferSize);

			m_ContextPtr->destroyBuffer(m_StagingBuffer, m_StagingBufferAlloc);

			flush();
		}
		void IndexBuffer::fill(void* p_BufferData, const uint32_t p_Size)
		{
			m_BufferSize = p_Size;

			VGL_INTERNAL_ASSERT_WARNING(p_BufferData != nullptr, "[vk::Buffer]Attempted to allocate Index buffer with 'p_BufferData' == nullptr, buffer will not be allocated");

			if (!p_BufferData) return;

			m_Count = p_Size/sizeof(uint32_t);

			m_StagingBufferAlloc = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_StagingBuffer).p_Alloc;

			auto data = m_ContextPtr->mapMemory(m_StagingBufferAlloc);
			memcpy(data, p_BufferData, m_BufferSize);
			m_ContextPtr->unmapMemory(m_StagingBufferAlloc);

			m_AllocInfo = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);

			m_ContextPtr->copyBuffer(m_StagingBuffer, m_Buffer, m_BufferSize);

			m_ContextPtr->destroyBuffer(m_StagingBuffer, m_StagingBufferAlloc);

			flush();
		}

		VertexArray::VertexArray()
		{

		}

		VertexArray::VertexArray(VertexBuffer& p_VertexBuffer)
		{
			fill(p_VertexBuffer);
		}

		VertexArray::VertexArray(std::vector<VertexBuffer*>& p_VertexBuffers)
		{
			fill(p_VertexBuffers);
		}

		VertexArray::VertexArray(const std::initializer_list<VertexBuffer*>& p_VertexBuffers)
		{
			fill(p_VertexBuffers);
		}

		VertexArray::VertexArray(std::vector<VertexBuffer*>& p_VertexBuffers, IndexBuffer& p_IndexBuffer)
		{
			fill(p_VertexBuffers, p_IndexBuffer);
		}

		VertexArray::VertexArray(const std::initializer_list<VertexBuffer*>& p_VertexBuffers, IndexBuffer& p_IndexBuffer)
		{
			fill(p_VertexBuffers, p_IndexBuffer);
		}

		VertexArray::VertexArray(VertexBuffer& p_VertexBuffer, IndexBuffer& p_IndexBuffer)
		{
			fill(p_VertexBuffer, p_IndexBuffer);
		}

		VertexArray::~VertexArray()
		{

		}

		void VertexArray::fill(VertexBuffer& p_VertexBuffer)
		{
			m_VertexBuffers.resize(1);
			m_AttributeDescriptions.resize(1);
			buffers.resize(1);
			m_BindingDescriptions.resize(1);

			m_VertexBuffers[0] = &p_VertexBuffer;

			for (auto& buffer : m_VertexBuffers)
			{
				m_AttributeDescriptions[0] = buffer->getAttributeDescriptions();
				buffers[0] = buffer->m_Buffer;
				m_BindingDescriptions[0] = buffer->getBindingDescription();
			}

			m_IsValid = true;
		}

		void VertexArray::fill(std::vector<VertexBuffer*>& p_VertexBuffers)
		{
			m_VertexBuffers = p_VertexBuffers;
			m_AttributeDescriptions.resize(m_VertexBuffers.size());
			buffers.resize(m_VertexBuffers.size());
			m_BindingDescriptions.resize(m_VertexBuffers.size());

			for (int i = 0; i < m_VertexBuffers.size(); i++)
			{
				m_AttributeDescriptions[i] = m_VertexBuffers[i]->getAttributeDescriptions();
				buffers[i] = m_VertexBuffers[i]->m_Buffer;
				m_BindingDescriptions[i] = m_VertexBuffers[i]->getBindingDescription();
			}

			m_IsValid = true;
		}

		void VertexArray::fill(const std::initializer_list<VertexBuffer*>& p_VertexBuffers)
		{
			m_VertexBuffers = std::vector<VertexBuffer*>(p_VertexBuffers.begin(), p_VertexBuffers.end());

			m_AttributeDescriptions.resize(m_VertexBuffers.size());
			buffers.resize(m_VertexBuffers.size());
			m_BindingDescriptions.resize(m_VertexBuffers.size());

			for (int i = 0; i < m_VertexBuffers.size(); i++)
			{
				m_AttributeDescriptions[i] = m_VertexBuffers[i]->getAttributeDescriptions();
				buffers[i] = m_VertexBuffers[i]->m_Buffer;
				m_BindingDescriptions[i] = m_VertexBuffers[i]->getBindingDescription();
			}

			m_IsValid = true;
		}

		void VertexArray::fill(std::vector<VertexBuffer*>& p_VertexBuffers, IndexBuffer& p_IndexBuffer)
		{
			indexBuffer = &p_IndexBuffer;

			m_VertexBuffers = p_VertexBuffers;

			m_AttributeDescriptions.resize(m_VertexBuffers.size());
			buffers.resize(m_VertexBuffers.size());
			m_BindingDescriptions.resize(m_VertexBuffers.size());

			for (int i = 0; i < m_VertexBuffers.size(); i++)
			{
				m_AttributeDescriptions[i] = m_VertexBuffers[i]->getAttributeDescriptions();
				buffers[i] = m_VertexBuffers[i]->m_Buffer;
				m_BindingDescriptions[i] = m_VertexBuffers[i]->getBindingDescription();
			}

			m_IsValid = true;
		}

		void VertexArray::fill(const std::initializer_list<VertexBuffer*>& p_VertexBuffers, IndexBuffer& p_IndexBuffer)
		{
			indexBuffer = &p_IndexBuffer;

			m_VertexBuffers = std::vector<VertexBuffer*>(p_VertexBuffers.begin(), p_VertexBuffers.end());

			m_AttributeDescriptions.resize(m_VertexBuffers.size());
			buffers.resize(m_VertexBuffers.size());
			m_BindingDescriptions.resize(m_VertexBuffers.size());

			for (int i = 0; i < m_VertexBuffers.size(); i++)
			{
				m_AttributeDescriptions[i] = m_VertexBuffers[i]->getAttributeDescriptions();
				buffers[i] = m_VertexBuffers[i]->m_Buffer;
				m_BindingDescriptions[i] = m_VertexBuffers[i]->getBindingDescription();
			}

			m_IsValid = true;
		}

		void VertexArray::update(const uint32_t p_Index)
		{
			buffers[p_Index] = m_VertexBuffers[p_Index]->m_Buffer;
		}

		void VertexArray::fill(VertexBuffer& p_VertexBuffer, IndexBuffer& p_IndexBuffer)
		{
			fill({ &p_VertexBuffer }, p_IndexBuffer);
		}

		std::vector<VkVertexInputBindingDescription> VertexArray::getBindingDescription()
		{
			return m_BindingDescriptions;
		}

		std::vector<std::vector<VkVertexInputAttributeDescription>> VertexArray::getAttributeDescriptions()
		{
			return m_AttributeDescriptions;
		}
	}
}
