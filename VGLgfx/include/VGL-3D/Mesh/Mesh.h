#pragma once

#include <iostream>

#include "../../Application/RenderAPI.h"// Possible dual inclusion (investage later)

#include "../../Math/Vector.h"
#include "../../Math/Transform.h"
#include "../../BufferLayout.h"

#include "Material.h"

namespace vgl
{
	//bool is_near(Vector3f& v1, Vector3f& v2) {
	//	return std::fabs(v1.x - v2.x) < 0.01f && std::fabs(v1.y - v2.y) < 0.01f && std::fabs(v1.z - v2.z) < 0.01f;
	//}
	//bool is_near(Vector2f& v1, Vector2f& v2) {
	//	return std::fabs(v1.x - v2.x) < 0.01f && std::fabs(v1.y - v2.y) < 0.01f;
	//}

	struct Vertex2T
	{
		Vertex2T() : position(0), normal(0) {};
		Vertex2T(Vector3f& p_Position, Vector3f& p_Normal) : position(p_Position), normal(p_Normal) {};
		Vertex2T(const Vertex2T& p_Vertex) {
			position = p_Vertex.position;
			normal = p_Vertex.normal;
		}
		~Vertex2T() {};

		bool operator<(const Vertex2T& v) const {
			return memcmp((void*)this, (void*)&v, sizeof(Vertex2T)) > 0;
		}

		Vector3f position;
		Vector3f normal;
	};
	struct Vertex3T
	{
		Vertex3T() : position(0), uv(0), normal(0) {};
		Vertex3T(Vector3f& p_Position, Vector2f& p_UV, Vector3f& p_Normal) : position(p_Position), uv(p_UV), normal(p_Normal) {};
		Vertex3T(const Vertex3T& p_Vertex) {
			position = p_Vertex.position;
			uv = p_Vertex.uv;
			normal = p_Vertex.normal;
		}
		~Vertex3T() {};

		bool operator<(const Vertex3T& v) const {
			return memcmp((void*)this, (void*)&v, sizeof(Vertex3T)) > 0;
		}

		Vector3f position;
		Vector2f uv;
		Vector3f normal;
	};
	struct Vertex5T
	{
		Vertex5T() : position(0), uv(0), normal(0), tangent(0), bitangent(0) {};
		Vertex5T(Vector3f p_Position, Vector2f p_UV, Vector3f p_Normal, Vector3f p_Tangent, Vector3f p_Bitangent)
			: position(p_Position), uv(p_UV), normal(p_Normal), tangent(p_Tangent), bitangent(p_Bitangent) {};
		Vertex5T(const Vertex5T& p_Vertex) {
			position = p_Vertex.position;
			uv = p_Vertex.uv;
			normal = p_Vertex.normal;
			tangent = p_Vertex.tangent;
			bitangent = p_Vertex.bitangent;
		}
		~Vertex5T() {};

		bool operator<(const Vertex5T& p_Vertex) const{
			return memcmp((void*)this, (void*)&p_Vertex, sizeof(Vertex5T)) > 0;
		} 

		Vector3f position;
		Vector2f uv;
		Vector3f normal;
		Vector3f tangent;
		Vector3f bitangent;
	};

	namespace vk
	{
		class Renderer;
		class BaseRenderer;
		class ForwardRenderer;
		class DeferredRenderer;
	}

	// Data needed for lightning in shaders
	struct EnvData
	{
		Image p_ShadowMap;
		ImageCube* p_IrradianceMap;
		ImageCube* p_PreFilteredMap;
		Image* p_BRDFLut;
	};

	enum class VertexLayout
	{
		PN = 0x01,		// Positions and Normals
		PNUV = 0x02,	// Position, Normals and Texture coordinates
		PNUVTB = 0x04	// Positions, Normals, Texture coordinates, Tangents and Bitangents
	};
	
	// Alignment for the uniform buffer
	struct MeshUBOAlignment
	{
		static const uint16_t mvp_offset = 0;
		static const uint16_t model_offset = 64;
		static const uint16_t viewPosition_offset = 128;
	};

	// Uses one buffer for all vertex data, materials are separated by indices
	class MeshData
	{
		public:
			MeshData() : vertices(std::vector<Vertex5T>()), indices(std::vector<uint32_t>()) {};
			~MeshData() {};

			void updateVertices(); // If the vertices(array) was modified, this function will update the GPU buffers
			void flush(); // Clears the vertices(array)
			void destroy(); // Flushes the vertices(array) and destroys the GPU buffers

			Material& getMaterial(uint32_t p_Index); // If p_Index > (num materials) - 1, p_Index = 0
			// p_Index = Material Index, p_Flag for rendering mesh of material (p_Flag = false -> mesh wont render)
			void render(const uint32_t p_Index, const bool p_Flag);
			bool& render(const uint32_t p_Index); // Get flag to check if mesh is rendering or not

			static void indexVertexData(std::vector<Vertex5T>& p_Vertices, MeshData& p_Data);
			static void indexVertexData(std::vector<Vertex3T>& p_Vertices, MeshData& p_Data);
			static void indexVertexData(std::vector<Vertex2T>& p_Vertices, MeshData& p_Data);

		public:
			
			std::vector<Vertex5T> vertices;
			std::vector<uint32_t> indices;
			std::vector<std::pair<uint32_t, uint32_t>> m_SubMeshIndices; // Materials

		public:
			std::string m_FileLocation; // File location of the 3D model file
      std::string m_FileName;
      std::string m_FileDirectory;

      bool m_TangentSpaceCorrectedUV = true;

			// GPU buffers
			VertexBuffer m_Vertices;
			IndexBuffer m_Indices;
			VertexArray m_VertexArray;

			Vector3f m_MaxPosition;
			Vector3f m_MinPosition;

			// If material is set to be recreated (eg. For any new changes to material)
			bool m_MTLRecreateFlag = false;
			// If mesh is set to be recreated (eg. For any new changes to the vertex data)
			bool m_RecreateFlag = false;

			void init(); // Initialize the necessary data

			// All indices correpsond for all vectors
			std::vector<Material> m_Materials;
		private:
			friend class OBJ_Loader;
			friend class Grid;
			friend class Skybox;
			friend class vk::Renderer;
			//friend class gl::Renderer;

			// If there is a MTL file (For OBJ file format)
			bool m_MTLValid = false;

			std::vector<ShaderDescriptorInfo> m_MTLDescriptorInfo;
			std::vector<ShaderDescriptor> m_MTLDescriptors;

			// Initialize the material info needed for rendering
			void initMaterials(/*EnvData& p_EnvData*/);

			VertexLayout m_VertexLayout;
	};
}
