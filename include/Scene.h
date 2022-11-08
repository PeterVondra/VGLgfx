#pragma once

#include "ECS/ECS.h"
#include <yaml-cpp/yaml.h>

namespace vgl
{
	class Scene : public ecs::ECS
	{
		public:
			Scene() { }
			~Scene() {}

      bool serialize_entity(ecs::EntityHandle& p_Handle, YAML::Emitter& p_Emitter);
      bool serialize_bin(const std::string& p_FilePath);
      bool deserialize_bin(const std::string& p_FilePath);  
      bool serialize_yaml(const std::string& p_FilePath);
      bool deserialize_yaml(const std::string& p_FilePath);

		protected:
		private:
			friend class SceneRenderer;

	
	};
}
