/*#include "Grid.h"

namespace vgl
{
    Grid::Grid(const unsigned int SIZE_X, const unsigned int SIZE_Y, const float QUAD_SIZE_X, const float QUAD_SIZE_Y)
    {
		for (unsigned int i = 0; i < SIZE_X; i++)
		{
			for (int j = 0; j < SIZE_Y; j++)
			{
				Vector3f size = Vector3f(QUAD_SIZE_X, 0, QUAD_SIZE_Y);
				float x = (float)i * (QUAD_SIZE_X / SIZE_X);
				float y = (float)j * (QUAD_SIZE_Y / SIZE_Y);

				Vertex vertex0;
				Vertex vertex1;
				Vertex vertex2;

				vertex0.position = Vector3f(x, 0, y);
				vertex1.position = Vector3f(x, 0, y + (QUAD_SIZE_Y / SIZE_Y));
				vertex2.position = Vector3f(x + (QUAD_SIZE_X / SIZE_X), 0, y);

				vertex0.uv = Vector2f(1.0f, 0.0f);
				vertex1.uv = Vector2f(1.0f, 1.0f);
				vertex2.uv = Vector2f(0.0f, 0.0f);

				Vector3f normal = Math::normalize(Math::cross(vertex1.position - vertex0.position, vertex2.position - vertex0.position));

				vertex0.normal = normal;
				vertex1.normal = normal;
				vertex2.normal = normal;

				Vertex vertex00;
				Vertex vertex01;
				Vertex vertex02;

				vertex00.position = (vertex0.position * -1) + size;
				vertex01.position = (vertex1.position * -1) + size;
				vertex02.position = (vertex2.position * -1) + size;

				normal = Math::normalize(Math::cross(vertex01.position - vertex00.position, vertex02.position - vertex00.position));

				vertex00.uv = Vector2f(0.0f, 1.0f);
				vertex01.uv = Vector2f(0.0f, 0.0f);
				vertex02.uv = Vector2f(1.0f, 1.0f);

				vertex00.normal = normal;
				vertex01.normal = normal;
				vertex02.normal = normal;

				vertices.push_back(vertex0);
				vertices.push_back(vertex1);
				vertices.push_back(vertex2);

				vertices.push_back(vertex00);
				vertices.push_back(vertex01);
				vertices.push_back(vertex02);
			}
		}

		calcTBN(vertices);

		indexVertexData(vertices, m_MeshData);

		init();
    }

    Grid::~Grid()
    {
        
    }

    void Grid::initialize()
    {
        
    }

	void Grid::recalculateNormals()
	{
		m_MeshData.vertices.clear();
		m_MeshData.indices.clear();

		for (int i = 0; i < vertices.size(); i += 3)
		{
			Vector3f normal = Math::normalize(Math::cross(vertices[i + 1].position - vertices[i].position, vertices[i + 2].position - vertices[i].position));

			vertices[i].normal = normal;
			vertices[i].normal = normal;
			vertices[i].normal = normal;
		}

		calcTBN(vertices);

		indexVertexData(vertices, m_MeshData);
	}

    void Grid::setVertexPosition(const unsigned int INDEX, const Vector3f POSITION)
    {
        //vertices[INDEX] = POSITION;
        //initialize();
    }

    float Grid::curve(const float X, const float Y)
    {
        return sin(X * 2 * Math::PI) * sin(Y * 2 * Math::PI) * 0.1;
    }
    float Grid::curve(Vector2f XY)
    {
        return sin(XY.x * 2 * Math::PI) * sin(XY.y * 2 * Math::PI) * 0.1;
    }

	void Grid::indexVertexData(std::vector<Vertex>& p_Vertices, MeshData& p_Data)
	{
		std::map<Vertex, uint32_t> vertexToIndex;

		std::map<Vertex, uint32_t>::iterator it;
		uint32_t index;
		for (uint32_t i = 0; i < p_Vertices.size(); i++)
		{
			it = vertexToIndex.find(p_Vertices[i]);
			if (it != vertexToIndex.end())
			{
				index = it->second;

				p_Data.vertices[index].tangent += p_Vertices[i].tangent;
				p_Data.vertices[index].normal += p_Vertices[i].normal;
				p_Data.vertices[index].bitangent += p_Vertices[i].bitangent;
				p_Data.indices.push_back(index);
				continue;
			}
			p_Data.vertices.push_back(p_Vertices[i]);

			p_Data.indices.push_back(p_Data.vertices.size() - 1);
			vertexToIndex[p_Vertices[i]] = p_Data.vertices.size() - 1;
		}
	}

	void Grid::calcTBN(std::vector<Vertex>& p_Vertices)
	{
		//calculate tangents
		for (unsigned int i = 0; i < p_Vertices.size(); i += 3)
		{
			Vector3f deltapos1 = p_Vertices[i + 1].position - p_Vertices[i].position;
			Vector3f deltapos2 = p_Vertices[i + 2].position - p_Vertices[i].position;

			Vector2f deltauv1 = p_Vertices[i + 1].uv - p_Vertices[i].uv;
			Vector2f deltauv2 = p_Vertices[i + 2].uv - p_Vertices[i].uv;

			float dirCorrection = (deltauv2.x * deltauv1.y - deltauv2.y * deltauv1.x) < 0.0f ? -1.0f : 1.0f;

			if (deltauv1.x * deltauv2.y == deltauv1.y * deltauv2.x) {
				deltauv1.x = 0.0; deltauv1.x = 1.0;
				deltauv2.x = 1.0; deltauv2.y = 0.0;
			}

			Vector3f tangent(1, 0, 0);
			Vector3f bitangent(0, 1, 0);

			tangent = (deltapos2 * deltauv1.y - deltapos1 * deltauv2.y) * dirCorrection;
			bitangent = (deltapos2 * deltauv1.x - deltapos1 * deltauv2.x) * dirCorrection;

			for (unsigned int j = 0; j < 3; j++)
			{
				Vertex& vertex = p_Vertices[i + j];

				vertex.tangent = Math::normalize(tangent - vertex.normal * (tangent * vertex.normal));
				vertex.bitangent = Math::normalize(bitangent - vertex.normal * (bitangent * vertex.normal));

				// reconstruct tangent/bitangent according to normal and bitangent/tangent when it's infinite or NaN.
				bool invalid_tangent =
					std::isnan(vertex.tangent.x) || std::isnan(vertex.tangent.y) || std::isnan(vertex.tangent.z) ||
					std::isinf(vertex.tangent.x) || std::isinf(vertex.tangent.y) || std::isinf(vertex.tangent.z);

				bool invalid_bitangent =
					std::isnan(vertex.bitangent.x) || std::isnan(vertex.bitangent.y) || std::isnan(vertex.bitangent.z) ||
					std::isinf(vertex.bitangent.x) || std::isinf(vertex.bitangent.y) || std::isinf(vertex.bitangent.z);

				if (invalid_tangent != invalid_bitangent) {
					if (invalid_tangent)
						vertex.tangent = Math::cross(vertex.normal, vertex.bitangent);
					else
						vertex.bitangent = Math::cross(vertex.tangent, vertex.normal);
				}
			}
		}
	}
}
*/