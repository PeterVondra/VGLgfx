#pragma once

#include<iostream>
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>

#include <vector>

#include "VkContext.h"
#include "../../BufferLayout.h"

namespace vgl
{
	namespace vk
	{
		class IndexBuffer;
		class VertexBuffer;

		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		// VERTEX ARRAY
		//////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Container for both vertex buffers and index buffer
		class VertexArray
		{
		public:

			VertexArray();
			VertexArray(VertexBuffer& p_VertexBuffer);
			VertexArray(std::vector<VertexBuffer*>& p_VertexBuffers);
			VertexArray(const std::initializer_list<VertexBuffer*>& p_VertexBuffers);

			VertexArray(std::vector<VertexBuffer*>& p_VertexBuffers, IndexBuffer& p_IndexBuffer);
			VertexArray(const std::initializer_list<VertexBuffer*>& p_VertexBuffers, IndexBuffer& p_IndexBuffer);
			VertexArray(VertexBuffer& p_VertexBuffer, IndexBuffer& p_IndexBuffer);

			~VertexArray();

			//Fill the vertex array with buffers
			void fill(VertexBuffer& p_VertexBuffer);
			void fill(std::vector<VertexBuffer*>& p_VertexBuffers);
			void fill(const std::initializer_list<VertexBuffer*>& p_VertexBuffers);

			void fill(std::vector<VertexBuffer*>& p_VertexBuffers, IndexBuffer& p_IndexBuffer);
			void fill(const std::initializer_list<VertexBuffer*>& p_VertexBuffers, IndexBuffer& p_IndexBuffer);
			void fill(VertexBuffer& p_VertexBuffer, IndexBuffer& p_IndexBuffer);

			void update(const uint32_t p_Index);

			size_t getIndexCount() { return indexBuffer->m_Count; }
			
			bool isValid() { return m_IsValid; }
		protected:
		private:
			friend class Renderer;
			friend class BaseRenderer;
			friend class ForwardRenderer;
			friend class DeferredRenderer;
			friend class CommandBuffer;

			bool m_IsValid = false;

			std::vector<VkVertexInputBindingDescription>	m_BindingDescriptions;
			std::vector<std::vector<VkVertexInputAttributeDescription>>	m_AttributeDescriptions;

			//Binding descriptions for memory layout
			std::vector<VkVertexInputBindingDescription>	getBindingDescription();
			std::vector<std::vector<VkVertexInputAttributeDescription>>	getAttributeDescriptions();

			std::vector<VkBuffer> buffers;
			std::vector<VertexBuffer*> m_VertexBuffers;
			IndexBuffer* indexBuffer;
		};

		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		// INDEX BUFFER
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		class IndexBuffer
		{
		public:

			IndexBuffer();
			~IndexBuffer();

			void destroy();

			IndexBuffer(std::vector<uint32_t>& p_BufferData);
			IndexBuffer(std::vector<uint32_t>* p_BufferData);

			void fill(std::vector<uint32_t>& p_BufferData);
			void fill(std::vector<uint32_t>* p_BufferData);
			void fill(void* p_BufferData, const uint32_t p_Size);

			template<typename T> void update(std::vector<T>& p_BufferData);
			void update(void* p_BufferData, const uint32_t p_Size);

			void allocate(const uint32_t p_Size);
			void allocateHV(const uint32_t p_Size); // Host visible buffer
			void copy(void* p_BufferData, uint32_t p_Offset, const uint32_t p_Size);
			template<typename T> void copy(T* p_BufferData, uint32_t p_Offset, const uint32_t p_Size);
			void create();
			void flush();

			bool isValid() { return m_IsValid; }

		protected:
		private:
			friend class VertexArray;
			friend class Renderer;
			friend class BaseRenderer;
			friend class ForwardRenderer;
			friend class DeferredRenderer;
			friend class CommandBuffer;

			Context* m_ContextPtr;

			VkBuffer m_Buffer;
			VkBuffer m_StagingBuffer;
			AllocationInfo m_AllocInfo;
			VmaAllocation m_StagingBufferAlloc;

			VkDeviceSize m_BufferSize;
			void* m_Data;

			uint32_t m_Count;

			bool m_AlreadyDestroyed = false;
			bool m_IsValid = false;
		};

