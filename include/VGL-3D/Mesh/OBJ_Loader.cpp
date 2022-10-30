#include "OBJ_Loader.h"
#include "../../Platform/Definitions.h"

namespace vgl
{
	std::vector<unsigned int>
		* OBJ_Loader::m_PositionIndices = new std::vector<unsigned int>,
		* OBJ_Loader::m_UvIndices = new std::vector<unsigned int>,
		* OBJ_Loader::m_NormalIndices = new std::vector<unsigned int>;

	std::vector<Vector3f>* OBJ_Loader::temp_Positions = new std::vector<Vector3f>;
	std::vector<Vector2f>* OBJ_Loader::temp_Uvs = new std::vector<Vector2f>;
	std::vector<Vector3f>* OBJ_Loader::temp_Normals = new std::vector<Vector3f>;
	//Utils::ThreadPool OBJ_Loader::pool = Utils::ThreadPool(std::thread::hardware_concurrency());

	bool OBJ_Loader::loadModel(const char* p_Directory, const char* p_FileName, MeshData& p_MeshData, bool p_TangentSpaceUVCorrection)
	{
		p_MeshData.m_FileLocation = std::string(p_Directory) + std::string(p_FileName);

		for (auto& mtl : p_MeshData.m_MTLDescriptors)
			if (!mtl.isValid())
				mtl.destroy();

		p_MeshData.m_Materials.clear();
		p_MeshData.m_MTLDescriptors.clear();
		p_MeshData.m_SubMeshIndices.clear();

		bool result = loadOBJ
		(
			p_Directory,
			p_FileName,
			p_MeshData,
			p_TangentSpaceUVCorrection
		);
		if (result)
		{
			p_MeshData.init();
			//p_Model.initMaterials();
		}
		flush();
		return result;
	}
	bool OBJ_Loader::loadModel(const char* p_Directory, const char* p_FileName, MeshData* p_MeshData, bool p_TangentSpaceUVCorrection)
	{
		p_MeshData->m_FileLocation = (std::string)p_Directory + (std::string)p_FileName;

		for (auto& mtl : p_MeshData->m_MTLDescriptors)
			if(!mtl.isValid())
				mtl.destroy();

		p_MeshData->m_Materials.clear();
		p_MeshData->m_MTLDescriptors.clear();
		p_MeshData->m_SubMeshIndices.clear();

		bool result = loadOBJ
		(
			p_Directory,
			p_FileName,
			*p_MeshData,
			p_TangentSpaceUVCorrection
		);
		if (result)
		{
			p_MeshData->init();
			//p_Model->initMaterials();
		}
		flush();
		return result;
	}

	bool OBJ_Loader::loadModel(const char* p_Directory, const char* p_FileName, MeshData* p_MeshData, bool p_ignoreMaterialsInit, bool p_TangentSpaceUVCorrection)
	{
		bool result = loadOBJ
		(
			p_Directory,
			p_FileName,
			*p_MeshData,
			p_TangentSpaceUVCorrection
		);
		if (result)
		{
			p_MeshData->init();
			//if(!p_ignoreMaterialsInit)
			//	p_Model->initMaterials();
		}
		flush();
		return result;
	}

