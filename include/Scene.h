#pragma once

#include "ECS/ECS.h"

namespace vgl
{
	class Scene : public ecs::ECS
	{
		public:
			Scene() { }
			~Scene() {}

		protected:
		private:
			friend class SceneRenderer;

	
	};
}