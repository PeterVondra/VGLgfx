#include "Shapes.h"

namespace vgl
{
	void computeHalfVertex(Vector3f v1, Vector3f v2, Vector3f& rs)
	{
		rs = v1 + v2;
		rs *= Math::magnitude(rs);
	}

	// Radius is one unit length
	void generateUVSphere(uint32_t p_Stacks, uint32_t p_Sectors, MeshData& p_MeshData)
	{
		std::vector<Vertex5T> vertices;
		std::vector<Vertex5T> vertices2;

		const float sectorStep = 2 * Math::PI / p_Sectors;
		const float stackStep = Math::PI / p_Stacks;
		float sectAngle, stackAngle;
		float xy = 0;
		float z = 0;
		float x, y;

		int32_t k1, k2;

		for (int32_t i = 0; i <= p_Stacks; ++i)
		{
			stackAngle = Math::PI / 2 - i * stackStep;
			xy = 1 * std::cos(stackAngle);
			z = 1 * std::sin(stackAngle);

			for (int32_t j = 0; j <= p_Sectors; ++j)
			{
				Vertex5T v;
				sectAngle = j * sectorStep;

				x = xy * std::cos(sectAngle);
				y = xy * std::sin(sectAngle);

				v.position = { x, y, z };
				v.normal = v.position;
				v.uv = { j / (float)p_Sectors, i / (float)p_Stacks };

				vertices2.push_back(v);
			}
		}

		for (int32_t i = 0; i < p_Stacks; ++i)
		{
			k1 = i * (p_Sectors + 1);
			k2 = k1 + p_Sectors + 1;

			for (int32_t j = 0; j < p_Sectors; ++j, ++k1, ++k2)
			{

				if (i != 0)
				{
					vertices.push_back(vertices2[k1]);
					vertices.push_back(vertices2[k2]);
					vertices.push_back(vertices2[k1 + 1]);
				}

				if (i != p_Stacks - 1)
				{
					vertices.push_back(vertices2[k1 + 1]);
					vertices.push_back(vertices2[k2]);
					vertices.push_back(vertices2[k2 + 1]);
				}
			}
		}

		for (uint32_t i = 0; i < vertices.size() && i + 3 < vertices.size(); i += 3)
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

			for (uint32_t j = 0; j < 3; j++)
			{
				Vertex5T& vertex = vertices[i + j];

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

				if (Math::dot(Math::cross(vertex.normal, vertex.tangent), vertex.bitangent) < 0.0f);
				{
					vertex.tangent *= -1.0f;
					vertex.bitangent *= -1.0f;
				}

			}
		}

		MeshData::indexVertexData(vertices, p_MeshData);

