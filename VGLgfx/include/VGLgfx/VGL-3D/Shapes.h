#pragma once

#include "Mesh/Mesh.h"

namespace vgl
{
	void computeHalfVertex(Vector3f v1, Vector3f v2, Vector3f& rs);

	// Radius is one unit length
	void generateUVSphere(uint32_t p_Stacks, uint32_t p_Sectors, MeshData& p_MeshData);
	// Radius is one unit length
	void generateICOSphere(uint32_t p_Subdivisions, MeshData& p_MeshData);
	// Radius is one unit length
	void generateCUBESphere(uint32_t p_Subdivisions, MeshData& p_MeshData);

	// Size is 1, origin is at {0, 0, 0} in model space
	void generateCube(MeshData& p_MeshData);

	void generateGrid(const uint32_t p_QuadCountX, const uint32_t p_QuadCountY, const Vector2f p_QuadSize, MeshData& p_MeshData);
}