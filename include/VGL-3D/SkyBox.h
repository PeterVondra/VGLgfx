#pragma once

#include "Mesh/Mesh.h"
#include "Shapes.h"
#include "../Application/RenderAPI.h"
#include "../GDefinitions.h"

namespace vgl
{
	class Skybox : public MeshData, public Transform3D
	{
		public:
			Skybox();
			Skybox(ImageCube& p_CubeMap);
			~Skybox();

			void create(ImageCube& p_CubeMap);

		private:
			ImageCube* m_CubeMap;
      
      AtmosphericScatteringInfo m_AtmosphericScatteringInfo;

			friend class vk::Renderer;

	};
}
