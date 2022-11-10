#include "Component.h"

namespace vgl
{
	namespace ecs
	{
		std::vector<COMPONENT_TYPE_TUPLE>* BaseComponent::componentTypes;

		uint32_t BaseComponent::registerComponentType(ComponentCreateFun p_CreateFun, ComponentFreeFun p_FreeFun, size_t p_Size)
		{
			if (componentTypes == nullptr)
				componentTypes = new std::vector<COMPONENT_TYPE_TUPLE>();
			uint32_t componentID = componentTypes->size();
			
			componentTypes->push_back(COMPONENT_TYPE_TUPLE(p_CreateFun, p_FreeFun, p_Size));

			return componentID;
		}
	}
}