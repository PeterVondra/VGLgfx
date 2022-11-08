/*#pragma once

#include <iostream>
#include <vector>
#include <cmath>

#include "../VGL-Core/Math/Math.h"
#include "../VGL-Core/Math/Vector.h"
#include "Mesh/Mesh.h"

namespace vgl
{
	class Grid : public Model
	{
		public:
			Grid() {};
			Grid(const unsigned int SIZE_X, const unsigned int SIZE_Y, const float QUAD_SIZE_X, const float QUAD_SIZE_Y);
			~Grid();

			void initialize();
			void setVertexPosition(const unsigned int INDEX, const Vector3f POSITION);

			float curve(const float X, const float Y);
			float curve(Vector2f XY);

			void recalculateNormals();

			std::vector<Vertex> vertices;
		protected:
		private:

			void indexVertexData(std::vector<Vertex>& p_Vertices, MeshData& p_Data);
			void calcTBN(std::vector<Vertex>& p_Vertices);
		
	};
}*/