		p_MeshData.init();
	}

	// Radius is one unit length
	void generateICOSphere(uint32_t p_Subdivisions, MeshData& p_MeshData)
	{
		std::vector<Vertex5T> vertices;
		std::vector<Vertex5T> vertices2;

		uint32_t index;

		const float H_Angle = Math::PI / 180 * 72;
		const float V_Angle = std::atan(1.0f / 2);
		int32_t i1 = 0, i2 = 0;
		float z, xy;
		float hAngle1 = -Math::PI / 2 - H_Angle / 2;
		float hAngle2 = -Math::PI / 2;

		vertices.resize(12);

		vertices[0].position = { 0, 0, 1 };

		for (int32_t i = 1; i <= 5; ++i)
		{
			i1 = i;
			i2 = (i + 5);

			z = std::sin(V_Angle);
			xy = std::cos(V_Angle);

			vertices[i1].position = { xy * cos(hAngle1), xy * sin(hAngle1), z };
			vertices[i2].position = { xy * cos(hAngle2), xy * sin(hAngle2), -z };

			hAngle1 += H_Angle;
			hAngle2 += H_Angle;
		}

		vertices[11].position = { 0, 0, 1 };

		Vertex5T newV1;
		Vertex5T newV2;
		Vertex5T newV3;

		for (int32_t i = 1; i < p_Subdivisions; i++)
		{
			vertices2 = vertices;
			vertices.clear();
			index = 0;

			for (int32_t j = 0; j < vertices2.size(); j += 3)
			{
				computeHalfVertex(vertices2[j].position, vertices2[j + 1].position, newV1.position);
				computeHalfVertex(vertices2[j + 1].position, vertices2[j + 2].position, newV2.position);
				computeHalfVertex(vertices2[j].position, vertices2[j + 2].position, newV3.position);

				vertices.push_back(vertices2[j]);
				vertices.push_back(newV1);
				vertices.push_back(newV3);

				vertices.push_back(newV1);
				vertices.push_back(vertices2[j + 1]);
				vertices.push_back(newV2);

				vertices.push_back(newV1);
				vertices.push_back(newV2);
				vertices.push_back(newV3);

				vertices.push_back(newV3);
				vertices.push_back(newV2);
				vertices.push_back(vertices2[j + 2]);
			}

		}

		for (uint32_t i = 0; i < vertices.size() && i + 3 < vertices.size(); i += 3)
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

			for (uint32_t j = 0; j < 3; j++)
			{
				Vertex5T& vertex = vertices[i + j];

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

				if (Math::dot(Math::cross(vertex.normal, vertex.tangent), vertex.bitangent) < 0.0f);
				{
					vertex.tangent *= -1.0f;
					vertex.bitangent *= -1.0f;
				}

			}
		}

		MeshData::indexVertexData(vertices, p_MeshData);

		p_MeshData.init();
	}

	// Radius is one unit length
	void generateCUBESphere(uint32_t p_Subdivisions, MeshData& p_MeshData)
	{
		std::vector<Vertex5T> vertices;

		Vector3f n1;
		Vector3f n2;
		Vector3f v;
		float a1;
		float a2;

		Vector2f t1;

		int32_t VPR = (int32_t)std::pow(2, p_Subdivisions) + 1;

		float rot[6] = { 0, Math::PI / 4, Math::PI / 4, -Math::PI / 4, -Math::PI / 4, Math::PI / 2 };
		Vector2f rotN[6] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 0 }, { 0, 1 }, { 1, 0 } };

		//for (int32_t k = 0; k < 6; k++) {
		for (uint32_t i = 0; i < VPR; ++i)
		{
			a2 = Math::DEG2RAD * (45 - 90 * i / (VPR - 1));
			n2 = { -std::sin(a2), std::cos(a2), 0 };

			t1.y = (float)i / (VPR - 1);

			for (uint32_t j = 0; j < VPR; ++j)
			{
				a1 = Math::DEG2RAD * (-45 + 90 * j / (VPR - 1));
				n1 = { -std::sin(a1), 0, -std::cos(a1) };

				v = Math::normalize(Math::cross(n1, n2));

				t1.x = (float)j / (VPR - 1);

				Vertex5T vertex;
				//Vector3f r = { Math::rotate(rotN[k], Math::RAD2DEG * rot[k]).x, 1, Math::rotate(rotN[k], Math::RAD2DEG * rot[k]).y};
				vertex.position = v;
				vertex.uv = t1;
				vertices.push_back(vertex);
			}
		}
		//}

		for (int32_t i = 0; i < vertices.size(); i += 3)
		{
			vertices[i].normal = Math::normalize(Math::cross(vertices[i + 1].position - vertices[i].position, vertices[i + 2].position - vertices[i].position));
			vertices[i + 1].normal = Math::normalize(Math::cross(vertices[i + 1].position - vertices[i].position, vertices[i + 2].position - vertices[i].position));
			vertices[i + 2].normal = Math::normalize(Math::cross(vertices[i + 1].position - vertices[i].position, vertices[i + 2].position - vertices[i].position));
		}

		for (uint32_t i = 0; i < vertices.size() && i + 3 < vertices.size(); i += 3)
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

			for (uint32_t j = 0; j < 3; j++)
			{
				Vertex5T& vertex = vertices[i + j];

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

				if (Math::dot(Math::cross(vertex.normal, vertex.tangent), vertex.bitangent) < 0.0f);
				{
					vertex.tangent *= -1.0f;
					vertex.bitangent *= -1.0f;
				}

			}
		}

		MeshData::indexVertexData(vertices, p_MeshData);

		p_MeshData.init();
	}

	void generateCube(MeshData& p_MeshData)
	{
		std::vector<Vertex5T> vertices;

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

		for (uint32_t i = 0; i < vertices.size(); i += 3)
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

			for (uint32_t j = 0; j < 3; j++)
			{
				Vertex5T& vertex = vertices[i + j];

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

		MeshData::indexVertexData(vertices, p_MeshData);

		p_MeshData.init();
	}

	void calcTBN(std::vector<Vertex5T>& p_Vertices)
	{
		//calculate tangents
		for (uint32_t i = 0; i < p_Vertices.size(); i += 3)
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

			for (uint32_t j = 0; j < 3; j++)
			{
				Vertex5T& vertex = p_Vertices[i + j];

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

	void generateGrid(const uint32_t p_QuadCountX, const uint32_t p_QuadCountY, const Vector2f p_QuadSize, MeshData& p_MeshData)
	{
		std::vector<Vertex5T> vertices;

		for (unsigned int i = 0; i < p_QuadCountX; i++)
		{
			for (int j = 0; j < p_QuadCountY; j++)
			{
				Vector3f size = Vector3f(p_QuadSize.x, 0, p_QuadSize.y);
				float x = (float)i * (p_QuadSize.x / p_QuadCountX);
				float y = (float)j * (p_QuadSize.y / p_QuadCountY);

				Vertex5T vertex0;
				Vertex5T vertex1;
				Vertex5T vertex2;

				vertex0.position = Vector3f(x, 0, y);
				vertex1.position = Vector3f(x, 0, y + (p_QuadSize.y / p_QuadCountY));
				vertex2.position = Vector3f(x + (p_QuadSize.x / p_QuadCountX), 0, y);

				//vertex0.uv = Vector2f(1.0f, 0.0f);
				//vertex1.uv = Vector2f(1.0f, 1.0f);
				//vertex2.uv = Vector2f(0.0f, 0.0f);

				Vector3f normal = Math::normalize(Math::cross(vertex1.position - vertex0.position, vertex2.position - vertex0.position));

				vertex0.normal = normal;
				vertex1.normal = normal;
				vertex2.normal = normal;

				Vertex5T vertex00;
				Vertex5T vertex01;
				Vertex5T vertex02;

				vertex00.position = (vertex0.position * -1) + size;
				vertex01.position = (vertex1.position * -1) + size;
				vertex02.position = (vertex2.position * -1) + size;

				normal = Math::normalize(Math::cross(vertex01.position - vertex00.position, vertex02.position - vertex00.position));

				//vertex00.uv = Vector2f(0.0f, 1.0f);
				//vertex01.uv = Vector2f(0.0f, 0.0f);
				//vertex02.uv = Vector2f(1.0f, 1.0f);

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

		MeshData::indexVertexData(vertices, p_MeshData);

		for (auto& v : p_MeshData.vertices) {
			v.position.y = Math::getRandomNumberND(0.0f, 300);
		}
		
		for (int i = 0; i < p_MeshData.vertices.size(); i += 6) {
			if (i >= p_MeshData.vertices.size())
				break;

			Vector3f normal0 = Math::normalize(Math::cross(p_MeshData.vertices[i+1].position - p_MeshData.vertices[i].position, p_MeshData.vertices[i + 2].position - p_MeshData.vertices[i].position));
			Vector3f normal = Math::normalize(Math::cross(p_MeshData.vertices[i+4].position - p_MeshData.vertices[i+3].position, p_MeshData.vertices[i + 5].position - p_MeshData.vertices[i+3].position));

			p_MeshData.vertices[i].normal = normal0;
			p_MeshData.vertices[i+1].normal = normal0;
			p_MeshData.vertices[i+2].normal = normal0;

			p_MeshData.vertices[i].uv = Vector2f(1.0f, 0.0f);
			p_MeshData.vertices[i + 1].uv = Vector2f(1.0f, 1.0f);
			p_MeshData.vertices[i + 2].uv = Vector2f(0.0f, 0.0f);

			p_MeshData.vertices[i+3].uv = Vector2f(0.0f, 1.0f);
			p_MeshData.vertices[i+4].uv = Vector2f(0.0f, 0.0f);
			p_MeshData.vertices[i+5].uv = Vector2f(1.0f, 1.0f);

			p_MeshData.vertices[i + 3].normal = normal;
			p_MeshData.vertices[i + 4].normal = normal;
			p_MeshData.vertices[i + 5].normal = normal;
		}

		calcTBN(p_MeshData.vertices);

		p_MeshData.init();
	}
}
