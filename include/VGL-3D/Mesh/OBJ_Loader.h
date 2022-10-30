#pragma once

#include <fstream>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <string.h>

#include "Mesh.h"

namespace vgl
{
	class OBJ_Loader
	{
		public:
			static bool loadModel(const char* p_Directory, const char* p_FileName, MeshData& p_MeshData, bool p_TangentSpaceUVCorrection);
			static bool loadModel(const char* p_Directory, const char* p_FileName, MeshData* p_MeshData, bool p_TangentSpaceUVCorrection);
			
		protected:
		private:
			friend class Skybox;

			//static Utils::ThreadPool pool;

			static bool loadModel(const char* p_Directory, const char* p_FileName, MeshData* p_Data, bool p_ignoreMaterialsInit, bool p_TangentSpaceUVCorrection);

			static bool loadOBJ
			(
				const char* p_Directory,
				const char* p_FileName,
				MeshData& p_MeshData,
				bool p_TangentSpaceUVCorrection
			);

			static void flush();

			static std::vector<unsigned int> *m_PositionIndices, *m_UvIndices, *m_NormalIndices;

			static std::vector<Vector3f>* temp_Positions;
			static std::vector<Vector2f>* temp_Uvs;
			static std::vector<Vector3f>* temp_Normals;
	};

	class MTL_Loader
	{
		public:

			static bool getMTLinfo
			(
				std::vector<Material>& materials,
				const char* texture_directory,
				const char* mtl_directory
			);

		protected:
		private:
	};
}
