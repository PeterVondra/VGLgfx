#include "Material.h"

namespace vgl
{
	Material::Material()
	{
			m_PrevConfig.m_Albedo = Vector3f(0.0f);
			m_PrevConfig.m_Metallic = 0.0f;
			m_PrevConfig.m_Roughness = 0.0f;
			m_PrevConfig.m_Ambient = 0.0f;
	}
	Material::Material(const Material& p)
	{
		*this = p;
	}
	Material::~Material()
	{

	}

	void Material::setName(const std::string& name)
	{
		m_Name = name;
	}
	
	std::string Material::getName()
	{
		return m_Name;
	}
}