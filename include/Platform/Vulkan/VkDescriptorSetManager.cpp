#include "VkDescriptorSetManager.h"

namespace vgl
{
	namespace vk
	{
		bool DescriptorSetInfo::isImageDescriptorsValid()
		{
			bool valid = false;
			for (auto& sampler : p_ImageDescriptors)
				if(sampler.p_Image)
					valid = sampler.isValid() && sampler.p_Image->isValid();
			return valid;
		}
		
		bool DescriptorSetInfo::isStorageImage3dDescriptorsValid()
		{
			bool valid = false;
			//for (auto& sampler : p_StorageImage3dDescriptors)
			//	if(sampler.p_Image)
			//		valid = sampler.isValid() && sampler.p_Image->isValid();
			return valid;
		}

		bool DescriptorSetInfo::isImageArrayDescriptorsValid()
		{
			bool valid = false;
			for (int i = 0; i < p_ImageArrayDescriptors.size(); i++) {
				if (p_ImageArrayDescriptors[i].isValid()) {
					for (int j = 0; j < p_ImageArrayDescriptors[i].p_Image->size(); j++) {
						valid = p_ImageArrayDescriptors[i].p_Image->operator[](j).isValid();
						if (!valid) {
#ifdef USE_LOGGING
							Utils::Logger::logMSG(
								"Image[" + std::to_string(j) + "] in image array index " + std::to_string(i) + "is not valid/complete",
								"DescriptorSetManager",
								Utils::Severity::Error
							);
#endif
							return false;
						}
					}
				}
				else
					return false;
			}
			return valid;
		}

		bool DescriptorSetInfo::isCubeMapDescriptorsValid()
		{
			bool valid = false;
			for (auto& sampler : p_CubeMapDescriptors)
				valid = sampler.isValid() && sampler.p_Image->isValid();
			return valid;
		}