	bool OBJ_Loader::loadOBJ
	(
		const char* p_Directory,
		const char* p_FileName,
		MeshData& p_MeshData,
		bool p_TangentSpaceUVCorrection
	)
	{
		FILE* file = fopen(((std::string)p_Directory + (std::string)p_FileName).c_str(), "r");
		if (file == NULL) {
			Utils::Logger::logMSG("OBJ File not found: " + Utils::to_string(p_FileName), "OBJ Loader", Utils::Result::Failed);
			return false;
		}
		Utils::Logger::logMSG("OBJ File found: " + Utils::to_string(p_FileName), "OBJ Loader", Utils::Result::Success);

		int read_size = 0;

		// Look for size of file
		fseek(file, 0, SEEK_END);
		long file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		struct MTLIndexData
		{
			std::vector<unsigned int> pos;
			std::vector<unsigned int> uv;
			std::vector<unsigned int> normal;
		};

		std::vector<MTLIndexData> mtlVertexIndices;
		int materialIndex = 0;
		int faceCount = 0;

		if((float)file_size / 1000000 > 1.0f)
			Utils::Logger::logMSG("OBJ File: " + Utils::to_string(p_FileName) + " : " + Utils::to_string((float)file_size / 1000000) + " MB", "OBJ Loader", Utils::Severity::Info);
		else
			Utils::Logger::logMSG("OBJ File: " + Utils::to_string(p_FileName) + " : " + Utils::to_string((float)file_size / 1000) + " KB", "OBJ Loader", Utils::Severity::Info);

		bool mtlFound = false;

		float start = Utils::Logger::getTimePoint();

		// Find MTL file and load MTL data
		while (1) {
			char lineHeader[1000] = { 0 };
			if (fscanf(file, "%s", lineHeader) == EOF || mtlFound) {
				if (!mtlFound)
				{
					p_MeshData.m_Materials.resize(1);
					p_MeshData.m_SubMeshIndices.resize(1);
					p_MeshData.m_MTLDescriptors.resize(1);
					mtlVertexIndices.resize(1);

					materialIndex = 0;
				}

				break;
			}

			if (strcmp(lineHeader, "mtllib") == 0) {
				char mtllib[60];

				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, " %s \n", mtllib);

				MTL_Loader load;
				mtlFound = load.getMTLinfo(p_MeshData.m_Materials, p_Directory, p_FileName = ((std::string)p_Directory + (std::string)mtllib).c_str());

				if (!mtlFound)
				{
					p_MeshData.m_Materials.resize(1);
					p_MeshData.m_SubMeshIndices.resize(1);
					p_MeshData.m_MTLDescriptors.resize(1);
					mtlVertexIndices.resize(1);

					materialIndex = 0;
				}
				else
				{
					p_MeshData.m_SubMeshIndices.resize(p_MeshData.m_Materials.size());
					p_MeshData.m_MTLDescriptors.resize(p_MeshData.m_Materials.size());
					mtlVertexIndices.resize(p_MeshData.m_Materials.size());
				}

				break;
			}
		}

		Utils::Logger::logMSG("Execution time: " + Utils::to_string(Utils::Logger::getTimePoint() - start), "Material Loader", Utils::Severity::Debug);

		start = Utils::Logger::getTimePoint();

		fseek(file, 0, SEEK_SET);

