#include "ECS.h"
#include <cstring>

namespace vgl
{
	namespace ecs
	{
		ECS::~ECS()
		{
			for (auto& component : components)
			{
				size_t typeSize = BaseComponent::getTypeSize(component.first);
				ComponentFreeFun freeFun = BaseComponent::getTypeFreeFun(component.first);

				for (int i = 0; i < component.second.size(); i += typeSize)
					freeFun((BaseComponent*)&component.second[i]);

			}

			for (auto& entity : entities)
				delete entity;
		}

		// Entity methods
		EntityHandle ECS::addEntity(BaseComponent** p_Components, const uint32_t* p_ComponentIDs, size_t p_ComponentCount)
		{
			ENTITY_TYPE* newEntity = new ENTITY_TYPE();
			EntityHandle handle = (EntityHandle)newEntity;

			for (int i = 0; i < p_ComponentCount; i++)
			{
				// Check components for valid ID
				if (!BaseComponent::isIDValid(p_ComponentIDs[i])){
					VGL_LOG_MSG("Invalid ID for new entity", "ECS", Utils::Severity::Warning);
					VGL_LOG_MSG("Entity creation", "ECS", Utils::Result::Failed);
					delete newEntity;
					return NULL_ENTITY_HANDLE;
				}

				addComponentInternal(handle, newEntity->second, p_ComponentIDs[i], p_Components[i]);
			}

			newEntity->first = entities.size();
			entities.push_back(newEntity);
			entityHandles.push_back(handle);
			return handle;
		}
		void ECS::deleteEntity(EntityHandle p_Handle)
		{
			std::vector<std::pair<uint32_t, uint32_t>>& entity = handleToEntity(p_Handle);
			for (auto& component : entity)
				deleteComponent(component.first, component.second);

			uint32_t dstIndex = handleToEntityIndex(p_Handle);
			uint32_t srcIndex = entities.size() - 1;
			delete entities[dstIndex];
			entities[dstIndex] = entities[srcIndex];
			entities[dstIndex]->first = dstIndex;
			entities.pop_back();
			entityHandles.pop_back();
		}

		// Component methods
		void ECS::addComponentInternal(EntityHandle p_Handle, std::vector<std::pair<uint32_t, uint32_t>>& p_Entity, uint32_t p_ComponentID, BaseComponent* p_BaseComponent)
		{
			ComponentCreateFun  createFun = BaseComponent::getTypeCreateFun(p_ComponentID);
			p_Entity.emplace_back(p_ComponentID, createFun(components[p_ComponentID], p_Handle, p_BaseComponent));
		}

		void ECS::deleteComponent(uint32_t p_ComponentID, uint32_t p_Index)
		{
			std::vector<uint8_t>& memory = components[p_ComponentID];
			ComponentFreeFun freeFun = BaseComponent::getTypeFreeFun(p_ComponentID);
			size_t typeSize = BaseComponent::getTypeSize(p_ComponentID);

			uint32_t srcIndex = memory.size() - typeSize;

			BaseComponent* srcComponent = (BaseComponent*)&memory[srcIndex];
			BaseComponent* dstComponent = (BaseComponent*)&memory[p_Index];

			freeFun(dstComponent);

			if (p_Index == srcIndex)
			{
				memory.resize(srcIndex);
				return;
			}

			memcpy(dstComponent, srcComponent, typeSize);

			std::vector<std::pair<uint32_t, uint32_t>>& srcComponents = handleToEntity(srcComponent->entity);

			for (int i = 0; i < srcComponents.size(); i++)
			{
				if (p_ComponentID == srcComponents[i].first && srcIndex == srcComponents[i].second)
				{
					srcComponents[i].second = p_Index;
					break;
				}
			}

			memory.resize(srcIndex);
		}

		bool ECS::deleteComponentInternal(EntityHandle p_Handle, uint32_t p_ComponentID)
		{
			std::vector<std::pair<uint32_t, uint32_t>>& entiyComponents = handleToEntity(p_Handle);

			for (int i = 0; i < entiyComponents.size(); i++)
			{
				if (p_ComponentID == entiyComponents[i].first)
				{
					deleteComponent(entiyComponents[i].first, entiyComponents[i].second);
					
					uint32_t srcIndex = entiyComponents.size() - 1;
					uint32_t dstIndex = i;
					entiyComponents[dstIndex] = entiyComponents[srcIndex];
					entiyComponents.pop_back();
					return true;
				}
			}

			return false;
		}