		DescriptorSetManager::DescriptorSetManager() : m_ContextPtr(&ContextSingleton::getInstance())
		{

		}
		DescriptorSetManager::~DescriptorSetManager()
		{

		}
		void DescriptorSetManager::copy(ShaderStage p_ShaderStage, void* p_Data, uint64_t p_Size, uint64_t p_Offset)
		{
			if (!m_DescriptorSetInfo->p_UniformBuffers[(int)p_ShaderStage].m_MappedData.empty())
			{
				void* mapped = (char*)(m_DescriptorSetInfo->p_UniformBuffers[(int)p_ShaderStage].m_MappedData[m_ContextPtr->m_ImageIndex]) + p_Offset;
				memcpy(mapped, p_Data, p_Size);
			}
		}
		void DescriptorSetManager::copyToStorageBuffer(void* p_Data, uint64_t p_Size, uint64_t p_Offset)
		{
			if (!m_DescriptorSetInfo)
				return;
			for(int i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
				if (!m_DescriptorSetInfo->p_StorageBuffer.m_MappedData.empty())
				{
					void* mapped = (char*)(m_DescriptorSetInfo->p_StorageBuffer.m_MappedData[i]) + p_Offset;
					memcpy(mapped, p_Data, p_Size);
				}
		}
		void DescriptorSetManager::update()
		{

		}
		void DescriptorSetManager::update(DescriptorSetInfo& p_Info)
		{

		}
		void DescriptorSetManager::create(DescriptorSetInfo& p_Info)
		{
			m_DescriptorSetInfo = &p_Info;

			if (!p_Info.isValid()) {
#ifdef USE_LOGGING
				Utils::Logger::logMSG("In function DescriptorSetManager::create(...): DescriptorSetInfo is not valid", "DescriptorSetManager", Utils::Severity::Error);
#endif
				return;
			}

			m_LayoutBindings.resize(m_ContextPtr->m_SwapchainImageCount);
			m_ImageDescriptorInfo.resize(m_ContextPtr->m_SwapchainImageCount);

			// s = swapchain image index
			for (uint32_t s = 0; s < m_ContextPtr->m_SwapchainImageCount; s++) {
				uint32_t imgDescriptorCount = 0;
				int32_t baseIndex = 0;

				// Normal Image and Cubemaps have same descriptor info, so they are treated the same
				if (m_DescriptorSetInfo->p_IsImageDescriptorsValid) {
					// Find all duplicated bindings (but with different frameindex)
					int32_t dec = 0;
					for (auto& imgDsc : m_DescriptorSetInfo->p_ImageDescriptors) {
						if (imgDsc.p_FrameIndex >= 0)
							dec++;
					}
					imgDescriptorCount += m_DescriptorSetInfo->p_ImageDescriptors.size() - dec/2; // Descriptor count for each frameindex, only half of duplicated bindings will be removed

					// Remove duplicated bindings (but with different frameindex)
					m_ImageDescriptorInfo[s].resize(m_DescriptorSetInfo->p_ImageDescriptors.size()-dec/2);

					int32_t ig = 0;
					for (int32_t i = 0; i < m_DescriptorSetInfo->p_ImageDescriptors.size(); i++) {
						// Image info for descriptor update, check if frameindex match with swapchain image index
						if (m_DescriptorSetInfo->p_ImageDescriptors[i].p_FrameIndex == s || m_DescriptorSetInfo->p_ImageDescriptors[i].p_FrameIndex < 0) {
							m_ImageDescriptorInfo[s][i-ig].imageLayout = m_DescriptorSetInfo->p_ImageDescriptors[i].p_ImageLayout;
							m_ImageDescriptorInfo[s][i-ig].imageView = m_DescriptorSetInfo->p_ImageDescriptors[i].p_Image->m_ImageView;
							m_ImageDescriptorInfo[s][i-ig].sampler = m_DescriptorSetInfo->p_ImageDescriptors[i].p_Image->m_Sampler;

							// s = 0, i = 0, k = 1, ig = 1
							// s = 0, i = 1, k = 0, ig = 1

							// s = 1, i = 0, k = 1, ig = 1
							// s = 1, i = 1, k = 0, ig = 2

							// Describe what type of desriptor will be used, and which stage
							VkDescriptorSetLayoutBinding layoutBinding;
							layoutBinding.binding = m_DescriptorSetInfo->p_ImageDescriptors[i].p_Binding;
							layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
							layoutBinding.descriptorCount = 1;
							layoutBinding.stageFlags = getShaderStageVkH(m_DescriptorSetInfo->p_ImageDescriptors[i].p_ShaderStage);
							layoutBinding.pImmutableSamplers = nullptr;

							if(s == 0) // Avoid duplicated bindings
								m_ImageDescriptorBindings.push_back(layoutBinding.binding);

							m_LayoutBindings[s].push_back(layoutBinding);
							continue;
						}
						ig++; // Duplicated binding was found (but with different frameindex)
					}

					// If image descriptors also included, increase baseIndex of m_ImageDescriptorInfo[s]
					baseIndex = m_DescriptorSetInfo->p_ImageDescriptors.size() - (dec / 2);
				}

				/*if (m_DescriptorSetInfo->p_IsStorageImage3dDescriptorsValid) {
					// Find all duplicated bindings (but with different frameindex)
					int32_t dec = 0;
					for (auto& imgDsc : m_DescriptorSetInfo->p_StorageImage3dDescriptors) {
						if (imgDsc.p_FrameIndex >= 0)
							dec++;
					}
					imgDescriptorCount += m_DescriptorSetInfo->p_StorageImage3dDescriptors.size() - dec / 2; // Descriptor count for each frameindex, only half of duplicated bindings will be removed

					if (baseIndex < 0)
						baseIndex = 0;

					// Remove duplicated bindings (but with different frameindex)
					if (m_DescriptorSetInfo->p_IsImageDescriptorsValid)
						m_ImageDescriptorInfo[s].resize(baseIndex + m_DescriptorSetInfo->p_StorageImage3dDescriptors.size() - dec / 2);
					else
						m_ImageDescriptorInfo[s].resize(m_DescriptorSetInfo->p_StorageImage3dDescriptors.size() - dec / 2);

					uint32_t ig = 0;
					for (uint32_t i = 0; i < m_DescriptorSetInfo->p_StorageImage3dDescriptors.size(); i++) {
						// Image/CubeMap info for descriptor update, check if frameindex match with swapchain image index
						if (m_DescriptorSetInfo->p_CubeMapDescriptors[i].p_FrameIndex == s || m_DescriptorSetInfo->p_StorageImage3dDescriptors[i].p_FrameIndex < 0) {
							m_ImageDescriptorInfo[s][baseIndex + i-ig].imageLayout = m_DescriptorSetInfo->p_StorageImage3dDescriptors[i].p_ImageLayout;
							m_ImageDescriptorInfo[s][baseIndex + i-ig].imageView = m_DescriptorSetInfo->p_StorageImage3dDescriptors[i].p_Image->m_ImageView;
							m_ImageDescriptorInfo[s][baseIndex + i-ig].sampler = m_DescriptorSetInfo->p_StorageImage3dDescriptors[i].p_Image->m_ImageSampler;

							// Describe what type of desriptor will be used, and which stage
							VkDescriptorSetLayoutBinding layoutBinding;
							layoutBinding.binding = m_DescriptorSetInfo->p_CubeMapDescriptors[i].p_Binding;
							layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
							layoutBinding.descriptorCount = 1;
							layoutBinding.stageFlags = getShaderStageVkH(m_DescriptorSetInfo->p_CubeMapDescriptors[i].p_ShaderStage);
							layoutBinding.pImmutableSamplers = nullptr;

							if(s == 0) // Avoid duplicated bindings
								m_ImageDescriptorBindings.push_back(layoutBinding.binding);

							m_LayoutBindings[s].push_back(layoutBinding);
							continue;
						}
						ig++; // Duplicated binding was found (but with different frameindex)
					}
				}*/
				
				if (m_DescriptorSetInfo->p_IsCubeMapDescriptorsValid) {
					// Find all duplicated bindings (but with different frameindex)
					int32_t dec = 0;
					for (auto& imgDsc : m_DescriptorSetInfo->p_CubeMapDescriptors) {
						if (imgDsc.p_FrameIndex >= 0)
							dec++;
					}
					imgDescriptorCount += m_DescriptorSetInfo->p_CubeMapDescriptors.size() - dec / 2; // Descriptor count for each frameindex, only half of duplicated bindings will be removed

					if (baseIndex < 0)
						baseIndex = 0;

					// Remove duplicated bindings (but with different frameindex)
					if (m_DescriptorSetInfo->p_IsImageDescriptorsValid)
						m_ImageDescriptorInfo[s].resize(baseIndex + m_DescriptorSetInfo->p_CubeMapDescriptors.size() - dec / 2);
					else
						m_ImageDescriptorInfo[s].resize(m_DescriptorSetInfo->p_CubeMapDescriptors.size() - dec / 2);

					uint32_t ig = 0;
					for (uint32_t i = 0; i < m_DescriptorSetInfo->p_CubeMapDescriptors.size(); i++) {
						// Image/CubeMap info for descriptor update, check if frameindex match with swapchain image index
						if (m_DescriptorSetInfo->p_CubeMapDescriptors[i].p_FrameIndex == s || m_DescriptorSetInfo->p_CubeMapDescriptors[i].p_FrameIndex < 0) {
							m_ImageDescriptorInfo[s][baseIndex + i-ig].imageLayout = m_DescriptorSetInfo->p_CubeMapDescriptors[i].p_ImageLayout;
							m_ImageDescriptorInfo[s][baseIndex + i-ig].imageView = m_DescriptorSetInfo->p_CubeMapDescriptors[i].p_Image->m_ImageView;
							m_ImageDescriptorInfo[s][baseIndex + i-ig].sampler = m_DescriptorSetInfo->p_CubeMapDescriptors[i].p_Image->m_Sampler;

							// Describe what type of desriptor will be used, and which stage
							VkDescriptorSetLayoutBinding layoutBinding;
							layoutBinding.binding = m_DescriptorSetInfo->p_CubeMapDescriptors[i].p_Binding;
							layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
							layoutBinding.descriptorCount = 1;
							layoutBinding.stageFlags = getShaderStageVkH(m_DescriptorSetInfo->p_CubeMapDescriptors[i].p_ShaderStage);
							layoutBinding.pImmutableSamplers = nullptr;

							if(s == 0) // Avoid duplicated bindings
								m_ImageDescriptorBindings.push_back(layoutBinding.binding);

							m_LayoutBindings[s].push_back(layoutBinding);
							continue;
						}
						ig++; // Duplicated binding was found (but with different frameindex)
					}
				}
				if (m_DescriptorSetInfo->p_IsImageArrayDescriptorsValid) {
					for (auto& imgArray : m_DescriptorSetInfo->p_ImageArrayDescriptors) {
						imgArray.m_ImageArrayDescriptorInfo.resize(imgArray.p_ImageInfo.size());

						// Image info for descriptor update
						for (int i = 0; i < imgArray.p_ImageInfo.size(); i++) {
							imgArray.m_ImageArrayDescriptorInfo[i].imageLayout = imgArray.p_ImageLayout;
							imgArray.m_ImageArrayDescriptorInfo[i].imageView = imgArray.p_ImageInfo[i].first;
							imgArray.m_ImageArrayDescriptorInfo[i].sampler = imgArray.p_ImageInfo[i].second;
						}

						imgDescriptorCount += imgArray.p_ImageInfo.size();

						// Describe what type of desriptor will be used, and which stage
						VkDescriptorSetLayoutBinding layoutBinding;
						layoutBinding.binding = imgArray.p_Binding;
						layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						layoutBinding.descriptorCount = imgArray.p_ImageInfo.size();
						layoutBinding.stageFlags = getShaderStageVkH(imgArray.p_ShaderStage);
						layoutBinding.pImmutableSamplers = nullptr;

						m_ImageDescriptorBindings.push_back(layoutBinding.binding);

						m_LayoutBindings[s].push_back(layoutBinding);
					}
				}
			}

			m_UniformBufferCount = 0;

			if (m_DescriptorSetInfo->p_StorageBuffer.isValid()) {
				if (!m_DescriptorSetInfo->p_StorageBuffer.m_IsValid)
					m_DescriptorSetInfo->p_StorageBuffer.createBuffer();

				// Describe what type of desriptor will be used, and which stage
				VkDescriptorSetLayoutBinding layoutBinding;
				layoutBinding.binding = m_DescriptorSetInfo->p_StorageBuffer.m_Binding; // Must be this binding in shader
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = getShaderStageVkH(m_DescriptorSetInfo->p_StorageBuffer.m_ShaderStage);
				layoutBinding.pImmutableSamplers = nullptr;
				
				for(int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
					m_LayoutBindings[i].push_back(layoutBinding);
			}
			if (m_DescriptorSetInfo->p_VertexUniformBuffer.isValid()) {
				if (!m_DescriptorSetInfo->p_VertexUniformBuffer.m_IsValid)
					m_DescriptorSetInfo->p_VertexUniformBuffer.createBuffer();

				// Describe what type of desriptor will be used, and which stage
				VkDescriptorSetLayoutBinding layoutBinding;
				layoutBinding.binding = m_DescriptorSetInfo->p_VertexUniformBuffer.m_Binding; // Must be this binding in shader
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				layoutBinding.pImmutableSamplers = nullptr;

				for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
					m_LayoutBindings[i].push_back(layoutBinding);

				m_UniformBufferCount++;
			}
			if (m_DescriptorSetInfo->p_FragmentUniformBuffer.isValid()) {
				if (!m_DescriptorSetInfo->p_FragmentUniformBuffer.m_IsValid)
					m_DescriptorSetInfo->p_FragmentUniformBuffer.createBuffer();

				// Describe what type of desriptor will be used, and which stage
				VkDescriptorSetLayoutBinding layoutBinding;
				layoutBinding.binding = m_DescriptorSetInfo->p_FragmentUniformBuffer.m_Binding; // Must be this binding in shader
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				layoutBinding.pImmutableSamplers = nullptr;

				for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
					m_LayoutBindings[i].push_back(layoutBinding);

				m_UniformBufferCount++;
			}
			if (m_DescriptorSetInfo->p_GeometryUniformBuffer.isValid()) {
				if(!m_DescriptorSetInfo->p_GeometryUniformBuffer.m_IsValid)
					m_DescriptorSetInfo->p_GeometryUniformBuffer.createBuffer();

				// Describe what type of desriptor will be used, and which stage
				VkDescriptorSetLayoutBinding layoutBinding;
				layoutBinding.binding = m_DescriptorSetInfo->p_GeometryUniformBuffer.m_Binding; // Must be this binding in shader
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;
				layoutBinding.pImmutableSamplers = nullptr;

				for (int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
					m_LayoutBindings[i].push_back(layoutBinding);

				m_UniformBufferCount++;
			}

			//VkDescriptorPoolSize poolSize = {};
			//poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			//poolSize.descriptorCount = static_cast<uint32_t>(m_ContextPtr->m_SwapchainImageCount * m_UniformBufferCount);
			//m_PoolSize.push_back(poolSize);

			uint32_t imgDescriptorCount = 0;

			//poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			//poolSize.descriptorCount = static_cast<uint32_t>(m_ContextPtr->m_SwapchainImageCount * imgDescriptorCount);
			//m_PoolSize.push_back(poolSize);

			createDescriptorSetLayout();
			//createDescriptorPool();
			createDescriptorSets();

			m_IsValid = true;
		}
		void DescriptorSetManager::destroy()
		{
			// Must have been created to be destroyed
			if (m_IsValid) {
				vkDeviceWaitIdle(m_ContextPtr->m_Device);

				if (m_DescriptorSetInfo->p_VertexUniformBuffer.isValid()) {
					m_DescriptorSetInfo->p_VertexUniformBuffer.destroy();
#ifdef USE_LOGGING
					Utils::Logger::logMSG("Destroyed vertex stage uniform buffer\n", "DescriptorSetManager", Utils::Severity::Trace);
#endif
				}
				if (m_DescriptorSetInfo->p_FragmentUniformBuffer.isValid()) {
					m_DescriptorSetInfo->p_VertexUniformBuffer.destroy();
#ifdef USE_LOGGING
					Utils::Logger::logMSG("Destroyed vertex stage uniform buffer\n", "DescriptorSetManager", Utils::Severity::Trace);
#endif
				}
				if (m_DescriptorSetInfo->p_GeometryUniformBuffer.isValid()) {
					m_DescriptorSetInfo->p_VertexUniformBuffer.destroy();
#ifdef USE_LOGGING
					Utils::Logger::logMSG("Destroyed vertex stage uniform buffer\n", "DescriptorSetManager", Utils::Severity::Trace);
#endif
				}

				//vkDestroyDescriptorPool(m_ContextPtr->m_Device, m_DescriptorPool, nullptr);
				//vkDestroyDescriptorSetLayout(m_ContextPtr->m_Device, m_DescriptorSetLayout, nullptr);

				for(int32_t i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++)
					vkFreeDescriptorSets(m_ContextPtr->m_Device, m_DescriptorSets[i].m_CurrentDescriptorPool, 1, &m_DescriptorSets[i].m_DescriptorSet);

				m_LayoutBindings.clear();

				m_ImageDescriptorInfo.clear();
				m_ImageDescriptorBindings.clear();

				m_IsValid = false;
			}
		}
		bool DescriptorSetManager::isValid()
		{
			return m_IsValid;
		}
/*		void DescriptorSetManager::createDescriptorPool()
		{
			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = m_PoolSize.size();
			poolInfo.pPoolSizes = m_PoolSize.data();
			poolInfo.maxSets = static_cast<uint32_t>(m_ContextPtr->m_SwapchainImageCount);

			VkResult result = vkCreateDescriptorPool(m_ContextPtr->m_Device, &poolInfo, nullptr, &m_DescriptorPool);

#ifdef USE_LOGGING
			if (result != VK_SUCCESS)
				Utils::Logger::logMSG("Failed to create descriptor pool\n", "DescriptorSetManager", Utils::Severity::Error);
			else
				Utils::Logger::logMSG("Created descriptor pool\n", "DescriptorSetManager", Utils::Severity::Debug);
#endif
		}*/
		void DescriptorSetManager::createDescriptorSetLayout()
		{
			// Describe and create the desriptor layouts for all the descriptors
			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = m_LayoutBindings.size();
			layoutInfo.pBindings = m_LayoutBindings[0].data();

			m_DescriptorSetLayout = m_ContextPtr->m_DescriptorSetLayoutCache.create(&layoutInfo);
		}
		void DescriptorSetManager::createDescriptorSets()
		{
			m_DescriptorSets.resize(m_ContextPtr->m_SwapchainImageCount);	
			
			for (int32_t j = 0; j < m_ContextPtr->m_SwapchainImageCount; j++){
				// Descriptor set for each swap chain image
				m_ContextPtr->m_DescriptorAllocators[j].allocate(&m_DescriptorSets[j].m_DescriptorSet, m_DescriptorSetLayout);
				// Store which pool the descriptor set was stored in
				m_DescriptorSets[j].m_CurrentDescriptorPool = m_ContextPtr->m_DescriptorAllocators[j].getPool(); // Get current pool

				// Point to the uniform buffer that holds the data
				std::vector<VkDescriptorBufferInfo> bufferInfos(m_UniformBufferCount);

				// Loop through all uniform buffer which should be used
				for (int32_t i = 0; i < m_UniformBufferCount; i++){
					bufferInfos[i].buffer = m_DescriptorSetInfo->p_UniformBuffers[i].m_Buffer;
					bufferInfos[i].offset = m_DescriptorSetInfo->p_UniformBuffers[i].m_Offsets[j];
					bufferInfos[i].range = m_DescriptorSetInfo->p_UniformBuffers[i].m_Size;

					VkWriteDescriptorSet dscWrite = {};
					dscWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					dscWrite.dstSet = m_DescriptorSets[j].m_DescriptorSet;
					dscWrite.dstBinding = m_DescriptorSetInfo->p_UniformBuffers[i].m_Binding;
					dscWrite.dstArrayElement = 0;
					dscWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					dscWrite.descriptorCount = 1;
					dscWrite.pBufferInfo = &bufferInfos[i];

					vkUpdateDescriptorSets(m_ContextPtr->m_Device, 1, &dscWrite, 0, nullptr);
				}
				if (m_DescriptorSetInfo->p_StorageBuffer.isValid()) {
					VkDescriptorBufferInfo bufferInfo = {};
					bufferInfo.buffer = m_DescriptorSetInfo->p_StorageBuffer.m_Buffer;
					bufferInfo.offset = m_DescriptorSetInfo->p_StorageBuffer.m_Offsets[j];
					bufferInfo.range = m_DescriptorSetInfo->p_StorageBuffer.m_Size;

					VkWriteDescriptorSet dscWrite = {};
					dscWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					dscWrite.dstSet = m_DescriptorSets[j].m_DescriptorSet;
					dscWrite.dstBinding = m_DescriptorSetInfo->p_StorageBuffer.m_Binding;
					dscWrite.dstArrayElement = 0;
					dscWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					dscWrite.descriptorCount = 1;
					dscWrite.pBufferInfo = &bufferInfo;

					vkUpdateDescriptorSets(m_ContextPtr->m_Device, 1, &dscWrite, 0, nullptr);
				}

				for (int32_t i = 0; i < m_ImageDescriptorInfo.size(); i++){
					//Send image data to uniform sampler
					VkWriteDescriptorSet imageDscWrite = {};
					imageDscWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					imageDscWrite.dstSet = m_DescriptorSets[j].m_DescriptorSet;
					imageDscWrite.dstBinding = m_ImageDescriptorBindings[i];
					imageDscWrite.dstArrayElement = 0;
					imageDscWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					imageDscWrite.descriptorCount = 1;
					imageDscWrite.pImageInfo = &m_ImageDescriptorInfo[j][i];
					vkUpdateDescriptorSets(m_ContextPtr->m_Device, 1, &imageDscWrite, 0, nullptr);
				}
				for(int32_t i = 0; i < m_DescriptorSetInfo->p_ImageArrayDescriptors.size(); i++){
					//Send image data to uniform sampler
					VkWriteDescriptorSet imageDscWrite = {};
					imageDscWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					imageDscWrite.dstSet = m_DescriptorSets[j].m_DescriptorSet;
					imageDscWrite.dstBinding = m_ImageDescriptorBindings[m_ImageDescriptorInfo.size() - 1 + i]; // Image array bindings are aligned after normal image bindings in the std::vector
					imageDscWrite.dstArrayElement = 0;
					imageDscWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					imageDscWrite.descriptorCount = m_DescriptorSetInfo->p_ImageArrayDescriptors[i].m_ImageArrayDescriptorInfo.size();
					imageDscWrite.pImageInfo = m_DescriptorSetInfo->p_ImageArrayDescriptors[i].m_ImageArrayDescriptorInfo.data();
					vkUpdateDescriptorSets(m_ContextPtr->m_Device, 1, &imageDscWrite, 0, nullptr);
				}
			}
		}

		void DescriptorSetManager::updateDescriptorSets()
		{
		}

		void DescriptorSetManager::flush()
		{
			//m_PoolSize.clear();
			m_LayoutBindings.clear();
			m_ImageDescriptorInfo.clear();
			m_ImageDescriptorBindings.clear();
			for (auto& imgArray : m_DescriptorSetInfo->p_ImageArrayDescriptors) {
				imgArray.m_ImageArrayDescriptorInfo.clear();
				imgArray.m_ImageArrayDescriptorBindings.clear();
			}
			
		}

		void UniformBuffer::createBuffer()
		{
			m_Size = m_ContextPtr->padUniformBufferSize(m_Size);

			VkDeviceSize bufferSize = m_Size* m_ContextPtr->m_SwapchainImageCount;
			
			m_MappedData.resize(m_ContextPtr->m_SwapchainImageCount);

			m_ContextPtr->createBuffer
			(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_Buffer,
				m_Memory
			);

			for (int i = 0; i < m_ContextPtr->m_SwapchainImageCount; i++) {
				vkMapMemory(m_ContextPtr->m_Device, m_Memory, i * m_Size, m_ContextPtr->m_SwapchainImageCount * m_Size, 0, &m_MappedData[i]);
				m_Offsets[i] = i * m_Size;
			}

			m_IsValid = true;
		}
		void UniformBuffer::destroy()
		{
			vkDeviceWaitIdle(m_ContextPtr->m_Device);

			vkUnmapMemory(m_ContextPtr->m_Device, m_Memory);

			vkDestroyBuffer(m_ContextPtr->m_Device, m_Buffer, nullptr);
			vkFreeMemory(m_ContextPtr->m_Device, m_Memory, nullptr);

			m_MappedData.clear();
			m_Offsets.clear();

			m_Size = 0;
			m_IsValid = false;
		}
		void StorageBuffer::createBuffer()
		{
			//m_Size = m_Size;

			VkDeviceSize bufferSize = m_ContextPtr->padUniformBufferSize(m_Size) * m_ContextPtr->m_SwapchainImageCount;

			m_MappedData.resize((uint32_t)m_ContextPtr->m_SwapchainImageCount);
			m_Offsets.resize((uint32_t)m_ContextPtr->m_SwapchainImageCount);

			m_AllocInfo = m_ContextPtr->createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_Buffer);

			m_MappedData[0] = m_ContextPtr->mapMemory(m_AllocInfo.p_Alloc);

			m_Offsets[0] = 0;
			for (int i = 1; i < m_ContextPtr->m_SwapchainImageCount; i++) {
				m_Offsets[i] = i * m_ContextPtr->padUniformBufferSize(m_Size);
				m_MappedData[i] = (char*)m_MappedData[0] + i * m_ContextPtr->padUniformBufferSize(m_Size);
			}

			m_IsValid = true;
		}
		void StorageBuffer::destroy()
		{
			if (m_IsValid) {
				vkDeviceWaitIdle(m_ContextPtr->m_Device);

				m_ContextPtr->unmapMemory(m_AllocInfo.p_Alloc);

				m_ContextPtr->destroyBuffer(m_Buffer, m_AllocInfo.p_Alloc);

				m_MappedData.clear();
				m_Offsets.clear();

				m_Size = 0;
				m_IsValid = false;
			}
		}
		VkShaderStageFlagBits getShaderStageVkH(ShaderStage p_ShaderStage)
		{
			if(p_ShaderStage == ShaderStage::VertexBit)
				return VK_SHADER_STAGE_VERTEX_BIT;
			if(p_ShaderStage == ShaderStage::FragmentBit)
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			if(p_ShaderStage == ShaderStage::GeometryBit)
				return VK_SHADER_STAGE_GEOMETRY_BIT;
			return VK_SHADER_STAGE_ALL;
		}
	}
}