		template<typename T>
		inline void IndexBuffer::update(std::vector<T>& p_BufferData)
		{
			if (!p_BufferData.empty())
			{
				destroy();

				m_BufferSize = sizeof(p_BufferData[0]) * p_BufferData.size();

				auto alloc = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_StagingBuffer);

				auto data = m_ContextPtr->mapMemory(alloc);
				memcpy(data, p_BufferData.data(), m_BufferSize);
				m_ContextPtr->unmapMemory(alloc);

				m_AllocInfo = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);

				m_ContextPtr->copyBuffer(m_StagingBuffer, m_Buffer, m_BufferSize);

				m_ContextPtr->destroyBuffer(m_StagingBuffer, alloc);
			}
		}
		inline void IndexBuffer::update(void* p_BufferData, const uint32_t p_Size)
		{
			if (!p_BufferData)
			{
				destroy();

				m_BufferSize = p_Size;

				m_StagingBufferAlloc = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_StagingBuffer).p_Alloc;

				auto data = m_ContextPtr->mapMemory(m_StagingBufferAlloc);
				memcpy(data, p_BufferData, m_BufferSize);
				m_ContextPtr->unmapMemory(m_StagingBufferAlloc);

				m_AllocInfo = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);

				m_ContextPtr->copyBuffer(m_StagingBuffer, m_Buffer, m_BufferSize);

				m_ContextPtr->destroyBuffer(m_StagingBuffer, m_StagingBufferAlloc);
			}
		}

		inline void IndexBuffer::allocate(const uint32_t p_Size)
		{
			if (m_IsValid)
				destroy();
			m_BufferSize = p_Size;

			m_StagingBufferAlloc = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_StagingBuffer).p_Alloc;

			m_ContextPtr->mapMemory(m_StagingBufferAlloc);
		}
		inline void IndexBuffer::allocateHV(const uint32_t p_Size)
		{
			if (m_IsValid)
				destroy();
			m_BufferSize = p_Size;

			m_StagingBufferAlloc = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, m_StagingBuffer).p_Alloc;

			m_ContextPtr->mapMemory(m_StagingBufferAlloc);

			m_IsValid = true;
		}
		inline void IndexBuffer::copy(void* p_BufferData, uint32_t p_Offset, const uint32_t p_Size)
		{
			if (m_Data)
				memcpy((char*)m_Data + p_Offset, p_BufferData, p_Size);
		}
		template<typename T>inline void IndexBuffer::copy(T* p_BufferData, uint32_t p_Offset, const uint32_t p_Size)
		{
			if (m_Data)
				memcpy((T*)m_Data + p_Offset, p_BufferData, p_Size);
		}
		inline void IndexBuffer::flush()
		{
			//VkMappedMemoryRange mappedRange = {};
			//mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			//mappedRange.memory = m_IndexBufferMemory;
			//mappedRange.offset = 0;
			//mappedRange.size = VK_WHOLE_SIZE;
			//vkFlushMappedMemoryRanges(m_ContextPtr->m_Device, 1, &mappedRange);
			vmaFlushAllocation(m_ContextPtr->m_VmaAllocator, m_AllocInfo.p_Alloc, 0, VK_WHOLE_SIZE);
		}
		inline void IndexBuffer::create()
		{
			if (m_Data)
			{
				m_ContextPtr->unmapMemory(m_StagingBufferAlloc);

				m_AllocInfo = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);

				m_ContextPtr->copyBuffer(m_StagingBuffer, m_Buffer, m_BufferSize);

				m_ContextPtr->destroyBuffer(m_StagingBuffer, m_StagingBufferAlloc);

				m_IsValid = true;

				flush();
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		// VERTEX BUFFER
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		class VertexBuffer
		{
		public:

			//Initializing the m_ContextPtr to the global vk context instance
			VertexBuffer();

			VertexBuffer(InputRate p_InputRate);
			VertexBuffer(BufferLayout* p_Layout);
			VertexBuffer(BufferLayout& p_Layout);
			VertexBuffer(BufferLayout* p_Layout, InputRate p_InputRate);
			VertexBuffer(BufferLayout& p_Layout, InputRate p_InputRate);

			template<typename T> VertexBuffer(BufferLayout* p_Layout, std::vector<T>& p_BufferData);
			template<typename T> VertexBuffer(BufferLayout& p_Layout, std::vector<T>& p_BufferData);

			template<typename T> VertexBuffer(std::vector<T>& p_BufferData);

			// Updates buffer with new data
			template<typename T> void update(std::vector<T>* p_BufferData);
			template<typename T> void update(std::vector<T>& p_BufferData);

			// Allocate space for staging buffer
			void allocate(const uint32_t p_Size);
			void allocateHV(const uint32_t p_Size);
			// Copy data to staging buffer
			void copy(void* p_BufferData, uint32_t p_Offset, const uint32_t p_Size);
			template<typename T> void copy(T* p_BufferData, uint32_t p_Offset, const uint32_t p_Size);
			// Copy staging buffers data to vertex buffers data (destroys staging buffer after)
			void create();
			void flush(); // If using Host visible buffer

			~VertexBuffer();

			// Destroys the buffer and its memory
			void destroy();

			//Set the layout for proper memory alignment
			void setLayout(BufferLayout* p_Layout);
			void setLayout(BufferLayout& p_Layout);

			static std::pair<std::vector<VkVertexInputAttributeDescription>, VkVertexInputBindingDescription> getAttributes(BufferLayout* p_Layout);
			static std::pair<std::vector<VkVertexInputAttributeDescription>, VkVertexInputBindingDescription> getAttributes(BufferLayout* p_Layout, InputRate p_InputRate);

			template<typename T> void fill(std::vector<T>& p_BufferData);

			bool operator==(VertexBuffer& p_VertexBuffer)
			{
				return m_Buffer == p_VertexBuffer.m_Buffer && m_Data == p_VertexBuffer.m_Data;
			}

			bool isValid() { return isValid; }

		protected:
		private:
			friend class VertexArray;
			friend class Renderer;
			friend class BaseRenderer;
			friend class ForwardRenderer;
			friend class DeferredRenderer;
			friend class CommandBuffer;

			InputRate m_InputRate;

			//For vertex attributes, vulkan uses it's own format.
			//converts Generic ShaderDataType to vulkan VkFormat type
			static VkFormat toVulkanDataType(ShaderDataType p_Type);

			//Fill buffer with a staging buffer
			template<typename T> void fillBuffer(std::vector<T>& p_BufferData);
			template<typename T> void fillBuffer(std::vector<T>* p_BufferData);
			template<typename T> void allocateBuffer(size_t p_Size);

			//Vulkan specific buffer
			void* m_Data; // Location of the mapped data in the staging buffer
			VkBuffer		m_Buffer;
			AllocationInfo	m_AllocInfo;
			VkDeviceSize	m_BufferSize = 0;
			VkBuffer		m_StagingBuffer;
			VmaAllocation	m_StagingBufferAlloc;

			// Set vertex buffer attributes
			void setAttribute(const uint32_t p_Binding, const uint32_t p_Location, const VkFormat p_Format, const uint32_t p_Offset);
			void setAttributes();

			static VkVertexInputAttributeDescription getAttribute(const uint32_t p_Binding, const uint32_t p_Location, const VkFormat p_Format, const uint32_t p_Offset);

			//Attribute descriptions for vertex buffer layout
			std::vector<VkVertexInputAttributeDescription>	m_AttributeDescriptions;
			VkVertexInputBindingDescription					m_BindingDescription;

			//Binding descriptions for memory layout
			VkVertexInputBindingDescription					getBindingDescription();
			std::vector<VkVertexInputAttributeDescription>	getAttributeDescriptions();

			//Pointer to global vk context instance 
			Context* m_ContextPtr;

			//Memory Layout
			BufferLayout* m_Layout;

			bool m_IsValid = false;
		};

		inline VertexBuffer::VertexBuffer()
		{
			m_ContextPtr = &ContextSingleton::getInstance();
			m_InputRate = InputRate::Vertex;
		}
		inline VertexBuffer::VertexBuffer(InputRate p_InputRate)
		{
			m_ContextPtr = &ContextSingleton::getInstance();
			m_InputRate = p_InputRate;
		}

		template<typename T>
		inline VertexBuffer::VertexBuffer(BufferLayout* p_Layout, std::vector<T>& p_BufferData)
		{
			m_ContextPtr = &vk::ContextSingleton::getInstance();
			m_Layout = p_Layout;
			setAttributes();
			fillBuffer(p_BufferData);
		}
		template<typename T>
		inline VertexBuffer::VertexBuffer(BufferLayout& p_Layout, std::vector<T>& p_BufferData)
		{
			m_ContextPtr = &vk::ContextSingleton::getInstance();
			m_Layout = &p_Layout;
			setAttributes();
			fillBuffer(p_BufferData);
		}
		template<typename T>
		inline VertexBuffer::VertexBuffer(std::vector<T>& p_BufferData)
		{
			m_ContextPtr = &vk::ContextSingleton::getInstance();
			fillBuffer(p_BufferData);
		}
		inline VertexBuffer::VertexBuffer(BufferLayout* p_Layout)
		{
			m_ContextPtr = &vk::ContextSingleton::getInstance();
			m_Layout = p_Layout;
			m_InputRate = InputRate::Vertex;
			setAttributes();
		}
		inline VertexBuffer::VertexBuffer(BufferLayout& p_Layout)
		{
			m_ContextPtr = &vk::ContextSingleton::getInstance();
			m_Layout = &p_Layout;
			m_InputRate = InputRate::Vertex;
			setAttributes();
		}
		inline VertexBuffer::VertexBuffer(BufferLayout* p_Layout, InputRate p_InputRate)
		{
			m_ContextPtr = &vk::ContextSingleton::getInstance();
			m_Layout = p_Layout;
			m_InputRate = p_InputRate;
			setAttributes();
		}
		inline VertexBuffer::VertexBuffer(BufferLayout& p_Layout, InputRate p_InputRate)
		{
			m_ContextPtr = &vk::ContextSingleton::getInstance();
			m_Layout = &p_Layout;
			m_InputRate = p_InputRate;
			setAttributes();
		}
		template<typename T>
		inline void VertexBuffer::update(std::vector<T>* p_BufferData)
		{
			if (!p_BufferData)
				return;
			if (!p_BufferData->empty())
			{
				destroy();

				//size of bytes to allocate into the buffer
				m_BufferSize = sizeof(p_BufferData->operator[](0)) * p_BufferData-size();

				m_StagingBufferAlloc = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_StagingBuffer).p_Alloc;

				// Copy data to a staging buffer
				m_Data = m_ContextPtr->mapMemory(m_StagingBufferAlloc);
				memcpy(m_Data, p_BufferData->data(), m_BufferSize);
				m_ContextPtr->unmapMemory(m_StagingBufferAlloc);

				m_AllocInfo = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);

				// Copy content of staging buffer(CPU) to vertex buffer(GPU)
				m_ContextPtr->copyBuffer(m_StagingBuffer, m_Buffer, m_BufferSize);

				m_ContextPtr->destroyBuffer(m_StagingBuffer, m_StagingBufferAlloc);

				flush();
			}
		}
		template<typename T>
		inline void VertexBuffer::update(std::vector<T>& p_BufferData)
		{
			fill(&p_BufferData);
		}

		inline void VertexBuffer::allocate(const uint32_t p_Size)
		{
			if (isValid)
				destroy();
			m_BufferSize = p_Size;

			m_StagingBufferAlloc = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_StagingBuffer).p_Alloc;

			m_Data = m_ContextPtr->mapMemory(m_StagingBufferAlloc);
		}
		inline void VertexBuffer::allocateHV(const uint32_t p_Size)
		{
			if (isValid)
				destroy();
			m_BufferSize = p_Size;

			m_AllocInfo = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, m_Buffer);

			m_Data = m_ContextPtr->mapMemory(m_AllocInfo.p_Alloc);

			m_IsValid = true;
		}
		inline void VertexBuffer::copy(void* p_BufferData, uint32_t p_Offset, const uint32_t p_Size)
		{
			if (m_Data)
				memcpy((char*)m_Data + p_Offset, p_BufferData, p_Size);
		}
		template<typename T>inline void VertexBuffer::copy(T* p_BufferData, uint32_t p_Offset, const uint32_t p_Size)
		{
			if (m_Data)
				memcpy((T*)m_Data + p_Offset, p_BufferData, p_Size);
		}
		inline void VertexBuffer::flush()
		{
			vmaFlushAllocation(m_ContextPtr->m_VmaAllocator, m_AllocInfo.p_Alloc, 0, VK_WHOLE_SIZE);
		}
		inline void VertexBuffer::create()
		{
			if (m_Data){
				m_ContextPtr->unmapMemory(m_StagingBufferAlloc);

				m_AllocInfo = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);

				m_ContextPtr->copyBuffer(m_StagingBuffer, m_Buffer, m_BufferSize);

				m_ContextPtr->destroyBuffer(m_StagingBuffer, m_StagingBufferAlloc);

				m_IsValid = true;

				flush();
			}
		}

		inline VertexBuffer::~VertexBuffer()
		{
			if (!m_IsValid)
			{
				//vkDeviceWaitIdle(m_ContextPtr->device);
				//if (m_VertexBuffer != VK_NULL_HANDLE)
				//	vkDestroyBuffer(m_ContextPtr->device, m_VertexBuffer, nullptr);
				//if (m_VertexBufferMemory != VK_NULL_HANDLE)
				//	vkFreeMemory(m_ContextPtr->device, m_VertexBufferMemory, nullptr);
			}
		}

		inline void VertexBuffer::destroy()
		{
			if (!m_IsValid)
			{
				m_AttributeDescriptions.clear();
				m_Data = nullptr;

				vkDeviceWaitIdle(m_ContextPtr->m_Device);
				if (m_Buffer != VK_NULL_HANDLE) {
					m_ContextPtr->destroyBuffer(m_Buffer, m_AllocInfo.p_Alloc);
#ifdef USE_LOGGING
					Utils::Logger::logMSG("Destroyed Vertex Buffer", "VkVertexBuffer", Utils::Severity::Trace);
#endif
				}
				m_IsValid = false;
			}
		}

		inline void VertexBuffer::setLayout(BufferLayout* p_Layout)
		{
			m_Layout = p_Layout;
			setAttributes();
		}
		inline void VertexBuffer::setLayout(BufferLayout& p_Layout)
		{
			m_Layout = &p_Layout;
			setAttributes();
		}

		template<typename T>
		inline void VertexBuffer::fill(std::vector<T>& p_BufferData)
		{
			fillBuffer(p_BufferData);
		}

		inline VkVertexInputBindingDescription VertexBuffer::getBindingDescription()
		{
			return m_BindingDescription;
		}

		inline std::vector<VkVertexInputAttributeDescription> VertexBuffer::getAttributeDescriptions()
		{
			return m_AttributeDescriptions;
		}

		inline void VertexBuffer::setAttribute(const uint32_t p_Binding, const uint32_t p_Location, const VkFormat p_Format, const uint32_t p_Offset)
		{
			VkVertexInputAttributeDescription attributeDescription = {};
			m_AttributeDescriptions.push_back(attributeDescription);

			m_AttributeDescriptions[m_AttributeDescriptions.size() - 1].binding = p_Binding;
			m_AttributeDescriptions[m_AttributeDescriptions.size() - 1].location = p_Location;
			m_AttributeDescriptions[m_AttributeDescriptions.size() - 1].format = p_Format;
			m_AttributeDescriptions[m_AttributeDescriptions.size() - 1].offset = p_Offset;
		}

		inline void VertexBuffer::setAttributes()
		{
			m_BindingDescription.binding = m_Layout->getBindingIndex();
			m_BindingDescription.stride = m_Layout->getStride();
			m_BindingDescription.inputRate = (VkVertexInputRate)m_InputRate;

			int stride = 0;
			for (auto& element : m_Layout->getElements())
			{
				if (element.getShaderDataType() == ShaderDataType::Mat4f)
				{
					for (int i = 0; i < 4; i++)
					{
						setAttribute(
							m_Layout->getBindingIndex(),
							element.getLocation() + i,
							toVulkanDataType(element.getShaderDataType()),
							stride * sizeof(float)
						);

						stride += 4 * sizeof(float);
					}
				}
				else
				{
					setAttribute(
						m_Layout->getBindingIndex(),
						element.getLocation(),
						toVulkanDataType(element.getShaderDataType()),
						element.getOffset()
					);
					stride += element.getShaderDataTypeSize(element.getShaderDataType());
				}
			}
		}

		inline VkVertexInputAttributeDescription VertexBuffer::getAttribute(const uint32_t p_Binding, const uint32_t p_Location, const VkFormat p_Format, const uint32_t p_Offset)
		{
			VkVertexInputAttributeDescription attributeDescription = {};

			attributeDescription.binding = p_Binding;
			attributeDescription.location = p_Location;
			attributeDescription.format = p_Format;
			attributeDescription.offset = p_Offset;

			return attributeDescription;
		}

		inline std::pair<std::vector<VkVertexInputAttributeDescription>, VkVertexInputBindingDescription> VertexBuffer::getAttributes(BufferLayout* p_Layout)
		{
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = p_Layout->getBindingIndex();
			bindingDescription.stride = p_Layout->getStride();
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			for (auto& element : p_Layout->getElements())
			{
				attributeDescriptions.push_back(getAttribute(
					p_Layout->getBindingIndex(),
					element.getLocation(),
					toVulkanDataType(element.getShaderDataType()),
					element.getOffset()
				));
			}

			return std::pair<std::vector<VkVertexInputAttributeDescription>, VkVertexInputBindingDescription>(attributeDescriptions, bindingDescription);
		}
		inline std::pair<std::vector<VkVertexInputAttributeDescription>, VkVertexInputBindingDescription> VertexBuffer::getAttributes(BufferLayout* p_Layout, InputRate p_InputRate)
		{
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = p_Layout->getBindingIndex();
			bindingDescription.stride = p_Layout->getStride();
			bindingDescription.inputRate = (VkVertexInputRate)p_InputRate;


			int stride = 0;
			for (auto& element : p_Layout->getElements())
			{
				if (element.getShaderDataType() == ShaderDataType::Mat4f)
				{
					for (int i = 0; i < 4; i++)
					{
						attributeDescriptions.push_back(getAttribute(
							p_Layout->getBindingIndex(),
							element.getLocation() + i,
							toVulkanDataType(element.getShaderDataType()),
							stride
						));
						stride += 4 * sizeof(float);
					}
				}
				else
				{
					attributeDescriptions.push_back(getAttribute(
						p_Layout->getBindingIndex(),
						element.getLocation(),
						toVulkanDataType(element.getShaderDataType()),
						element.getOffset()
					));
					stride += element.getShaderDataTypeSize(element.getShaderDataType());
				}
			}

			return std::pair<std::vector<VkVertexInputAttributeDescription>, VkVertexInputBindingDescription>(attributeDescriptions, bindingDescription);
		}

		inline VkFormat VertexBuffer::toVulkanDataType(ShaderDataType p_Type)
		{
			switch (p_Type)
			{
			case ShaderDataType::Float:		return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Vec2f:		return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Vec3f:		return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Vec4f:		return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderDataType::ImVec2f:	return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::ImVec4f:	return VK_FORMAT_R8G8B8A8_UNORM;
			case ShaderDataType::Int:		return VK_FORMAT_R32_SINT;
			case ShaderDataType::Vec2i:		return VK_FORMAT_R32G32_SINT;
			case ShaderDataType::Vec3i:		return VK_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Vec4i:		return VK_FORMAT_R32G32B32A32_SINT;
			case ShaderDataType::Mat3f:		return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Mat4f:		return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderDataType::Bool:		Utils::Logger::logMSG("Bool not supported yet", "VertexBuffer Data Type", Utils::Severity::Error);
			}
			Utils::Logger::logMSG("Unknown type", "Vk Data Type", Utils::Severity::Error);
		}

		template<typename T>
		inline void VertexBuffer::fillBuffer(std::vector<T>& p_BufferData)
		{
			fillBuffer(&p_BufferData);
		}

		template<typename T>
		inline void VertexBuffer::fillBuffer(std::vector<T>* p_BufferData)
		{
			if (!p_BufferData)
				return;
			if (!p_BufferData->empty())
			{
				destroy();

				//size of bytes to allocate into the buffer
				m_BufferSize = sizeof(p_BufferData->operator[](0)) * p_BufferData - size();

				m_StagingBufferAlloc = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_StagingBuffer).p_Alloc;

				// Copy data to a staging buffer
				m_Data = m_ContextPtr->mapMemory(m_StagingBufferAlloc);
				memcpy(m_Data, p_BufferData->data(), m_BufferSize);
				m_ContextPtr->unmapMemory(m_StagingBufferAlloc);

				m_AllocInfo = m_ContextPtr->createBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);

				// Copy content of staging buffer(CPU) to vertex buffer(GPU)
				m_ContextPtr->copyBuffer(m_StagingBuffer, m_Buffer, m_BufferSize);

				m_ContextPtr->destroyBuffer(m_StagingBuffer, m_StagingBufferAlloc);

				flush();
			}
		}
	}
}