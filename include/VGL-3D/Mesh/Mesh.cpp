#include "Mesh.h"

namespace vgl
{
	void MeshData::init()
	{
		if (m_Materials.size() == 0)
		{
			m_Materials.resize(1);
			m_MaterialIndices.resize(1);
			m_MtlDescriptors.resize(1);

			m_MaterialIndices[0].first = 0;
			m_MaterialIndices[0].second = indices.size();
		}

		BufferLayout layout = { {
				{ ShaderDataType::Vec3f, 0, "inPosition"  },
				{ ShaderDataType::Vec2f, 1, "inUv"		  },
				{ ShaderDataType::Vec3f, 2, "inNormal"    },
				{ ShaderDataType::Vec3f, 3, "inTangent"   },
				{ ShaderDataType::Vec3f, 4, "inBitangent" }
			},0
		};

		m_Vertices.setLayout(layout);
		m_Vertices.fill(vertices5T);
		m_Indices.fill(indices);
		m_VertexArray.fill(m_Vertices, m_Indices);
	}
	void MeshData::updateVertices()
	{
		m_Vertices.update(verticesf);
		m_Indices.update(indices);
	}
	void MeshData::initMaterials(EnvData& p_EnvData)
	{
		m_MTLCompleted = true;

		if (m_Materials.size() == 0)
		{
			m_Materials.resize(1);
			m_MaterialIndices.resize(1);
			m_MtlDescriptors.resize(1);

			m_MaterialIndices[0].first = 0;
			m_MaterialIndices[0].second = indices.size();

			m_Materials[0].config.m_Albedo = Vector3f(1.0f);
			m_Materials[0].config.m_Metallic = 1.0f;
			m_Materials[0].config.m_Roughness = 1.0f;
			m_Materials[0].config.m_Ambient = 1.0f;
		}

		for (int i = 0; i < m_Materials.size(); i++)
		{
			m_Materials[i].config.m_Ambient = 1.0f;

			// Plus 16 for alignment
			m_MtlDescriptors[i].setUniformBlock(ShaderStage::VertexBit, "ubo", nullptr, 3 * sizeof(Matrix4f) + 16);
			m_MtlDescriptors[i].setUniformBlock(ShaderStage::FragmentBit, "ubo", nullptr, 14 * sizeof(float) );

			ShaderInfo& info = m_Materials[i].m_ShaderInfo;
			info.p_LightCaster = (uint32_t)LightCaster::Direction;
			info.p_LightingType = (uint32_t)LightingType::PBR;
			info.p_LightMaps = 0;
			info.p_Effects = 0;

			if (m_Materials[i].m_AlbedoMap.isComplete())
			{
				m_MtlUniformManagers[i].setUniform("image", m_Materials[i].m_AlbedoMap, 2);
				info.p_LightMaps |= (uint32_t)LightMaps::Albedo;
			}

			if (m_Materials[i].m_MetallicMap.isComplete())
			{
				m_MtlUniformManagers[i].setUniform("image", m_Materials[i].m_MetallicMap, 3);
				info.p_LightMaps |= (uint32_t)LightMaps::Metallic;
			}

			if (m_Materials[i].m_RoughnessMap.isComplete())
			{
				m_MtlUniformManagers[i].setUniform("image", m_Materials[i].m_RoughnessMap, 4);
				info.p_LightMaps |= (uint32_t)LightMaps::Roughness;
			}

			if (m_Materials[i].m_AOMap.isComplete())
			{
				m_MtlUniformManagers[i].setUniform("image", m_Materials[i].m_AOMap, 5);
				info.p_LightMaps |= (uint32_t)LightMaps::AO;
			}

			if (m_Materials[i].m_NormalMap.isComplete())
			{
				m_MtlUniformManagers[i].setUniform("image", m_Materials[i].m_NormalMap, 6);
				info.p_Effects |= (uint32_t)Effects::NormalMapping;
			}

			if (m_Materials[i].m_DisplacementMap.isComplete())
			{
				m_MtlUniformManagers[i].setUniform("imageDisp", m_Materials[i].m_DisplacementMap, 7);
				info.p_Effects |= (uint32_t)Effects::POM;
			}

			// Directional light shadow map
			//for(int32_t k = 0; k < p_EnvData.p_DShadowMap.size(); k++) // k = swapchain image index
			//	if (p_EnvData.p_DShadowMap[k]->isComplete()) {
			//		m_MtlUniformInfos[i].p_ImageDescriptors.emplace_back(k, p_EnvData.p_DShadowMap[k], binding, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
			//		info.p_Effects |= (uint32_t)Effects::ShadowMapping;
			//	}
			//binding++;
			//
			//// Spot light shadow maps
			//for (auto& shadow_map : p_EnvData.p_SShadowMap) {
			//	for (int32_t k = 0; k < shadow_map.size(); k++) // k = swapchain image index
			//		if (shadow_map[k]->isComplete()) {
			//			m_MtlUniformInfos[i].p_ImageDescriptors.emplace_back(k, shadow_map[k], binding, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
			//			info.p_Effects |= (uint32_t)Effects::ShadowMapping;
			//		}
			//	binding++;
			//}
			//
			//// Point light shadow maps
			//for (auto& shadow_cubemap : p_EnvData.p_PShadowMap) {
			//	for (int32_t k = 0; k < shadow_cubemap.size(); k++) // k = swapchain image index
			//		if (shadow_cubemap[k]->isComplete()) {
			//			m_MtlUniformInfos[i].p_CubeMapDescriptors.emplace_back(k, shadow_cubemap[k], binding, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
			//			info.p_Effects |= (uint32_t)Effects::ShadowMapping;
			//		}
			//	binding++;
			//}
			
			if(p_EnvData.p_IrradianceMap && p_EnvData.p_PrefilteredMap && p_EnvData.p_BRDFLut)
				if (p_EnvData.p_IrradianceMap->isComplete() && p_EnvData.p_PrefilteredMap->isComplete() && p_EnvData.p_BRDFLut->isComplete()) {
					m_MtlUniformInfos[i].p_CubeMapDescriptors.emplace_back(p_EnvData.p_IrradianceMap, binding);
					binding++;
					m_MtlUniformInfos[i].p_CubeMapDescriptors.emplace_back(p_EnvData.p_PrefilteredMap, binding);

					m_Materials[i].m_ShaderInfo.p_MaxPrefilteredLod = p_EnvData.p_PrefilteredMap->getMipLevels();

					binding++;
					m_MtlUniformInfos[i].p_ImageDescriptors.emplace_back(p_EnvData.p_BRDFLut, binding);
					binding++;
					info.p_Effects |= (uint32_t)Effects::IBL;
				}

			m_MtlUniformManagers[i].create();
		}
	}