		BaseComponent* ECS::getComponentInternal(std::vector<std::pair<uint32_t, uint32_t>>& p_EntityComponents, std::vector<uint8_t>& p_Memory, uint32_t p_ComponentID)
		{
			for (uint32_t i = 0; i < p_EntityComponents.size(); i++)
			{
				if (p_ComponentID == p_EntityComponents[i].first)
				{
					return (BaseComponent*)&p_Memory[p_EntityComponents[i].second];
				}
			}

			return nullptr;
		}

		// System methods
		void ECS::updateSystems(SystemList& p_SystemList)
		{
			std::vector<BaseComponent*> componentParam;
			std::vector<std::vector<uint8_t>*> componentMemory;
			
			for (uint32_t i = 0; i < p_SystemList.size(); i++)
			{
				const std::vector<uint32_t> componentTypes = p_SystemList[i]->getComponentTypes();
				if (componentTypes.size() == 1)
				{
					size_t typeSize = BaseComponent::getTypeSize(componentTypes[0]);
					std::vector<uint8_t>& memory = components[componentTypes[0]];
					for (uint32_t j = 0; j < memory.size(); j += typeSize)
					{
						BaseComponent* component = (BaseComponent*)&memory[j];
						p_SystemList[i]->updateComponents(&component);
					}
				}
				else
				{
					updateSystemsMulti(p_SystemList, i, componentTypes, componentParam, componentMemory);

				}
			}
		}

		void ECS::updateSystemsMulti
		(
			SystemList& p_SystemList,
			uint32_t p_Index, 
			const std::vector<uint32_t>& p_ComponentTypes, 
			std::vector<BaseComponent*>& p_ComponentParam,
			std::vector<std::vector<uint8_t>*>& p_ComponentMemory
		)
		{
			const std::vector<uint32_t>& componentFlags = p_SystemList[p_Index]->getComponentFlags();

			#undef max
			p_ComponentParam.resize(std::max(p_ComponentParam.size(), p_ComponentTypes.size()));
			p_ComponentMemory.resize(std::max(p_ComponentMemory.size(), p_ComponentTypes.size()));

			for (uint32_t i = 0; i < p_ComponentTypes.size(); i++)
				p_ComponentMemory[i] = &components[p_ComponentTypes[i]];

			uint32_t minSizeIndex = findLeastCommonComponent(p_ComponentTypes, componentFlags);
			size_t typeSize = BaseComponent::getTypeSize(p_ComponentTypes[minSizeIndex]);

			for (uint32_t i = 0; i < p_ComponentMemory[minSizeIndex]->size(); i += typeSize)
			{
				p_ComponentParam[minSizeIndex] = (BaseComponent*)&p_ComponentMemory[minSizeIndex][i];
				std::vector<std::pair<uint32_t, uint32_t>>& entityComponents = handleToEntity(p_ComponentParam[minSizeIndex]->entity);

				bool isValid = true;
				for (uint32_t j = 0; j < p_ComponentTypes.size(); j++)
				{
					if (j == minSizeIndex)
						continue;

					p_ComponentParam[j] == getComponentInternal(entityComponents, *p_ComponentMemory[j], p_ComponentTypes[j]);
					if (p_ComponentParam[j] == nullptr && (componentFlags[j] & (int)Flag::Optional) == 0)
					{
						isValid = false;
						break;
					}
				}

				if(isValid)
					p_SystemList[p_Index]->updateComponents(&p_ComponentParam[0]);
			}
		}

		uint32_t ECS::findLeastCommonComponent(const std::vector<uint32_t>& p_ComponentTypes, const std::vector<uint32_t>& p_ComponentFlags)
		{
			uint32_t minSize = (uint32_t) -1;
			uint32_t minIndex = (uint32_t)-1;

			for (uint32_t i = 1; i < p_ComponentTypes.size(); i++)
			{
				if ((p_ComponentFlags[i] & (int)Flag::Optional) != 0)
					continue;

				size_t typeSize = BaseComponent::getTypeSize(p_ComponentTypes[i]);
				uint32_t size = components[p_ComponentTypes[i]].size()/typeSize;

				if (size <= minSize) {
					minSize = size;
					minIndex = i;
				}
			}

			return minIndex;
		}
	}
}