		while (1) {
			char lineHeader[1000] = { 0 };
			if (fscanf(file, "%s", lineHeader) == EOF)
				break;
 
			if (strcmp(lineHeader, "v") == 0) {
				Vector3f position;

				fgets(lineHeader, sizeof(lineHeader), file);
				if (sscanf(lineHeader, " %f %f %f \n", &position.x, &position.y, &position.z) == 3)
					temp_Positions->emplace_back(position);
			}
			else if (strcmp(lineHeader, "vt") == 0) {
				Vector2f uv;

				fgets(lineHeader, sizeof(lineHeader), file);

				//If texture coordinate is a vector 2
				if (sscanf(lineHeader, " %f %f \n", &uv.x, &uv.y) == 2)
				{
					temp_Uvs->emplace_back(uv);
				}
			}
			else if (strcmp(lineHeader, "vn") == 0) {
				Vector3f normal;

				fgets(lineHeader, sizeof(lineHeader), file);
				if (sscanf(lineHeader, " %f %f %f \n", &normal.x, &normal.y, &normal.z) == 3)
					temp_Normals->emplace_back(normal);
			}

			else if (strcmp(lineHeader, "usemtl") == 0) {
				char mtlName[50];
				
				if (mtlFound)
				{
					fgets(lineHeader, sizeof(lineHeader), file);
					sscanf(lineHeader, " %s\n", mtlName);

					for (int i = 0; i < p_MeshData.m_Materials.size(); i++)
					{
						if (p_MeshData.m_Materials[i].getName() == (std::string)mtlName)
						{
							materialIndex = i;
							break;
						}
					}
				}
			}

			else if (strcmp(lineHeader, "f") == 0) {
				unsigned int positionIndex[3], uvIndex[3], normalIndex[3];
				unsigned int temp_PositionIndex, temp_NormalIndex, temp_UvIndex;

				fgets(lineHeader, sizeof(lineHeader), file);

				// ( 1 )  f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
				if (sscanf(lineHeader, " %d/%d/%d %d/%d/%d %d/%d/%d\n",
					&positionIndex[0], &uvIndex[0], &normalIndex[0],
					&positionIndex[1], &uvIndex[1], &normalIndex[1],
					&positionIndex[2], &uvIndex[2], &normalIndex[2]) == 9)
				{
					//Vertex 1
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[0]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[1]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[2]);

					//Vertex 2
					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[0]);
					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[1]);
					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[2]);

					//Vertex 3
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[0]);
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[1]);
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[2]);
				}

				// ( 2 )  no texture coordinates
				// ( 2 )  f v1//vn1 v2//vn2 v3//vn3
				if (sscanf(lineHeader, " %d//%d %d//%d %d//%d\n",
					&positionIndex[0], &normalIndex[0],
					&positionIndex[1], &normalIndex[1],
					&positionIndex[2], &normalIndex[2]) == 6)
				{

					//Vertex 1
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[0]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[1]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[2]);

					//Vertex 3
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[0]);
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[1]);
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[2]);
				}
				
				// ( 2.1 )  no normals
				// ( 2.1 )  f v1/vn1 v2/vn2 v3/vn3
				if (sscanf(lineHeader, " %d/%d %d/%d %d/%d\n",
					&positionIndex[0], &uvIndex[0],
					&positionIndex[1], &uvIndex[1],
					&positionIndex[2], &uvIndex[2]) == 6)
				{

					//Vertex 1
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[0]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[1]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[2]);

					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[0]);
					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[1]);
					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[2]);
				}

				// ( 3 )  only positions
				// ( 3 )  f v1 v2 v3
				if (sscanf(lineHeader, " %d %d %d\n",
					&positionIndex[0],
					&positionIndex[1],
					&positionIndex[2]) == 3)
				{

					//Vertex 1
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[0]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[1]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[2]);
				}

				// ( 4 )  convert quads to triangles
				// ( 4 )  f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 v4/vt4/vn4
				if (sscanf(lineHeader, " %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
					&positionIndex[0], &uvIndex[0], &normalIndex[0],
					&positionIndex[1], &uvIndex[1], &normalIndex[1],
					&positionIndex[2], &uvIndex[2], &normalIndex[2],
					&temp_PositionIndex, &temp_UvIndex, &temp_NormalIndex) == 12)
				{

					//vec4(1 2 3 4) = vec3(1 2 3) vec3(2 3 4)
					//Vertex 1
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[0]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[1]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[2]);
					
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[0]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[2]);
					mtlVertexIndices[materialIndex].pos.emplace_back(temp_PositionIndex);

					//Vertex 2
					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[0]);
					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[1]);
					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[2]);
					
					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[0]);
					mtlVertexIndices[materialIndex].uv.emplace_back(uvIndex[2]);
					mtlVertexIndices[materialIndex].uv.emplace_back(temp_UvIndex);

					//Vertex 3
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[0]);
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[1]);
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[2]);
					
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[0]);
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[2]);
					mtlVertexIndices[materialIndex].normal.emplace_back(temp_NormalIndex);
				}

				// ( 5 )  convert quads to triangles
				// ( 5 )  f v1//vn1 v2//vn2 v3//vn3 v4//vn4
				if (sscanf(lineHeader, " %d//%d %d//%d %d//%d %d//%d\n",
					&positionIndex[0], &normalIndex[0],
					&positionIndex[1], &normalIndex[1],
					&positionIndex[2], &normalIndex[2],
					&temp_PositionIndex, &temp_NormalIndex) == 8)
				{

					//vec4(1 2 3 4) = vec3(1 2 3) vec3(2 3 4)
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[0]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[1]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[2]);

					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[0]);
					mtlVertexIndices[materialIndex].pos.emplace_back(positionIndex[2]);
					mtlVertexIndices[materialIndex].pos.emplace_back(temp_PositionIndex);

					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[0]);
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[1]);
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[2]);

					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[0]);
					mtlVertexIndices[materialIndex].normal.emplace_back(normalIndex[2]);
					mtlVertexIndices[materialIndex].normal.emplace_back(temp_NormalIndex);
				}
			}
		}

		std::vector<Vertex5T> vertices;

		Vertex5T vertex;

		unsigned int first = 0;
		unsigned int last = 0;

		if (mtlVertexIndices.empty())
		{
			Utils::Logger::logMSG("No vertex positions in OBJ file", "OBJ Loader", Utils::Result::Failed);
			return false;
		}

		for (int j = 0; j < mtlVertexIndices.size(); j++)
		{
			first = last;
			last = first + mtlVertexIndices[j].pos.size();
			p_MeshData.m_SubMeshIndices[j] = std::pair<unsigned int, unsigned int>(first, last);
			
			for (int i = 0; i < mtlVertexIndices[j].pos.size(); i++)
			{
				vertex.position = temp_Positions->operator[](std::fabs(mtlVertexIndices[j].pos[i]) - 1);
				
				if (!mtlVertexIndices[j].uv.empty())
					vertex.uv = temp_Uvs->operator[](std::fabs(mtlVertexIndices[j].uv[i]) - 1);

				if (!mtlVertexIndices[j].normal.empty())
					vertex.normal = temp_Normals->operator[](std::fabs(mtlVertexIndices[j].normal[i]) - 1);

				vertices.emplace_back(vertex);
			}
		}

		if (!vertices.empty())
		{
			//calculate tangents
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

					if (p_TangentSpaceUVCorrection)
					{
						if (Math::dot(Math::cross(vertex.bitangent, vertex.tangent), vertex.normal) < 0.0f);
							vertex.bitangent *= -1.0f;
					}
				}

			}
			
			MeshData::indexVertexData(vertices, p_MeshData);

			Utils::Logger::logMSG("Execution time: " + Utils::to_string(Utils::Logger::getTimePoint() - start), "OBJ Loader", Utils::Severity::Debug);

			return true;
		}

		Utils::Logger::logMSG("Execution time: " + Utils::to_string(Utils::Logger::getTimePoint() - start), "OBJ Loader", Utils::Severity::Debug);
		Utils::Logger::logMSG("No vertex positions in OBJ file", "OBJ Loader", Utils::Result::Failed);

		return false;
	}

	void OBJ_Loader::flush()
	{

		temp_Positions->shrink_to_fit();
		temp_Uvs->shrink_to_fit();
		temp_Normals->shrink_to_fit();

		delete temp_Positions;
		delete temp_Uvs;
		delete temp_Normals;

		temp_Positions = new std::vector <Vector3f>;
		temp_Uvs = new std::vector<Vector2f>;
		temp_Normals = new std::vector<Vector3f>;
	}

	bool MTL_Loader::getMTLinfo
	(
		std::vector<Material>& materials,
		const char* texture_directory,
		const char* mtl_directory
	)
	{
		//open file
		FILE* file = fopen(mtl_directory, "r");
		if (file == NULL) {
			std::cout << "Unable to open file " << "(" << mtl_directory << ")" << std::endl;
			return false;
		}
		else {
			std::cout << "succesfully loaded " << "(" << mtl_directory << ")" << std::endl;
		}

		//has to be -1 for updating what material correlate with the next reading
		unsigned int material_index = -1;

		int read_size = 0;

		//get the file size
		fseek(file, 0, SEEK_END);
		int file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		Utils::Logger::logMSG("MTL File: " + std::string(mtl_directory) + " : " + Utils::to_string((float)file_size / 1000) + " KB", "Material Loader", Utils::Severity::Info);

		int i = 0;
		while (1) {
			//allocate sizeof(char) * 300 bytes for reading
			char lineHeader[1000];

			//if there is nothing else in this line, stop parsing
			if (fscanf(file, "%s", lineHeader) == EOF)
				break;

			read_size += strlen(lineHeader) + 1;

			if (strcmp(lineHeader, "newmtl") == 0) {
				char name[50];
				int char_size;
				//allocate memory for calculation
				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, " %s %n", name, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//create new material and put it in the vector
				material_index++;
				Material newMaterial;

				std::cout << "index : " << material_index << " name : " << name << std::endl;

				newMaterial.m_Name = name;
				materials.emplace_back(newMaterial);
			}
			else if (strcmp(lineHeader, "Ns") == 0) {
				float shine;
				int char_size;
				//allocate memory for reading
				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, "%f\n %n", &shine, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;
#undef max
				//put the "shininess" to material
				materials[material_index].config.m_Roughness = std::max(0.01f, shine);
			}
			else if (strcmp(lineHeader, "Ka") == 0) {
				Vector3f ambient;
				int char_size;

				//allocate memory for reading
				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, "%f %f %f\n %n", &ambient.x, &ambient.y, &ambient.z, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//put the read ambient color to material
				materials[material_index].config.m_Ambient = ambient.x;
			}
			else if (strcmp(lineHeader, "Kd") == 0) {
				Vector3f diffuse;
				int char_size;

				//allocate memory for reading
				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, "%f %f %f\n %n", &diffuse.x, &diffuse.y, &diffuse.z, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//put the read diffuse color to material
			}
			else if (strcmp(lineHeader, "Ks") == 0) {
				Vector3f specular;
				int char_size;
				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, "%f %f %f\n %n", &specular.x, &specular.y, &specular.z, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//put the read specular color to material
				materials[material_index].config.m_Metallic = specular.x + 0.01;
			}
			else if (/*strcmp(lineHeader, "map_Ka") == 0 ||*/ strcmp(lineHeader, "map_Kd") == 0) {
				char texture_path[200];
				int char_size;

				//allocate memory for reading
				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, " %s %n", texture_path, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//get the diffuse texture for the materials
				ImageLoader::getImageFromPath(materials[material_index].m_AlbedoMap, (texture_directory + std::string(texture_path)).c_str(), SamplerMode::Repeat);
				if (!materials[material_index].m_AlbedoMap.isValid())
					std::cerr << "failed to load diffuse texture" << std::endl;
			}
			else if (strcmp(lineHeader, "map_Ks") == 0) {
				char texture_path[200];
				int char_size;

				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, " %s %n", texture_path, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//get the specular texture for the materials
				vk::ImageLoader::getImageFromPath(materials[material_index].m_MetallicMap, (texture_directory + std::string(texture_path)).c_str(), SamplerMode::Repeat);
				if (!materials[material_index].m_MetallicMap.isValid())
					std::cerr << "failed to load specular texture" << std::endl;
			}
			else if (strcmp(lineHeader, "map_bump") == 0) {
				char texture_path[200];
				int char_size;

				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, " %s %n", texture_path, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//get the specular texture for the materials
				ImageLoader::getImageFromPath(materials[material_index].m_NormalMap, (texture_directory + std::string(texture_path)).c_str(), SamplerMode::Repeat);
				if (!materials[material_index].m_NormalMap.isValid())
					std::cerr << "failed to load normal texture" << texture_path << std::endl;
			}
			else if (strcmp(lineHeader, "map_Bump") == 0) {
				char texture_path[200];
				int char_size;

				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, " %s %n", texture_path, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//get the normal texture for the materials
				ImageLoader::getImageFromPath(materials[material_index].m_NormalMap, (texture_directory + std::string(texture_path)).c_str(), SamplerMode::Repeat);
				if (!materials[material_index].m_NormalMap.isValid())
					std::cerr << "failed to load normal texture" << texture_path << std::endl;
			}
			else if (strcmp(lineHeader, "map_disp") == 0) {
				char texture_path[200];
				int char_size;

				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, " %s %n", texture_path, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//get the normal texture for the materials
				ImageLoader::getImageFromPath(materials[material_index].m_DisplacementMap, (texture_directory + std::string(texture_path)).c_str(), SamplerMode::Repeat);
				if (!materials[material_index].m_DisplacementMap.isValid())
					std::cerr << "failed to load displacement texture" << texture_path << std::endl;
			}
			else if (strcmp(lineHeader, "map_Ao") == 0) {
				char texture_path[200];
				int char_size;

				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, " %s %n", texture_path, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//get the normal texture for the materials
				ImageLoader::getImageFromPath(materials[material_index].m_AOMap, (texture_directory + std::string(texture_path)).c_str(), SamplerMode::Repeat);
				if (!materials[material_index].m_AOMap.isValid())
					std::cerr << "failed to load ambient occlusion texture" << texture_path << std::endl;
			}
			else if (strcmp(lineHeader, "map_M") == 0) {
				char texture_path[200];
				int char_size;

				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, " %s %n", texture_path, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//get the normal texture for the materials
				ImageLoader::getImageFromPath(materials[material_index].m_MetallicMap, (texture_directory + std::string(texture_path)).c_str(), SamplerMode::Repeat);
				if (!materials[material_index].m_MetallicMap.isValid())
					std::cerr << "failed to load metallic texture" << texture_path << std::endl;
			}
			else if (strcmp(lineHeader, "map_R") == 0) {
				char texture_path[200];
				int char_size;

				fgets(lineHeader, sizeof(lineHeader), file);
				sscanf(lineHeader, " %s %n", texture_path, &char_size);

				//put byte size into read_size for progressbar calculation
				read_size += sizeof(char) * char_size;

				//get the normal texture for the materials
				ImageLoader::getImageFromPath(materials[material_index].m_RoughnessMap, (texture_directory + std::string(texture_path)).c_str(), SamplerMode::Repeat);
				if (!materials[material_index].m_RoughnessMap.isValid())
					std::cerr << "failed to load roughness texture" << texture_path << std::endl;
			}
		}
		return true;
	}
}