	Material& MeshData::getMaterial(uint32_t index)
	{
		if (m_Materials.size() > 0)
			return m_Materials[(index >= m_Materials.size() || index < 0) ? 0 : index];
	}
	void MeshData::render(const uint32_t index, const bool p_Flag)
	{
		if (m_Materials.size() > 0)
			m_Materials[(index >= m_Materials.size() || index < 0) ? 0 : index].config.render = p_Flag;
	}
	bool& MeshData::render(const uint32_t index)
	{
		if (m_Materials.size() > 0)
			return m_Materials[(index >= m_Materials.size() || index < 0) ? 0 : index].config.render;
	}

	void MeshData::flush()
	{
		indices.clear();
		verticesf.clear();
		m_Vertices.destroy();
		m_Indices.destroy();
	}

	void MeshData::destroy()
	{
		for (int i = 0; i < m_MtlUniformManagers.size(); i++)
		{
			m_MtlUniformManagers[i].destroy();

			if (m_Materials[i].m_AlbedoMap.isComplete())
				m_Materials[i].m_AlbedoMap.destroy();

			if (m_Materials[i].m_NormalMap.isComplete())
				m_Materials[i].m_NormalMap.destroy();

			if (m_Materials[i].m_DisplacementMap.isComplete())
				m_Materials[i].m_DisplacementMap.destroy();

			if (m_Materials[i].m_MetallicMap.isComplete())
				m_Materials[i].m_MetallicMap.destroy();

			if (m_Materials[i].m_RoughnessMap.isComplete())
				m_Materials[i].m_RoughnessMap.destroy();

			if (m_Materials[i].m_AOMap.isComplete())
				m_Materials[i].m_AOMap.destroy();
		}

		flush();
	}

	void MeshData::indexVertexData(std::vector<Vertex5T>& p_Vertices, MeshData& p_Data)
	{
		std::map<Vertex5T, uint32_t> vertexToIndex;

		std::map<Vertex5T, uint32_t>::iterator it;
		uint32_t index;
		for (uint32_t i = 0; i < p_Vertices.size(); i++)
		{
			it = vertexToIndex.find(p_Vertices[i]);
			if (it != vertexToIndex.end())
			{
				index = it->second;

				p_Data.vertices5T[index].tangent += p_Vertices[i].tangent;
				p_Data.vertices5T[index].normal += p_Vertices[i].normal;
				p_Data.vertices5T[index].bitangent += p_Vertices[i].bitangent;
				p_Data.indices.push_back(index);
				continue;
			}
			p_Data.vertices5T.push_back(p_Vertices[i]);

			p_Data.indices.push_back(p_Data.vertices5T.size() - 1);
			vertexToIndex[p_Vertices[i]] = p_Data.vertices5T.size() - 1;
		}
	}
	void MeshData::indexVertexData(std::vector<Vertex3T>& p_Vertices, MeshData& p_Data)
	{
		std::map<Vertex3T, uint32_t> vertexToIndex;

		std::map<Vertex3T, uint32_t>::iterator it;
		uint32_t index;
		for (uint32_t i = 0; i < p_Vertices.size(); i++)
		{
			it = vertexToIndex.find(p_Vertices[i]);
			if (it != vertexToIndex.end())
			{
				index = it->second;

				p_Data.vertices3T[index].normal += p_Vertices[i].normal;
				p_Data.indices.push_back(index);
				continue;
			}
			p_Data.vertices3T.push_back(p_Vertices[i]);

			p_Data.indices.push_back(p_Data.vertices3T.size() - 1);
			vertexToIndex[p_Vertices[i]] = p_Data.vertices3T.size() - 1;
		}
	}
	void MeshData::indexVertexData(std::vector<Vertex2T>& p_Vertices, MeshData& p_Data)
	{
		std::map<Vertex2T, uint32_t> vertexToIndex;

		std::map<Vertex2T, uint32_t>::iterator it;
		uint32_t index;
		for (uint32_t i = 0; i < p_Vertices.size(); i++)
		{
			it = vertexToIndex.find(p_Vertices[i]);
			if (it != vertexToIndex.end())
			{
				index = it->second;

				p_Data.vertices2T[index].normal += p_Vertices[i].normal;
				p_Data.indices.push_back(index);
				continue;
			}
			p_Data.vertices2T.push_back(p_Vertices[i]);

			p_Data.indices.push_back(p_Data.vertices2T.size() - 1);
			vertexToIndex[p_Vertices[i]] = p_Data.vertices2T.size() - 1;
		}
	}
}