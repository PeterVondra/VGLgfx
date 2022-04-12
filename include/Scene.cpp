#include "Scene.h"

#include <fstream>

#include "DefaultECSComponents.h"
#include "VGL-3D/Mesh/OBJ_Loader.h"

namespace YAML
{
	template<> struct convert<Vector2f>
	{
		static Node encode(const Vector2f& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}
		static bool decode(const Node& node, Vector2f& rhs) {
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};
	template<> struct convert<Vector3f>
	{
		static Node encode(const Vector3f& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}
		static bool decode(const Node& node, Vector3f& rhs) {
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};
	template<> struct convert<Vector4f>
	{
		static Node encode(const Vector4f& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}
		static bool decode(const Node& node, Vector4f& rhs) {
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
	template<> struct convert<Matrix4f>
	{
		static Node encode(const Matrix4f& rhs) {
			Node node;

			node.push_back(rhs[0]);
			node.push_back(rhs[1]);
			node.push_back(rhs[2]);
			node.push_back(rhs[3]);
			node.push_back(rhs[4]);
			node.push_back(rhs[5]);
			node.push_back(rhs[6]);
			node.push_back(rhs[7]);
			node.push_back(rhs[8]);
			node.push_back(rhs[9]);
			node.push_back(rhs[10]);
			node.push_back(rhs[11]);
			node.push_back(rhs[12]);
			node.push_back(rhs[13]);
			node.push_back(rhs[14]);
			node.push_back(rhs[15]);

			return node;
		}
		static bool decode(const Node& node, Matrix4f& rhs) {
			if (!node.IsSequence() || node.size() != 16)
				return false;

			rhs[0] = node[0].as<float>();
			rhs[1] = node[1].as<float>();
			rhs[2] = node[2].as<float>();
			rhs[3] = node[3].as<float>();
			rhs[4] = node[4].as<float>();
			rhs[5] = node[5].as<float>();
			rhs[6] = node[6].as<float>();
			rhs[7] = node[7].as<float>();
			rhs[8] = node[8].as<float>();
			rhs[9] = node[9].as<float>();
			rhs[10] = node[10].as<float>();
			rhs[11] = node[11].as<float>();
			rhs[12] = node[12].as<float>();
			rhs[13] = node[13].as<float>();
			rhs[14] = node[14].as<float>();
			rhs[15] = node[15].as<float>();

			return true;
		}
	};
}

namespace vgl
{
	YAML::Emitter& operator<<(YAML::Emitter& p_Emitter, const Vector2f& p_Vector)
	{
		p_Emitter << YAML::Flow;
		p_Emitter << YAML::BeginSeq << p_Vector.x << p_Vector.y << YAML::EndSeq;
		return p_Emitter;
	}
	YAML::Emitter& operator<<(YAML::Emitter& p_Emitter, const Vector3f& p_Vector)
	{
		p_Emitter << YAML::Flow;
		p_Emitter << YAML::BeginSeq << p_Vector.x << p_Vector.y << p_Vector.z << YAML::EndSeq;
		return p_Emitter;
	}
	YAML::Emitter& operator<<(YAML::Emitter& p_Emitter, const Vector4f& p_Vector)
	{
		p_Emitter << YAML::Flow;
		p_Emitter << YAML::BeginSeq << p_Vector.x << p_Vector.y << p_Vector.z << p_Vector.w << YAML::EndSeq;
		return p_Emitter;
	}
	YAML::Emitter& operator<<(YAML::Emitter& p_Emitter, const Matrix4f& p_Matrix)
	{
		p_Emitter << YAML::Flow;
		p_Emitter << YAML::BeginSeq;
		for(uint32_t i = 0; i < 16; i++)
			p_Emitter << p_Matrix[i];
		p_Emitter << YAML::EndSeq;
		return p_Emitter;
	}

	bool Scene::serialize_entity(ecs::EntityHandle& p_Handle, YAML::Emitter& p_Emitter)
	{
		EntityNameComponent* name = getComponent<EntityNameComponent>(p_Handle);
		Mesh3DComponent* mesh = getComponent<Mesh3DComponent>(p_Handle);
		Transform3DComponent* transform = getComponent<Transform3DComponent>(p_Handle);
		Camera3DComponent* camera = getComponent<Camera3DComponent>(p_Handle);
		CameraController3DComponent* cameraController = getComponent<CameraController3DComponent>(p_Handle);
		SkyboxComponent* skybox = getComponent<SkyboxComponent>(p_Handle);
		DirectionalLight3DComponent* directional_light = getComponent<DirectionalLight3DComponent>(p_Handle);
		DShadowMapComponent* d_shadow_map = getComponent<DShadowMapComponent>(p_Handle);

		p_Emitter << YAML::BeginMap; // Entity
		p_Emitter << YAML::Key << "Entity" << YAML::Value << name->Name;

		if (mesh != nullptr) {
			if (mesh->mesh != nullptr) {
				p_Emitter << YAML::Key << "Mesh3DComponent";
				p_Emitter << YAML::BeginMap; // Mesh3DComponent

				p_Emitter << YAML::Key << "3D_ModelDirectory" << YAML::Value << mesh->mesh->m_FileDirectory;
				p_Emitter << YAML::Key << "3D_ModelName" << YAML::Value << mesh->mesh->m_FileName;
				p_Emitter << YAML::Key << "TangentSpaceCorrectedUV" << YAML::Value << mesh->mesh->m_TangentSpaceCorrectedUV;

				p_Emitter << YAML::Key << "Materials" << YAML::Value << YAML::BeginSeq;
				for (uint32_t i = 0; i < mesh->mesh->m_MaterialIndices.size(); i++) {
					p_Emitter << YAML::BeginMap; // Material
					p_Emitter << YAML::Key << "Material" << YAML::Value << mesh->mesh->getMaterial(i).m_Name;
					p_Emitter << YAML::Key << "Albedo" << YAML::Value << mesh->mesh->getMaterial(i).config.m_Albedo;
					p_Emitter << YAML::Key << "Ambient" << YAML::Value << mesh->mesh->getMaterial(i).config.m_Ambient;
					p_Emitter << YAML::Key << "Metallic" << YAML::Value << mesh->mesh->getMaterial(i).config.m_Metallic;
					p_Emitter << YAML::Key << "PDMDepth" << YAML::Value << mesh->mesh->getMaterial(i).config.m_PDMDepth;
					p_Emitter << YAML::Key << "Roughness" << YAML::Value << mesh->mesh->getMaterial(i).config.m_Roughness;
					p_Emitter << YAML::Key << "Render" << YAML::Value << mesh->mesh->getMaterial(i).config.render;
					p_Emitter << YAML::EndMap; // Material
				}
				p_Emitter << YAML::EndSeq; // Mesh3DComponent
				p_Emitter << YAML::EndMap; // Mesh3DComponent
			}
		}
		
		if (skybox != nullptr) {
			p_Emitter << YAML::Key << "SkyBoxComponent";
			p_Emitter << YAML::BeginMap; // SkyBoxComponent

			p_Emitter << YAML::Key << "Atmospheric_Scattering" << YAML::Value << skybox->AtmosphericScattering;
			p_Emitter << YAML::Key << "HDR_Image_Path" << YAML::Value << skybox->HDR_Image_Path;

			p_Emitter << YAML::Key << "Atmosphere_Info";
			p_Emitter << YAML::BeginMap; // Atmosphere
			p_Emitter << YAML::Key << "Ray_Origin" << YAML::Value << skybox->AtmosphericScatteringInfo.p_RayOrigin;
			p_Emitter << YAML::Key << "Sun_Position" << YAML::Value << skybox->AtmosphericScatteringInfo.p_SunPos;
			p_Emitter << YAML::Key << "Sun_Intensity" << YAML::Value << skybox->AtmosphericScatteringInfo.p_SunIntensity;
			p_Emitter << YAML::Key << "Planet_Radius" << YAML::Value << skybox->AtmosphericScatteringInfo.p_PlanetRadius;
			p_Emitter << YAML::Key << "Atmosphere_Radius" << YAML::Value << skybox->AtmosphericScatteringInfo.p_AtmosphereRadius;
			p_Emitter << YAML::Key << "Rayleigh_Scale" << YAML::Value << skybox->AtmosphericScatteringInfo.p_RayleighSC;
			p_Emitter << YAML::Key << "Mie_Scale" << YAML::Value << skybox->AtmosphericScatteringInfo.p_MieSC;
			p_Emitter << YAML::Key << "Rayleigh_Scale_Height" << YAML::Value << skybox->AtmosphericScatteringInfo.p_RayleighSHeight;
			p_Emitter << YAML::Key << "Mie_Scale_Height" << YAML::Value << skybox->AtmosphericScatteringInfo.p_MieSHeight;
			p_Emitter << YAML::Key << "Mie_Scatter_Scale" << YAML::Value << skybox->AtmosphericScatteringInfo.p_MieDir;
			p_Emitter << YAML::Key << "Scale" << YAML::Value << skybox->AtmosphericScatteringInfo.p_Scale;
			p_Emitter << YAML::EndMap; // Atmosphere

			p_Emitter << YAML::EndMap; // SkyBoxComponent
		}
		
		if (transform != nullptr) {
			p_Emitter << YAML::Key << "Transform3DComponent";
			p_Emitter << YAML::BeginMap; // Transform3DComponent
			p_Emitter << YAML::Key << "Position" << YAML::Value << transform->transform.getPosition();
			p_Emitter << YAML::Key << "Rotation" << YAML::Value << transform->transform.getRotation();
			p_Emitter << YAML::Key << "Scale" << YAML::Value << transform->transform.getScalar();
			p_Emitter << YAML::Key << "Model" << YAML::Value << transform->transform.model;
			p_Emitter << YAML::EndMap; // Transform3DComponent
		}
		
		if (d_shadow_map != nullptr) {
			p_Emitter << YAML::Key << "DShadowMapComponent";
			p_Emitter << YAML::BeginMap; // DShadowMapComponent

			p_Emitter << YAML::Key << "Resolution" << YAML::Value << d_shadow_map->ShadowMap.m_Resolution.getVec2f();
			p_Emitter << YAML::Key << "Direction" << YAML::Value << d_shadow_map->ShadowMap.m_Direction;
			p_Emitter << YAML::Key << "DepthBiasConstant" << YAML::Value << d_shadow_map->ShadowMap.m_DepthBiasConstant;
			p_Emitter << YAML::Key << "DepthBiasSlope" << YAML::Value << d_shadow_map->ShadowMap.m_DepthBiasSlope;
			p_Emitter << YAML::Key << "View_Matrix" << YAML::Value << d_shadow_map->ShadowMap.m_View;
			p_Emitter << YAML::Key << "Projection_Matrix" << YAML::Value << d_shadow_map->ShadowMap.m_Projection;

			p_Emitter << YAML::EndMap; // DShadowMapComponent
		}
		
		if (directional_light != nullptr) {
			p_Emitter << YAML::Key << "DirectionalLightComponent";
			p_Emitter << YAML::BeginMap; // Directional_Light
			p_Emitter << YAML::Key << "Direction" << YAML::Value << directional_light->Direction;
			p_Emitter << YAML::Key << "Color" << YAML::Value << directional_light->Color;
			p_Emitter << YAML::EndMap; // Directional_Light
		}
		
		if (camera != nullptr) {
			p_Emitter << YAML::Key << "CameraComponent";
			p_Emitter << YAML::BeginMap; // CameraComponent
			p_Emitter << YAML::Key << "ViewType" << YAML::Value << (uint32_t)camera->camera.getView();
			p_Emitter << YAML::Key << "ViewModZ" << YAML::Value << camera->camera.getViewModifier();
			p_Emitter << YAML::Key << "PerspectiveFOV" << YAML::Value << camera->camera.getFieldOfView();
			p_Emitter << YAML::Key << "AspectRatio" << YAML::Value << camera->camera.getAspectRatio();
			p_Emitter << YAML::Key << "Position" << YAML::Value << camera->camera.getPosition();
			p_Emitter << YAML::Key << "Rotation" << YAML::Value << camera->camera.getRotation();
			p_Emitter << YAML::Key << "ViewDirection" << YAML::Value << camera->camera.getViewDirection();
			p_Emitter << YAML::Key << "PerspectiveMatrix" << YAML::Value << camera->camera.getPerspectiveMatrix();
			p_Emitter << YAML::Key << "OrthographicMatrix" << YAML::Value << camera->camera.getOrthographicMatrix();
			p_Emitter << YAML::Key << "ViewMatrix" << YAML::Value << camera->camera.getViewMatrix();
			p_Emitter << YAML::EndMap; // CameraComponent
		}
		
		if (cameraController != nullptr) {
			p_Emitter << YAML::Key << "CameraController3DComponent";
			p_Emitter << YAML::BeginMap; // CameraController3DComponent
			p_Emitter << YAML::Key << "Enabled" << YAML::Value << cameraController->controller.isEnabled();
			p_Emitter << YAML::Key << "MovementSpeed" << YAML::Value << cameraController->controller.m_MovementSpeed;
			p_Emitter << YAML::Key << "Sensitivity" << YAML::Value << cameraController->controller.getSensitivity();
			p_Emitter << YAML::Key << "Zoom" << YAML::Value << cameraController->controller.getZoom();
			p_Emitter << YAML::Key << "Yaw" << YAML::Value << cameraController->controller.getYaw();
			p_Emitter << YAML::Key << "Pitch" << YAML::Value << cameraController->controller.getPitch();
			p_Emitter << YAML::Key << "PitchConstrain" << YAML::Value << cameraController->controller.isPitchConstrained();
			p_Emitter << YAML::EndMap; // CameraController3DComponent
		}

		p_Emitter << YAML::EndMap; // Entity
		
		return true;
	}

	bool Scene::serialize_yaml(const std::string& p_FilePath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		for (ecs::EntityHandle entity : getEntities()) {
			if (!entity)
				continue;

			serialize_entity(entity, out);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(p_FilePath);
		fout << out.c_str();

		return true;
	}
	bool Scene::serialize_bin(const std::string& p_FilePath)
	{
		return false;
	}
	bool Scene::deserialize_yaml(const std::string& p_FilePath)
	{
		for (auto& handle : getEntities()) {
			auto mesh = getComponent<Mesh3DComponent>(handle);
			auto shadowMap = getComponent<DShadowMapComponent>(handle);
			
			if(shadowMap) shadowMap->ShadowMap.destroy();

			if(mesh) if(mesh->mesh) mesh->mesh->destroy();

			deleteEntity(handle);
		}

		std::ifstream stream(p_FilePath);
		std::stringstream sstream;
		sstream << stream.rdbuf();

		YAML::Node data = YAML::Load(sstream.str());
		if (!data["Scene"]) {
			Utils::Logger::logMSG("No Scene Found in " + p_FilePath, "Scene", Utils::Severity::Warning);
			return false;
		}

		std::string scene_name = data["Scene"].as<std::string>();
		Utils::Logger::logMSG("Deserialized Scene: " + scene_name, "Scene", Utils::Severity::Info);

		auto entities = data["Entities"];
		if (entities) {
			for (auto& entity : entities) {
				std::string uuid = entity["Entity"].as<std::string>();
				Utils::Logger::logMSG("Deserialized Entity: " + uuid, "Scene", Utils::Severity::Info);

				ecs::EntityHandle handle = addEntity(*(new EntityNameComponent(uuid.c_str())));

				Mesh3DComponent* mesh = nullptr;
				Transform3DComponent* transform = nullptr;
				Camera3DComponent* camera = nullptr;
				CameraController3DComponent* cameraController = nullptr;
				SkyboxComponent* skybox = nullptr;
				DirectionalLight3DComponent* directional_light = nullptr;
				DShadowMapComponent* shadowMap = nullptr;

				auto& meshNode = entity["Mesh3DComponent"];
				auto& transformNode = entity["Transform3DComponent"];
				auto& cameraNode = entity["CameraComponent"];
				auto& skyboxNode = entity["SkyBoxComponent"];
				auto& dShadowMapNode = entity["DShadowMapComponent"];
				auto& dLightNode = entity["DirectionalLightComponent"];
				auto& cameraControllerNode = entity["CameraController3DComponent"];

				if (meshNode) {
					mesh = new Mesh3DComponent;
					mesh->mesh = new MeshData;
					mesh->mesh->m_FileName = meshNode["3D_ModelName"].as<std::string>();

					mesh->mesh->m_FileDirectory = meshNode["3D_ModelDirectory"].as<std::string>();
					mesh->mesh->m_TangentSpaceCorrectedUV = meshNode["TangentSpaceCorrectedUV"].as<bool>();

					if (mesh->mesh->m_FileName == "Sphere")
						generateUVSphere(50, 50, *mesh->mesh);
					else if (mesh->mesh->m_FileName == "Cube")
						generateCube(*mesh->mesh);
					/*else if (mesh->mesh->m_FileName == "Grid") {

					}*/
					else OBJ_Loader::loadModel((mesh->mesh->m_FileDirectory + "/").c_str(), mesh->mesh->m_FileName.c_str(), mesh->mesh, mesh->mesh->m_TangentSpaceCorrectedUV);

					int i = 0;
					for (auto& mtl : meshNode["Materials"]) {
						mesh->mesh->getMaterial(i).config.m_Albedo = mtl["Albedo"].as<Vector3f>();
						mesh->mesh->getMaterial(i).config.m_Ambient = mtl["Ambient"].as<float>();
						mesh->mesh->getMaterial(i).config.m_Metallic = mtl["Metallic"].as<float>();
						mesh->mesh->getMaterial(i).config.m_PDMDepth = mtl["PDMDepth"].as<float>();
						mesh->mesh->getMaterial(i).config.m_Roughness = mtl["Roughness"].as<float>();
						mesh->mesh->getMaterial(i).config.render = mtl["Render"].as<bool>();
						i++;
					}

					addComponent(handle, mesh);
				}
				if (transformNode) {
					transform = new Transform3DComponent;
					transform->transform.setPosition(transformNode["Position"].as<Vector3f>());
					transform->transform.rotation = transformNode["Rotation"].as<Vector3f>();
					transform->transform.setScale(transformNode["Scale"].as<Vector3f>());
					transform->transform.model = transformNode["Model"].as<Matrix4f>();
					addComponent(handle, transform);
				}
				
				if (dShadowMapNode) {
					shadowMap = new DShadowMapComponent;

					Vector2f v = dShadowMapNode["Resolution"].as<Vector2f>();
					shadowMap->ShadowMap.m_Resolution = { (int32_t)v.x, (int32_t)v.y };
					shadowMap->ShadowMap.m_Direction = dShadowMapNode["Direction"].as<Vector3f>();
					shadowMap->ShadowMap.m_DepthBiasConstant = dShadowMapNode["DepthBiasConstant"].as<float>();
					shadowMap->ShadowMap.m_DepthBiasSlope = dShadowMapNode["DepthBiasSlope"].as<float>();
					shadowMap->ShadowMap.m_View = dShadowMapNode["View_Matrix"].as<Matrix4f>();
					shadowMap->ShadowMap.m_Projection = dShadowMapNode["Projection_Matrix"].as<Matrix4f>();

					// Create shadow map framebuffer
					shadowMap->ShadowMap.m_Attachment.m_FramebufferAttachmentInfo.p_RectangleData = const_cast<vk::VertexArray*>(&vk::GraphicsContext::getRecVao());
					shadowMap->ShadowMap.m_Attachment.m_FramebufferAttachmentInfo.p_Size = shadowMap->ShadowMap.m_Resolution;
					shadowMap->ShadowMap.m_Attachment.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
						shadowMap->ShadowMap.m_Resolution,
						vk::ImageFormat::D16UN_1C,
						vk::Layout::DepthR,
						vk::BorderColor::OpaqueWhite,
						vk::SamplerMode::ClampToEdge, true, true
					);
					shadowMap->ShadowMap.m_Attachment.create();

					addComponent(handle, shadowMap);
				}
				
				if (dLightNode) {
					directional_light = new DirectionalLight3DComponent;
					directional_light->Direction = dLightNode["Direction"].as<Vector3f>();
					directional_light->Color = dLightNode["Color"].as<Vector3f>();
					addComponent(handle, directional_light);
				}
				
				if (skyboxNode) {
					skybox = new SkyboxComponent;
					skybox->AtmosphericScattering = skyboxNode["Atmospheric_Scattering"].as<bool>();
					skybox->HDR_Image_Path = skyboxNode["HDR_Image_Path"].as<std::string>();

					auto& atmosphereNode = skyboxNode["Atmosphere_Info"];

					skybox->AtmosphericScatteringInfo.p_RayOrigin			= atmosphereNode["Ray_Origin"].as<Vector3f>();
					skybox->AtmosphericScatteringInfo.p_SunPos				= atmosphereNode["Sun_Position"].as<Vector3f>();
					skybox->AtmosphericScatteringInfo.p_SunIntensity		= atmosphereNode["Sun_Intensity"].as<float>();
					skybox->AtmosphericScatteringInfo.p_PlanetRadius		= atmosphereNode["Planet_Radius"].as<float>();
					skybox->AtmosphericScatteringInfo.p_AtmosphereRadius	= atmosphereNode["Atmosphere_Radius"].as<float>();
					skybox->AtmosphericScatteringInfo.p_RayleighSC			= atmosphereNode["Rayleigh_Scale"].as<Vector3f>();
					skybox->AtmosphericScatteringInfo.p_MieSC				= atmosphereNode["Mie_Scale"].as<float>();
					skybox->AtmosphericScatteringInfo.p_RayleighSHeight		= atmosphereNode["Rayleigh_Scale_Height"].as<float>();
					skybox->AtmosphericScatteringInfo.p_MieSHeight			= atmosphereNode["Mie_Scale_Height"].as<float>();
					skybox->AtmosphericScatteringInfo.p_MieDir				= atmosphereNode["Mie_Scatter_Scale"].as<float>();
					skybox->AtmosphericScatteringInfo.p_Scale				= atmosphereNode["Scale"].as<float>();

					addComponent(handle, skybox);
				}

				if (cameraNode) {
					camera = new Camera3DComponent;
					camera->camera.setView((View)cameraNode["ViewType"].as<uint32_t>());
					camera->camera.setViewModifier(cameraNode["ViewModZ"].as<float>());
					camera->camera.setFieldOfView(cameraNode["PerspectiveFOV"].as<float>());
					camera->camera.setAspectRatio(cameraNode["AspectRatio"].as<float>());
					camera->camera.setPosition(cameraNode["Position"].as<Vector3f>());
					camera->camera.rotation = cameraNode["Rotation"].as<Vector3f>();
					Vector3f dir = cameraNode["ViewDirection"].as<Vector3f>();
					camera->camera.setViewDirection(dir.x, dir.y, dir.z);
					camera->camera.setPerspectiveMatrix(cameraNode["PerspectiveMatrix"].as<Matrix4f>());
					camera->camera.setOrthographicMatrix(cameraNode["OrthographicMatrix"].as<Matrix4f>());
					camera->camera.setViewMatrix(cameraNode["ViewMatrix"].as<Matrix4f>());
					addComponent(handle, camera);
				}
				if (cameraControllerNode) {
					cameraController = new CameraController3DComponent;

					cameraController->controller.enable(cameraControllerNode["Enabled"].as<bool>());
					cameraController->controller.m_MovementSpeed = cameraControllerNode["MovementSpeed"].as<float>();
					cameraController->controller.setSensitivity(cameraControllerNode["Sensitivity"].as<float>());
					cameraController->controller.setZoom(cameraControllerNode["Zoom"].as<float>());
					cameraController->controller.setYaw(cameraControllerNode["Yaw"].as<float>());
					cameraController->controller.setPitch(cameraControllerNode["Pitch"].as<float>());
					cameraController->controller.constrainPitch(cameraControllerNode["PitchConstrain"].as<bool>());

					addComponent(handle, cameraController);
				}
			}
		}
	}
	bool Scene::deserialize_bin(const std::string& p_FilePath)
	{
		return false;
	}
	
}
