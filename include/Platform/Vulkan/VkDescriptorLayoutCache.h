#pragma once

#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "../../Utils/Logger.h"
#include "../../Utils/Cache.h"
#include "VkDefinitions.h"

#define USE_LOGGING


namespace vgl
{
	namespace vk
	{
		struct DescriptorSetLayoutInfo
		{
			DescriptorSetLayoutInfo() {};
			~DescriptorSetLayoutInfo() {};

			std::vector<VkDescriptorSetLayoutBinding> bindings;

			inline bool operator==(const DescriptorSetLayoutInfo& x) const 
			{
				if (x.bindings.size() != bindings.size())
					return false;
				for (int32_t i = 0; i < bindings.size(); i++) {
					if (x.bindings[i].binding != bindings[i].binding)
						return false;
					if (x.bindings[i].descriptorType != bindings[i].descriptorType)
						return false;
					if (x.bindings[i].descriptorCount != bindings[i].descriptorCount)
						return false;
					if (x.bindings[i].stageFlags != bindings[i].stageFlags)
						return false;
				}
				return true;
			}

			inline size_t hash() const 
			{
				size_t result = std::hash<size_t>()(bindings.size());
				for (const VkDescriptorSetLayoutBinding& b : bindings) {
					size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

					result ^= std::hash<size_t>()(binding_hash);
				}

				return result;
			}
		};

		typedef class DescriptorLayoutCache
		{
			public:
				DescriptorLayoutCache(VkDevice p_Device) : m_Device(p_Device)
				{

				}
				~DescriptorLayoutCache()
				{

				}

				inline void destroy()
				{
					for (auto& pair : m_LayoutCache)
						vkDestroyDescriptorSetLayout(m_Device, pair.second, nullptr);
					m_LayoutCache.clear();
				}
				enum class DescriptorType
				{
					Combined_Image_Sampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					Storage_Buffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					Uniform_Buffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER

				};
				struct Binding
				{
					Binding(DescriptorType p_Type, ShaderStage p_ShaderStage, uint32_t p_Binding, uint32_t p_DescriptorCount = 1) 
					{
						binding.binding = p_Binding;
						binding.descriptorType = (VkDescriptorType)p_Type;
						binding.descriptorCount = p_DescriptorCount;
						binding.stageFlags = (VkShaderStageFlags)p_ShaderStage;
						binding.pImmutableSamplers = nullptr;
					}

					VkDescriptorSetLayoutBinding binding;
				};
				inline VkDescriptorSetLayout createLayout(std::initializer_list<Binding> p_Bindings)
				{
					VkDescriptorSetLayoutCreateInfo info = {};
					info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
					info.bindingCount = p_Bindings.size();
					info.pBindings = &p_Bindings.begin()->binding;

					return create(&info);
				}
				inline VkDescriptorSetLayout create(void* p_Info) 
				{
					VkDescriptorSetLayoutCreateInfo* info = (VkDescriptorSetLayoutCreateInfo*)p_Info;

					DescriptorSetLayoutInfo layoutInfo;
					layoutInfo.bindings.reserve(info->bindingCount);
					bool isSorted = true;
					int32_t lastBinding = -1;

					for (int32_t i = 0; i < info->bindingCount; i++) {
						layoutInfo.bindings.push_back(info->pBindings[i]);

						if (info->pBindings[i].binding > lastBinding) {
							lastBinding = info->pBindings[i].binding;
							continue;
						}
						isSorted = false;
					}

					if (!isSorted) {
						std::sort(layoutInfo.bindings.begin(), layoutInfo.bindings.end(),
							[](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b){
								return a.binding < b.binding;
							}
						);
					}

					auto it = m_LayoutCache.find(layoutInfo);
					if (it != m_LayoutCache.end())
						return (*it).second;

					VkDescriptorSetLayout layout;
					VkResult result = vkCreateDescriptorSetLayout(m_Device, info, nullptr, &layout);

#ifdef USE_LOGGING
					if (result != VK_SUCCESS)
						Utils::Logger::logMSG("Failed to create descriptor set layout\n", "DescriptorSetManager", Utils::Severity::Error);
					else
						Utils::Logger::logMSG("Created descriptor set layout\n", "DescriptorSetManager", Utils::Severity::Debug);
#endif

					m_LayoutCache[layoutInfo] = layout;
					return layout;
				}
			private:
				friend class Context;

				VkDevice m_Device;

				struct DescriptorLayoutHash {
					std::size_t operator()(const DescriptorSetLayoutInfo& k) const {
						return k.hash();
					}
				};

				std::unordered_map<DescriptorSetLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> m_LayoutCache;
		} DLC;
	}
}