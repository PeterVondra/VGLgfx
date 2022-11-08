#pragma once

#include <iostream>
#include "Component.h"

namespace vgl
{
	namespace ecs
	{
		enum class Flag
		{
			Optional = 1,
		};

		class BaseSystem
		{
			public:
				BaseSystem() {}
				virtual void updateComponents(ecs::BaseComponent** p_BaseComponents){}

				const std::vector<uint32_t>& getComponentTypes() { return componentTypes; }
				const std::vector<uint32_t>& getComponentFlags() { return componentFlags; }

				bool isValid();
			protected:
				void addComponentType(uint32_t p_ComponentType, uint32_t p_ComponentFlag = 0)
				{
					componentTypes.push_back(p_ComponentType);
					componentFlags.push_back(p_ComponentFlag);
				}
			private:
				std::vector<uint32_t> componentTypes;
				std::vector<uint32_t> componentFlags;
		};

		class SystemList
		{
			public:
				inline bool addSystem(BaseSystem& p_System)
				{
					if (!p_System.isValid())
						return false;

					systems.push_back(&p_System);
					return true;
				}

				inline size_t size() { return systems.size(); };

				inline BaseSystem* operator[](const uint32_t p_Index) { return systems[p_Index]; }

				bool removeSystem(BaseSystem& p_System);
			private:
				std::vector<BaseSystem*> systems;

		};
	}
}