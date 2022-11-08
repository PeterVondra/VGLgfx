#include "SkyBox.h"


namespace vgl
{
	Skybox::Skybox()
	{
		generateCube(*this);
		setScale(10000000);
	}
	Skybox::Skybox(ImageCube& p_CubeMap) : m_CubeMap(&p_CubeMap)
	{
		generateCube(*this);

		// Plus 16 for alignment
    m_MTLDescriptorInfo[0].p_VertexUniformBuffer = UniformBuffer(2*sizeof(Matrix4f)); 
    if(p_CubeMap.isValid())
      m_MTLDescriptorInfo[0].addImageCube(&p_CubeMap, 1);

		setScale(10000000);
	}
	void Skybox::create(ImageCube& p_CubeMap)
	{
		m_CubeMap = &p_CubeMap;

    if(m_MTLDescriptors[0].isValid())
      m_MTLDescriptors[0].destroy();  

    // Plus 16 for alignment
    m_MTLDescriptorInfo[0].p_VertexUniformBuffer = UniformBuffer(2*sizeof(Matrix4f)); 
    if(p_CubeMap.isValid())
      m_MTLDescriptorInfo[0].addImageCube(&p_CubeMap, 1);	

		setScale(10000000);
	}
	Skybox::~Skybox()
	{

	}

}
