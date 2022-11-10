#pragma once

#include "VGL-Core/ECS/ECS.h"
#include "VGL-Core/Math/Transform.h"

#include "VGL-3D/Mesh/Mesh.h"

namespace vgl
{
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
		Transform3D transform;
	};

	// Camera 3D component
	struct VGL_ECS_COMPONENT(Camera3DComponent)
	{
		Camera camera;
	};

	// Script component
	struct VGL_ECS_COMPONENT(ScriptComponent)
	{

	};
	
	// Shadow map component, added to LightSource entity
<<<<<<< HEAD
	struct VGL_ECS_COMPONENT(DShadowMapComponent)
	{
		vk::D_ShadowMap ShadowMap;
	};
	
	// Shadow map component, added to LightSource entity
	struct VGL_ECS_COMPONENT(PShadowMapComponent)
	{
		vk::P_ShadowMap ShadowMap;
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
=======
	struct VGL_ECS_COMPONENT(ShadowMapComponent)
>>>>>>> parent of e6feae2 (Rendering available)
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