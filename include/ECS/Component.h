#pragma once

#include <iostream>
#include <vector>
#include <tuple>

#define NULL_ENTITY_HANDLE nullptr
#define COMPONENT_TYPE_TUPLE std::tuple<ComponentCreateFun, ComponentFreeFun, size_t>

namespace vgl
{
	namespace ecs
	{
		typedef void* EntityHandle;

		struct BaseComponent;
		typedef uint32_t	(*ComponentCreateFun) (std::vector<uint8_t>& p_Memory, EntityHandle p_Entity, BaseComponent* p_BaseComponent);
		typedef void		(*ComponentFreeFun) (BaseComponent* p_BaseComponent);

		struct BaseComponent
		{
			public:
				static uint32_t registerComponentType(ComponentCreateFun p_CreateFun, ComponentFreeFun p_FreeFun, size_t p_Size);
				EntityHandle entity = NULL_ENTITY_HANDLE;

				inline static ComponentCreateFun getTypeCreateFun(uint32_t id){ return std::get<0>(componentTypes->operator[](id)); }
				inline static ComponentFreeFun getTypeFreeFun(uint32_t id) { return std::get<1>(componentTypes->operator[](id)); }
				inline static size_t getTypeSize(uint32_t id){ { return std::get<2>(componentTypes->operator[](id)); } }
				inline static bool isIDValid(uint32_t ID) { return ID < componentTypes->size(); }

			private:
				static std::vector<COMPONENT_TYPE_TUPLE>* componentTypes;

		};

		template<typename T>
		struct Component : public BaseComponent
		{
			static const ComponentCreateFun CREATE_FUN;
			static const ComponentFreeFun FREE_FUN;
			static const uint32_t ID;
			static const size_t SIZE;
		};

		template<typename T>
		uint32_t ComponentCreate(std::vector<uint8_t>& p_Memory, EntityHandle p_Entity, BaseComponent* p_BaseComponent)
		{
			uint32_t index = p_Memory.size();
			p_Memory.resize(index + T::SIZE);
			T* component = new(&p_Memory[index]) T(*(T*)p_BaseComponent);
			component->entity = p_Entity;
			return index;
		}
		template<typename T>
		void ComponentFree(BaseComponent* p_BaseComponent)
		{
			T* component = (T*)p_BaseComponent;
			component->~T();
		}

		template<typename T> const uint32_t Component<T>::ID(BaseComponent::registerComponentType(ComponentCreate<T>, ComponentFree<T>, sizeof(T)));
		template<typename T> const size_t Component<T>::SIZE(sizeof(T));

		template<typename T> const ComponentCreateFun Component<T>::CREATE_FUN(ComponentCreate<T>);
		template<typename T> const ComponentFreeFun Component<T>::FREE_FUN(ComponentFree<T>);

	}
}