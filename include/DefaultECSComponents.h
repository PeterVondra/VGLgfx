#pragma once

#include "ECS/ECS.h"
#include "GDefinitions.h"
#include "Math/Transform.h"
#include "Math/Camera.h"

#include "VGL-3D/Mesh/Mesh.h"
#include "VGL-3D/SkyBox.h"

namespace vgl
{
 	struct VGL_ECS_COMPONENT(EntityNameComponent)
	{
		EntityNameComponent(){}
	    EntityNameComponent(std::string p_Name) : Name(p_Name) {};
		std::string Name;
	};
  
	// Camera controller 3D component
	struct VGL_ECS_COMPONENT(CameraController3DComponent)
	{
		CameraController3D controller;
	};

	struct VGL_ECS_COMPONENT(CameraComponent)
	{
		Camera camera;
	};
	// Transform 3D component
	struct VGL_ECS_COMPONENT(Transform3DComponent)
	{
		Transform3D transform;
	};

	// Mesh 3D component
	struct VGL_ECS_COMPONENT(Mesh3DComponent)
	{
		MeshData* mesh = nullptr;
		uint32_t materialID = 0;
	};
	// Material component
	struct VGL_ECS_COMPONENT(MaterialComponent)
	{
		Material* material = nullptr;
		uint32_t materialID = 0;
	};

	struct VGL_ECS_COMPONENT(SkyboxComponent)
	{
		Skybox* skybox;
		std::string HDR_Image_Path;
		bool _AtmosphericScattering = false;
		AtmosphericScatteringInfo _AtmosphericScatteringInfo;
	};

	// Script component
	struct VGL_ECS_COMPONENT(ScriptComponent)
	{

	};
	
	// Shadow map component, added to LightSource entity
	struct VGL_ECS_COMPONENT(DShadowMapComponent)
	{
		D_ShadowMap ShadowMap;
	};
	
	// Shadow map component, added to LightSource entity
	struct VGL_ECS_COMPONENT(PShadowMapComponent)
	{
		//vk::P_ShadowMap ShadowMap;
	};
	
	// Directional Light 3D component, added to LightSource entity
	struct VGL_ECS_COMPONENT(DirectionalLight3DComponent)
	{
		Vector3f Direction;
		Vector3f Color;
		float VolumetricLightDensity = 1.0f;
	};
	
	// Point Light 3D component, added to LightSource entity
	struct VGL_ECS_COMPONENT(PointLight3DComponent)
	{
		Vector3f Position;
		Vector3f Color = Vector3f(1.0f);
		float Radius = 75;
		int32_t ShadowMapID = -1; // == -1/not using shadow map
		Matrix4f Transform;
	};
	
	// Spot Light 3D component, added to LightSource entity
	struct VGL_ECS_COMPONENT(SpotLight3DComponent)
	{

	};

}

/*
Basic Model entity
Transform3DComponent,
...
Material3DComponent[n],
...
*/
