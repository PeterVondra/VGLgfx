/*#pragma once

#include "Mesh/Mesh.h"

namespace vgl
{
	void computeHalfVertex(Vector3f v1, Vector3f v2, Vector3f& rs)
	{
		rs = v1 + v2;
		rs *= Math::magnitude(rs);
	}

	// Radius is one unit length
	void generateUVSphere(unsigned int p_Stacks, unsigned int p_Sectors, MeshData& p_MeshData)
	{
		std::vector<Vertex5T> vertices;
		std::vector<Vertex5T> vertices2;

		const float sectorStep = 2 * Math::PI / p_Sectors;
		const float stackStep = Math::PI / p_Stacks;
		float sectAngle, stackAngle;
		float xy = 0;
		float z = 0;
		float x, y;

		int k1, k2;

		for (int i = 0; i <= p_Stacks; ++i)
		{
			stackAngle = Math::PI / 2 - i * stackStep;
			xy = 1 * std::cos(stackAngle);
			z = 1 * std::sin(stackAngle);

			for (int j = 0; j <= p_Sectors; ++j)
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

		for (int i = 0; i < p_Stacks; ++i)
		{
			k1 = i * (p_Sectors + 1);
			k2 = k1 + p_Sectors + 1;

			for (int j = 0; j < p_Sectors; ++j, ++k1, ++k2)
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

		for (unsigned int i = 0; i < vertices.size() && i + 3 < vertices.size(); i += 3)
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
	void generateICOSphere(unsigned int p_Subdivisions, MeshData& p_MeshData)
	{
		std::vector<Vertex5T> vertices;
		std::vector<Vertex5T> vertices2;

		unsigned int index;

		const float H_Angle = Math::PI / 180 * 72;
		const float V_Angle = std::atanf(1.0f / 2);
		int i1 = 0, i2 = 0;
		float z, xy;
		float hAngle1 = -Math::PI / 2 - H_Angle / 2;
		float hAngle2 = -Math::PI / 2;

		vertices.resize(12);

		vertices[0].position = { 0, 0, 1 };

		for (int i = 1; i <= 5; ++i)
		{
			i1 = i;
			i2 = (i + 5);

			z = std::sinf(V_Angle);
			xy = std::cosf(V_Angle);

			vertices[i1].position = { xy * cosf(hAngle1), xy * sinf(hAngle1), z };
			vertices[i2].position = { xy * cosf(hAngle2), xy * sinf(hAngle2), -z };

			hAngle1 += H_Angle;
			hAngle2 += H_Angle;
		}

		vertices[11].position = { 0, 0, 1 };

		Vertex5T newV1;
		Vertex5T newV2;
		Vertex5T newV3;

		for (int i = 1; i < p_Subdivisions; i++)
		{
			vertices2 = vertices;
			vertices.clear();
			index = 0;

			for (int j = 0; j < vertices2.size(); j += 3)
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

		for (unsigned int i = 0; i < vertices.size() && i + 3 < vertices.size(); i += 3)
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
	void generateCUBESphere(unsigned int p_Subdivisions, MeshData& p_MeshData)
	{
		std::vector<Vertex5T> vertices;

		Vector3f n1;
		Vector3f n2;
		Vector3f v;
		float a1;
		float a2;

		Vector2f t1;

		int VPR = (int)std::pow(2, p_Subdivisions) + 1;

		float rot[6] = { 0, Math::PI / 4, Math::PI / 4, -Math::PI / 4, -Math::PI / 4, Math::PI / 2 };
		Vector2f rotN[6] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 0 }, { 0, 1 }, { 1, 0 } };

		//for (int k = 0; k < 6; k++) {
		for (unsigned int i = 0; i < VPR; ++i)
		{
			a2 = Math::DEG2RAD * (45 - 90 * i / (VPR - 1));
			n2 = { -std::sin(a2), std::cos(a2), 0 };

			t1.y = (float)i / (VPR - 1);

			for (unsigned int j = 0; j < VPR; ++j)
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

		for (int i = 0; i < vertices.size(); i += 3)
		{
			vertices[i].normal = Math::normalize(Math::cross(vertices[i + 1].position - vertices[i].position, vertices[i + 2].position - vertices[i].position));
			vertices[i + 1].normal = Math::normalize(Math::cross(vertices[i + 1].position - vertices[i].position, vertices[i + 2].position - vertices[i].position));
			vertices[i + 2].normal = Math::normalize(Math::cross(vertices[i + 1].position - vertices[i].position, vertices[i + 2].position - vertices[i].position));
		}

		for (unsigned int i = 0; i < vertices.size() && i + 3 < vertices.size(); i += 3)
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
}*/