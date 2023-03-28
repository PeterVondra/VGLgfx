#pragma once

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>

#include "Mesh.h"

namespace vgl
{
	enum class ImportOptions {

	};

	namespace {
		//MeshData processMesh(aiMesh* p_Mesh, aiScene* p_Scene)
		//{
		//	std::vector<Vertex5T> vertices;
		//	std::vector<uint32_t> indices;
		//	std::vector<vk::Image> textures;
		//
		//	for (uint32_t i = 0; i < p_Mesh->mNumVertices; i++) {
		//		Vertex5T vertex;
		//		vertex.position = { p_Mesh->mVertices[i].x, p_Mesh->mVertices[i].y, p_Mesh->mVertices[i].z };
		//		vertex.normal = { p_Mesh->mNormals[i].x, p_Mesh->mNormals[i].y, p_Mesh->mNormals[i].z };
		//
		//		if (p_Mesh->mTextureCoords[0])
		//			vertex.uv = { p_Mesh->mTextureCoords[0][i].x, p_Mesh->mTextureCoords[0][i].y };
		//
		//		vertices.push_back(vertex);
		//	}
		//
		//	for (uint32_t i = 0; i < p_Mesh->mNumFaces; i++)
		//	{
		//		aiFace face = p_Mesh->mFaces[i];
		//		for (uint32_t j = 0; j < face.mNumIndices; j++)
		//			indices.push_back(face.mIndices[j]);
		//	}
		//
		//	if (p_Mesh->mMaterialIndex >= 0)
		//	{
		//		aiMaterial* material = p_Scene->mMaterials[p_Mesh->mMaterialIndex];
		//
		//		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
		//			
		//		}
		//		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION); i++) {}
		//		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_SPECULAR); i++) {}
		//		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_EMISSIVE); i++) {}
		//		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_HEIGHT); i++) {}
		//		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_NORMALS); i++) {}
		//		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_SHININESS); i++) {}
		//		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_OPACITY); i++) {}
		//		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_DISPLACEMENT); i++) {}
		//		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_LIGHTMAP); i++) {}
		//	}
		//
		//	if (p_Mesh->mMaterialIndex >= 0) {
		//
		//	}
		//
		//	
		//}

		void processNode(aiNode* p_Node, const aiScene* p_Scene)
		{
			// process all the node's meshes (if any)
			for (uint32_t i = 0; i < p_Node->mNumMeshes; i++){
				aiMesh* mesh = p_Scene->mMeshes[p_Node->mMeshes[i]];
				//meshes.push_back(processMesh(mesh, p_Scene));
			}
			// then do the same for each of its children
			for (uint32_t i = 0; i < p_Node->mNumChildren; i++){
				processNode(p_Node->mChildren[i], p_Scene);
			}
		}
	}

	inline bool loadModel(std::string p_FilePath, MeshData& p_MeshData) {
		

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(p_FilePath, 
			aiProcess_Triangulate | 
			aiProcess_GenNormals | aiProcess_CalcTangentSpace | 
			aiProcess_GenSmoothNormals | aiProcess_ImproveCacheLocality | 
			aiProcess_GenUVCoords);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return false;
		}
		std::string directory = p_FilePath.substr(0, p_FilePath.find_last_of('/'));
		processNode(scene->mRootNode, scene);
	}
}