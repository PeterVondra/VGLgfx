#include "SkyBox.h"


namespace vgl
{
	Skybox::Skybox()
	{
		generateCube(*this);

		create();

		setScale(10000000);
	}
	Skybox::Skybox(ImageCube& p_CubeMap) : m_CubeMap(&p_CubeMap)
	{
		generateCube(*this);

		create(p_CubeMap);
	}
	void Skybox::create()
	{
		if (m_MTLDescriptors[0].isValid())
			m_MTLDescriptors[0].destroy();

		// Plus 16 for alignment
		m_MTLDescriptorInfo[0].p_VertexUniformBuffer = UniformBuffer(sizeof(Matrix4f));
		m_MTLDescriptors[0].create(m_MTLDescriptorInfo[0]);

		setScale(10000000);
	}
	void Skybox::create(ImageCube& p_CubeMap)
	{
		m_CubeMap = &p_CubeMap;

		if(m_MTLDescriptors[0].isValid())
		  m_MTLDescriptors[0].destroy();  

		// Plus 16 for alignment
		m_MTLDescriptorInfo[0].p_VertexUniformBuffer = UniformBuffer(sizeof(Matrix4f)); 
		if(p_CubeMap.isValid())
			m_MTLDescriptorInfo[0].addImageCube(&p_CubeMap, 1);	
		m_MTLDescriptors[0].create(m_MTLDescriptorInfo[0]);

		setScale(10000000);
	}
	Skybox::~Skybox()
	{

	}

}
