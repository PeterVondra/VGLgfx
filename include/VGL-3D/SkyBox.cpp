#include "SkyBox.h"


namespace vgl
{
	Skybox::Skybox()
	{
		generateCube(*this);
		setScale(10000000);
	}
	Skybox::Skybox(vk::CubeMap& p_CubeMap) : m_CubeMap(&p_CubeMap)
	{
		generateCube(*this);

		// Plus 16 for alignment
		m_MtlUniformManagers[0].setUniformBlock(vk::ShaderStage::VertexBit, "ubo", nullptr, 2 * sizeof(Matrix4f));
		m_MtlUniformManagers[0].setUniform("image", *m_CubeMap, 1);
		m_MtlUniformManagers[0].create();
		
		setScale(10000000);
	}
	void Skybox::create(vk::CubeMap& p_CubeMap)
	{
		m_CubeMap = &p_CubeMap;

		if (m_MtlUniformManagers[0].complete)
			m_MtlUniformManagers[0].destroy();

		m_MtlUniformManagers[0] = vk::UniformManager();

		// Plus 16 for alignment
		m_MtlUniformManagers[0].setUniformBlock(vk::ShaderStage::VertexBit, "ubo", nullptr, 2 * sizeof(Matrix4f));
		m_MtlUniformManagers[0].setUniform("image", *m_CubeMap, 1);
		m_MtlUniformManagers[0].create();

		setScale(10000000);
	}
	Skybox::~Skybox()
	{

	}

}