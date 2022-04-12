#pragma once

#include "Mesh/Mesh.h"
#include "Shapes.h"
#include "../Platform/Vulkan/Buffers/VulkanImage.h"

namespace vgl
{
	class Skybox : public MeshData, public Transform3D
	{
		public:
			Skybox();
			Skybox(vk::CubeMap& p_CubeMap);
			~Skybox();

			void create(vk::CubeMap& p_CubeMap);

		private:
			vk::CubeMap* m_CubeMap;
			friend class vk::Renderer;

	};
}