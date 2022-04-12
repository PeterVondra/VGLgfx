/*#include "Cuboid.h"

namespace vgl
{
	Cuboid::Cuboid(Vector3f p_Size, Vector3f p_Position, Vector3f p_Color) : Model::Model()
	{
		scale(p_Size);

		vertices.emplace_back(Vector3f(0.5f, 0.5f, -0.5f), Vector2f(1.0f, 1.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, -0.5f, -0.5f), Vector2f(1.0f, 0.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, -0.5f, -0.5f), Vector2f(0.0f, 0.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, -0.5f, -0.5f), Vector2f(0.0f, 0.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, 0.5f, -0.5f), Vector2f(0.0f, 1.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, 0.5f, -0.5f), Vector2f(1.0f, 1.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0), Vector3f(0));
		
		vertices.emplace_back(Vector3f(-0.5f, -0.5f, 0.5f), Vector2f(0.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, -0.5f, 0.5f), Vector2f(1.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, 0.5f, 0.5f), Vector2f(1.0f, 1.0f), Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, 0.5f, 0.5f), Vector2f(1.0f, 1.0f), Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, 0.5f, 0.5f), Vector2f(0.0f, 1.0f), Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, -0.5f, 0.5f), Vector2f(0.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0), Vector3f(0));
		
		vertices.emplace_back(Vector3f(-0.5f, 0.5f, 0.5f), Vector2f(1.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, 0.5f, -0.5f), Vector2f(1.0f, 1.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, -0.5f, -0.5f), Vector2f(0.0f, 1.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, -0.5f, -0.5f), Vector2f(0.0f, 1.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, -0.5f, 0.5f), Vector2f(0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, 0.5f, 0.5f), Vector2f(1.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		
		vertices.emplace_back(Vector3f(0.5f, -0.5f, -0.5f), Vector2f(0.0f, 1.0f), Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, 0.5f, -0.5f), Vector2f(1.0f, 1.0f), Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, 0.5f, 0.5f), Vector2f(1.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, 0.5f, 0.5f), Vector2f(1.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, -0.5f, 0.5f), Vector2f(0.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, -0.5f, -0.5f), Vector2f(0.0f, 1.0f), Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0), Vector3f(0));
		
		vertices.emplace_back(Vector3f(-0.5f, -0.5f, -0.5f), Vector2f(0.0f, 1.0f), Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, -0.5f, -0.5f), Vector2f(1.0f, 1.0f), Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, -0.5f, 0.5f), Vector2f(1.0f, 0.0f), Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, -0.5f, 0.5f), Vector2f(0.0f, 0.0f), Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, -0.5f, -0.5f), Vector2f(0.0f, 1.0f), Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, -0.5f, 0.5f), Vector2f(1.0f, 0.0f), Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0), Vector3f(0));
		
		vertices.emplace_back(Vector3f(0.5f, 0.5f, 0.5f), Vector2f(0.0f, 1.0f), Vector3f(0.0f, 1.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, 0.5f, -0.5f), Vector2f(1.0f, 1.0f), Vector3f(0.0f, 1.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, 0.5f, -0.5f), Vector2f(1.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, 0.5f, -0.5f), Vector2f(1.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(-0.5f, 0.5f, 0.5f), Vector2f(0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), Vector3f(0), Vector3f(0));
		vertices.emplace_back(Vector3f(0.5f, 0.5f, 0.5f), Vector2f(0.0f, 1.0f), Vector3f(0.0f, 1.0f, 0.0f), Vector3f(0), Vector3f(0));

		for (unsigned int i = 0; i < vertices.size(); i += 3)
		{
			Vector3f deltapos1 = vertices[i + 1].position - vertices[i].position;
			Vector3f deltapos2 = vertices[i + 2].position - vertices[i].position;

			Vector2f deltauv1 = vertices[i + 1].uv - vertices[i].uv;
			Vector2f deltauv2 = vertices[i + 2].uv - vertices[i].uv;

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
				Vertex& vertex = vertices[i + j];

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

		indexVertexData(vertices, m_MeshData);

		init();
	}

	Cuboid::~Cuboid()
	{

	}

	void Cuboid::indexVertexData(std::vector<Vertex>& p_Vertices, MeshData& p_Data)
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
}*/
