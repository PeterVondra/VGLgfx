/*#pragma once

#include "Mesh/Model.h"
#include "../VGL-Core/Math/Transform.h"

namespace vgl
{
	class Cuboid : public Model
	{
		public:
			Cuboid() {};
			Cuboid(Vector3f p_Size, Vector3f p_Position, Vector3f p_Color);
			~Cuboid();

			Vector3f getSize() { return scalar; }

		protected:
		private:
			void indexVertexData(std::vector<Vertex>& p_Vertices, MeshData& p_Data);

			std::vector<Vertex> vertices;
			Vector3f m_Size;

	};
}*/