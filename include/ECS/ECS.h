#pragma once

#include <iostream>
#include <vector>
#include <map>

#include "Component.h"
#include "System.h"
#include "../Utils/Logger.h"
#include "../Math/Math.h"

#define VGL_ECS_COMPONENT(T) T : public vgl::ecs::Component<T> // Name of component/struct/class is T
#define VGL_ECS_SYSTEM(T) T : public vgl::ecs::BaseSystem // Name of system/struct/class is T

#define ENTITY_TYPE std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>
#define NULL_COPY_AND_ASSIGN(T) \
	T(const T& other) {(void)other;} \
	void operator=(const T& other) { (void)other; }

namespace vgl
{
	namespace ecs
	{
		class ECS
		{
			public:
				ECS(){}
				~ECS();

				template<class... ComponentT>
				EntityHandle addEntity(ComponentT&... ts)
				{
					BaseComponent* components[] = { &ts... };
					uint32_t componentIDs[] = { ComponentT::ID... };
					return addEntity(components, componentIDs, sizeof(componentIDs) / sizeof(componentIDs[0]));
				}

				// Entity methods
				EntityHandle addEntity(BaseComponent** p_Components, const uint32_t* p_ComponentIDs, size_t p_ComponentCount);
				void deleteEntity(EntityHandle p_Handle);

				// Component methods
				template<class ComponentT>
				inline void addComponent(EntityHandle p_Entity, ComponentT* p_Component)
				{
					addComponentInternal(p_Entity, handleToEntity(p_Entity), ComponentT::ID, p_Component);
				}
				template<class ComponentT>
				inline bool deleteComponent(EntityHandle p_Entity)
				{
					return deleteComponentInternal(p_Entity, ComponentT::ID);
				}
				template<class ComponentT>
				inline ComponentT* getComponent(EntityHandle p_Entity)
				{
					return (ComponentT*)getComponentInternal(handleToEntity(p_Entity), components[ComponentT::ID], ComponentT::ID);
				}

				// System methods
				void updateSystems(SystemList& p_SystemList);

				std::vector<EntityHandle>& getEntities() { return entityHandles; }

			private:
				std::map<uint32_t, std::vector<uint8_t>> components;
				
				std::vector<ENTITY_TYPE*> entities;
				std::vector<EntityHandle> entityHandles;

				inline ENTITY_TYPE* handleToRawType(EntityHandle handle)
				{
					return (ENTITY_TYPE*)handle;
				}
				inline uint32_t handleToEntityIndex(EntityHandle p_Handle)
				{
					return handleToRawType(p_Handle)->first;
				}
				inline std::vector<std::pair<uint32_t, uint32_t>> & handleToEntity(EntityHandle p_Handle)
				{
					return handleToRawType(p_Handle)->second;
				}
				void deleteComponent(uint32_t p_ComponentID, uint32_t p_Index);
				bool deleteComponentInternal(EntityHandle p_Handle, uint32_t p_ComponentID);
				void addComponentInternal(EntityHandle p_Handle, std::vector<std::pair<uint32_t, uint32_t>>& p_Entity, uint32_t p_ComponentID, BaseComponent* p_BaseComponent);
				BaseComponent* getComponentInternal(std::vector<std::pair<uint32_t, uint32_t>>& p_EntityComponents, std::vector<uint8_t>& p_Memory, uint32_t p_ComponentID);

				void updateSystemsMulti
				(
					SystemList& p_SystemList,
					uint32_t p_Index, 
					const std::vector<uint32_t>& p_ComponentTypes, 
					std::vector<BaseComponent*>& p_ComponentParam,
					std::vector<std::vector<uint8_t>*>& p_ComponentMemory
				);

				uint32_t findLeastCommonComponent(const std::vector<uint32_t>& p_ComponentTypes, const std::vector<uint32_t>& p_ComponentFlags);

				NULL_COPY_AND_ASSIGN(ECS);
		};

	}
}