#include "System.h"

namespace vgl
{
	namespace ecs
	{
		bool BaseSystem::isValid()
		{
			for (auto& flag : componentFlags)
			{
				if ((flag & (uint32_t)Flag::Optional) == 0)
					return true;
			}
			return false;
		}

		bool SystemList::removeSystem(BaseSystem& p_System)
		{
			for (uint32_t i = 0; i < systems.size(); i++)
			{
				if (&p_System == systems[i])
				{
					systems.erase(systems.begin() + i);
					return true;
				}
			}
			return false;
		}
	}
}