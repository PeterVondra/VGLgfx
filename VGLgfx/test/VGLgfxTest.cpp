#include <ostream>
#define VGL_RENDER_API_VULKAN
#include <VGLgfx.h>
#include "../include/Utils/FileGUI.h"

class TestLayer : public vgl::Layer
{
public:
	void onAttach() {
		m_Camera = vgl::Camera(Vector3f(0.0f, 1.0f, 0.0f));
		m_Camera.setView(vgl::View::First_Person);
		m_CameraController.bind(m_Camera);
		m_CameraController.setDeltaTime(m_WindowPtr->getDeltaTime());
		m_CameraController.enable(true);

		vgl::Mesh3DComponent* mesh = new vgl::Mesh3DComponent();
		vgl::Transform3DComponent* transform = new vgl::Transform3DComponent();
		transform->transform.scale(1);
		mesh->mesh = new vgl::MeshData();
		
		vgl::OBJ_Loader::loadModel("../../VGLgfxEditor/projects/Global_Assets/3D_Models/Sponza/", "sponza.obj", mesh->mesh, false);
		mesh->mesh->getMaterial(19).config.m_Roughness = 0.77f;

		vgl::Mesh3DComponent* mesh2 = new vgl::Mesh3DComponent();
		vgl::Transform3DComponent* transform2 = new vgl::Transform3DComponent();
		transform2->transform.scale(50);
		transform2->transform.setPosition(100.0f, 10, -100.0f);
		mesh2->mesh = new vgl::MeshData();

		//vgl::OBJ_Loader::loadModel("../../VGLgfxEditor/projects/Global_Assets/3D_Models/", "Cthulhu.obj", mesh2->mesh, false);
		//mesh2->mesh->getMaterial(0).config.m_Roughness = 0.07f;
		//mesh2->mesh->getMaterial(0).config.m_Metallic = 1.00f;

		auto point_light = new vgl::PointLight3DComponent;
		point_light->Color = { 0.4f, 0.1f, 1.0f };
		point_light->Position = { 0.0f, 800.0f, 0.0f };
		point_light->Radius = 150;
		point_light->ShadowMapID = 0;

		auto directional_light = new vgl::DirectionalLight3DComponent;
		directional_light->Color = { 1, 1, 1 };
		directional_light->LightIntensity = 12;
		directional_light->Direction = { 0.333, 1, 0.333 };

		auto shadow_map = new vgl::DShadowMapComponent;
		shadow_map->ShadowMap.m_Projection = Matrix4f::orthoRH_ZO(-3000, 3000, -3000, 3000, 1, 4800);
		shadow_map->ShadowMap.create({ int32_t(4096), int32_t(4096) }, vgl::ImageFormat::D32SF);
		
		auto pshadow_map = new vgl::PShadowMapComponent;
		pshadow_map->ShadowMap.m_Position = &point_light->Position;
		pshadow_map->ShadowMap.create({ int32_t(512), int32_t(512) }, vgl::ImageFormat::C32SF_1C);

		auto skybox = new vgl::SkyboxComponent;
		skybox->_AtmosphericScatteringInfo = {};
		skybox->_AtmosphericScattering = true;
		skybox->skybox = new vgl::Skybox();

		m_Scene.addEntity(*mesh, *transform, *(new vgl::EntityNameComponent("Sponza")));
		//m_Scene.addEntity(*mesh2, *transform2, *(new vgl::EntityNameComponent("Cthulhu")));
		//m_PointLightEntity = m_Scene.addEntity(*point_light, *pshadow_map, *(new vgl::EntityNameComponent("Point Light")));
		//m_PointLightEntity = m_Scene.addEntity(*point_light2, *(new vgl::EntityNameComponent("Point Light")));
		m_DirectionalLightEntity = m_Scene.addEntity(*directional_light, *shadow_map, *(new vgl::EntityNameComponent("Directional Light")));
		m_SkyBoxEntity = m_Scene.addEntity(*skybox, *(new vgl::EntityNameComponent("SkyBox")));

		//vgl::GraphicsContextSingleton::getInstance().generateAtmosphericScatteringCubeMap({ 1024, 1024 }, m_Scene.getComponent<SkyboxComponent>(m_SkyBoxEntity)->AtmosphericScatteringInfo);

		m_RenderPipeline.setup(m_RendererPtr, m_WindowPtr, &m_Scene);
		vgl::ImageLoader::getImageFromPath(img, "data/CMakeResources/Vulkan.png", vgl::ColorSpace::RGB);
		vgl::ImageLoader::getImageFromPath(m_DefaultImage, "data/textures/default.png", vgl::ColorSpace::RGB);
	}
	void onDetach() {}
	void onUpdate(vgl::Renderer& p_Renderer) {

		if (vgl::Input::keyIsPressed(vgl::Key::F11))
			m_WindowPtr->toggleFullscreen();

		float rotSpeed = m_WindowPtr->getDeltaTime() / 1.2;

		if (m_DirectionalLightEntity) {
			auto directional_light = m_Scene.getComponent<vgl::DirectionalLight3DComponent>(m_DirectionalLightEntity);

			if (vgl::Input::keyIsDown(vgl::Key::Left))
				directional_light->Direction = Math::rotate(directional_light->Direction, { 0, 1, 0 }, rotSpeed);
			else if (vgl::Input::keyIsDown(vgl::Key::Right))
				directional_light->Direction = Math::rotate(directional_light->Direction, { 0, 1, 0 }, -rotSpeed);
			if (vgl::Input::keyIsDown(vgl::Key::Up))
				directional_light->Direction = Math::rotate(directional_light->Direction, { 0, 0, 1 }, rotSpeed);
			else if (vgl::Input::keyIsDown(vgl::Key::Down))
				directional_light->Direction = Math::rotate(directional_light->Direction, { 0, 0, 1 }, -rotSpeed);

			if (vgl::Input::keyIsPressed(vgl::Key::R))
				directional_light->Direction = { 0.333, 1, 0.333 };

			m_Scene.getComponent<vgl::SkyboxComponent>(m_SkyBoxEntity)->_AtmosphericScatteringInfo.p_SunPos = directional_light->Direction;
		}

		m_CameraController.setDeltaTime(m_WindowPtr->getDeltaTime());
		m_CameraController.update();

		vgl::RenderInfo info;
		info.p_AlphaBlending = true;
		info.p_CullMode = vgl::CullMode::BackBit;
		info.p_IATopology = vgl::IATopoogy::TriList;
		info.p_PolygonMode = vgl::PolygonMode::Fill;

		m_RenderPipeline.render(info);
	}
	void onImGuiUpdate() {
		static std::string time = "";

		if (m_WindowPtr->onTick())
			time = "FPS: " + std::to_string((int)(1 / (m_WindowPtr->getDeltaTime()))) + "   MS: " + std::to_string(m_WindowPtr->getDeltaTime() * 1000);

		ImGui::DockSpaceOverViewport();

		if (m_DirectionalLightEntity) {
			auto shadow_map = m_Scene.getComponent<vgl::DShadowMapComponent>(m_DirectionalLightEntity);
			if (shadow_map) {
				ImGui::Begin("Shadow Map", nullptr, ImGuiWindowFlags_NoDecoration);
				vgl::ImGuiContext::Image(shadow_map->ShadowMap.m_Attachment.getCurrentImageAttachments()[0].getImage(), ImGui::GetContentRegionAvail());
				ImGui::End();
			}
		}

		ImGui::Begin("GBuffer", nullptr);
		vgl::ImGuiContext::Image(m_RenderPipeline.m_GBuffer.getCurrentImageAttachments()[0].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		vgl::ImGuiContext::Image(m_RenderPipeline.m_GBuffer.getCurrentImageAttachments()[1].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		vgl::ImGuiContext::Image(m_RenderPipeline.m_GBuffer.getCurrentImageAttachments()[2].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		vgl::ImGuiContext::Image(m_RenderPipeline.m_GBuffer.getCurrentImageAttachments()[3].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		ImGui::End();

		ImGui::Begin("LightPass", nullptr);
		vgl::ImGuiContext::Image(m_RenderPipeline.m_LightPassFramebuffer.getCurrentImageAttachments()[0].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		ImGui::End();

		ImGui::Begin("SSAO", nullptr);
		vgl::ImGuiContext::Image(m_RenderPipeline.m_SSAOBlurFramebuffer.getCurrentImageAttachments()[0].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		ImGui::End();
		
		ImGui::Begin("SSR", nullptr);
		vgl::ImGuiContext::Image(m_RenderPipeline.m_SSRFramebuffer.getCurrentImageAttachments()[0].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		ImGui::End();
		
		ImGui::Begin("Upsampling", nullptr);
		vgl::ImGuiContext::Image(m_RenderPipeline.m_UpsamplingFramebuffers[0].getCurrentImageAttachments()[0].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 2.0f, 2.0f });
		ImGui::PushStyleColor(ImGuiCol_ChildBg, { (float)230 / 256, (float)180 / 256, (float)66 / 256, 1 });
		ImGui::Begin("Viewport", nullptr);
		m_ViewportData.m_Position.x = ImGui::GetCursorScreenPos().x;
		m_ViewportData.m_Position.y = ImGui::GetCursorScreenPos().y;
		m_ViewportData.m_Size = { (int32_t)ImGui::GetContentRegionAvail().x, (int32_t)ImGui::GetContentRegionAvail().y };
		ImGui::PushClipRect(
			{ (float)m_ViewportData.m_Position.x, (float)m_ViewportData.m_Position.y },
			{ (float)m_ViewportData.m_Position.x + (float)m_ViewportData.m_Size.x,
			  (float)m_ViewportData.m_Position.y + (float)m_ViewportData.m_Size.y },
			false
		);
		ImGuizmo::SetDrawlist();
		vgl::ImGuiContext::Image(m_RenderPipeline.m_HDRFramebuffer.getCurrentImageAttachments()[0].getImage(), { (float)m_ViewportData.m_Size.x, (float)m_ViewportData.m_Size.y });
		m_ViewportFocused = ImGui::IsWindowFocused();
		m_Camera.setAspectRatio((float)m_ViewportData.m_Size.x / m_ViewportData.m_Size.y);
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

		if (m_Camera.getView() == vgl::View::First_Person) {
			if (vgl::Input::keyIsPressed(vgl::Key::Tab)) {
				m_CameraController.enable(!m_CameraController.isEnabled());
				if (!m_CameraController.isEnabled()) {
					m_WindowPtr->enableCursor();
					ImGui::GetIO().WantCaptureKeyboard = true;
					ImGui::GetIO().WantCaptureMouse = true;
					ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
				}
				else {
					m_WindowPtr->disableCursor();
					ImGui::GetIO().WantCaptureKeyboard = false;
					ImGui::GetIO().WantCaptureMouse = false;
					ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
				}
			}
		}
		else if (m_Camera.getView() == vgl::View::Third_Person) {
			if ((vgl::Input::mouseButtonIsDown(0) || vgl::Input::mouseIsScrolling()) && m_ViewportFocused) {
				m_CameraController.enable(true);
				if (m_CameraController.isEnabled()) {
					m_WindowPtr->disableCursor();
					ImGui::GetIO().WantCaptureKeyboard = false;
					ImGui::GetIO().WantCaptureMouse = false;
					ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
				}
			}
			else {
				m_CameraController.enable(false);
				m_WindowPtr->enableCursor();
				ImGui::GetIO().WantCaptureKeyboard = true;
				ImGui::GetIO().WantCaptureMouse = true;
				ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			}
		}

		ImGui::Begin("Editor Camera");
		ImGui::Text(("Position: [" + std::to_string(m_Camera.getPosition().x) + ", " + std::to_string(m_Camera.getPosition().y) + "]").c_str());
		ImGui::Text(("Rotation: [" + std::to_string(m_Camera.getRotation().x) + ", " + std::to_string(m_Camera.getRotation().y) + std::to_string(m_Camera.getRotation().z) + "]").c_str());
		if (ImGui::TreeNodeEx("Camera", vgl::EditorColorScheme::TreeNodeFlags)) {

			if (m_Camera.getView() == vgl::View::First_Person) {
				if (ImGui::Button("Third Person View"))
					m_Camera.setView(vgl::View::Third_Person);
			}
			else if (m_Camera.getView() == vgl::View::Third_Person) {
				if (ImGui::Button("First Person View"))
					m_Camera.setView(vgl::View::First_Person);
			}
			ImGui::Text("[Press tab to enable/disable first person viewing]");
			ImGui::Text("Field Of View");
			ImGui::DragFloat("##32", &const_cast<float&>(m_Camera.getFieldOfView()), 1, 0);
			ImGui::Text("Movement Speed");
			ImGui::DragFloat("##33", &m_CameraController.m_MovementSpeed, 1);
			ImGui::TreePop();
		}
		ImGui::End();

		ImGui::BeginMainMenuBar();
		ImGui::Text(time.c_str());
		ImGui::EndMainMenuBar();

		ImGui::Begin("Post Processing");
		if (ImGui::TreeNodeEx("HDR##S", vgl::EditorColorScheme::TreeNodeFlags)) {
			ImGui::BeginColumns("##HDR", 2);
			ImGui::Text("Filmic Strength");
			ImGui::Spacing();
			ImGui::Text("Filmic Scale");
			ImGui::Spacing();
			ImGui::Text("Gamma");
			ImGui::Spacing();
			ImGui::Text("Exposure");
			ImGui::Spacing();
			ImGui::Text("Adaptive Exposure Rate");
			ImGui::Spacing();
			ImGui::Text("Bloom Threshold");
			ImGui::Spacing();
			ImGui::Text("Bloom Filter Radius");
			float offset = ImGui::GetColumnOffset();
			ImGui::NextColumn();
			ImGui::SetColumnWidth(1, ImGui::GetWindowSize().x - offset);
			ImGui::DragFloat("##113", &m_RenderPipeline.m_HDRInfo.filmicStrength, 1);
			ImGui::DragFloat("##2", &m_RenderPipeline.m_HDRInfo.filmicScale, 1);
			ImGui::DragFloat("##3", &m_RenderPipeline.m_HDRInfo.gamma, 0.01);
			ImGui::DragFloat("##4", &m_RenderPipeline.m_HDRInfo.exposure, 0.01);
			ImGui::DragFloat("##42", &m_RenderPipeline.m_HDRInfo.exposure_adapt_rate, 0.0001, 0.0001);
			ImGui::DragFloat("##43", &m_RenderPipeline.m_BloomThreshold, 0.1, 0);
			ImGui::DragFloat("##44", &m_RenderPipeline.m_FilterRadius, 0.0001, 0.000);
#undef max
#undef min
			std::max(m_RenderPipeline.m_HDRInfo.exposure_adapt_rate, 0.000001f);
			ImGui::EndColumns();
			ImGui::Separator();
			ImGui::Checkbox("Vignette", (bool*)&m_RenderPipeline.m_HDRInfo.vignetting);
			ImGui::Text("Use optical lens vignetting?");
			ImGui::DragFloat("Vignette Out##VO", &m_RenderPipeline.m_HDRInfo.vignout, 0.01);
			ImGui::DragFloat("Vignette In##VI", &m_RenderPipeline.m_HDRInfo.vignin, 0.01);
			ImGui::DragFloat("Vignette Fade##VF", &m_RenderPipeline.m_HDRInfo.vignfade, 0.1);
			ImGui::TreePop();
		}
		ImGui::Separator();
		if (ImGui::TreeNodeEx("FXAA##S", vgl::EditorColorScheme::TreeNodeFlags)) {
			ImGui::BeginColumns("##FXAA", 2);
			ImGui::Text("FXAA Bias");
			ImGui::Spacing();
			ImGui::Text("FXAA Min");
			ImGui::Spacing();
			ImGui::Text("FXAA Span Max");
			ImGui::Spacing();
			float offset = ImGui::GetColumnOffset();
			ImGui::NextColumn();
			ImGui::SetColumnWidth(1, ImGui::GetWindowSize().x - offset);
			static float fxaaBias = 1 / m_RenderPipeline.m_FXAAInfo.FXAA_REDUCTION_BIAS;
			static float fxaaMin = 1 / m_RenderPipeline.m_FXAAInfo.FXAA_REDUCTION_MIN;
			if (ImGui::DragFloat("##5", &fxaaBias, 2))
				m_RenderPipeline.m_FXAAInfo.FXAA_REDUCTION_BIAS = 1.0f / fxaaBias;
			if (ImGui::DragFloat("##6", &fxaaMin, 2))
				m_RenderPipeline.m_FXAAInfo.FXAA_REDUCTION_MIN = 1.0f / fxaaMin;

			ImGui::DragFloat("##7", &m_RenderPipeline.m_FXAAInfo.FXAA_Span_Max, 2);
			ImGui::EndColumns();
			ImGui::TreePop();
		}
		ImGui::Separator();
		if (ImGui::TreeNodeEx("SSAO##S", vgl::EditorColorScheme::TreeNodeFlags)) {
			ImGui::BeginColumns("##SSAO", 2);
			ImGui::Text("Intensity");
			ImGui::Spacing();
			ImGui::Text("Radius");
			ImGui::Spacing();
			ImGui::Text("Bias");
			ImGui::Spacing();
			float offset = ImGui::GetColumnOffset();
			ImGui::NextColumn();
			ImGui::SetColumnWidth(1, ImGui::GetWindowSize().x - offset);
			ImGui::DragFloat("##SSAO1", &m_RenderPipeline.SSAO_Intensity, 0.01);
			ImGui::DragFloat("##SSAO2", &m_RenderPipeline.SSAO_Radius, 0.01);
			ImGui::DragFloat("##SSAO3", &m_RenderPipeline.SSAO_Bias, 0.01);
			std::max(m_RenderPipeline.m_HDRInfo.exposure_adapt_rate, 0.000001f);
			ImGui::EndColumns();
			ImGui::TreePop();
		}
		ImGui::Separator();
		if (ImGui::TreeNodeEx("SSR##SSR", vgl::EditorColorScheme::TreeNodeFlags)) {
			ImGui::BeginColumns("##SSR", 2);
			ImGui::Text("Step Length");
			ImGui::Spacing();
			ImGui::Text("Max Steps");
			ImGui::Spacing();
			ImGui::Text("Binary Search Steps");
			ImGui::Spacing();
			ImGui::Text("Reflection Specular Falloff Exponent");
			ImGui::Spacing();
			float offset = ImGui::GetColumnOffset();
			ImGui::NextColumn();
			ImGui::SetColumnWidth(1, ImGui::GetWindowSize().x - offset);
			ImGui::DragFloat("##SSR1", &m_RenderPipeline.SSR_Data.SSR_StepLength, 0.01);
			ImGui::DragFloat("##SSR2", &m_RenderPipeline.SSR_Data.SSR_MaxSteps, 0.01);
			ImGui::DragFloat("##SSR3", &m_RenderPipeline.SSR_Data.SSR_BinarySearchSteps, 1);
			ImGui::DragFloat("##SSR4", &m_RenderPipeline.SSR_Data.SSR_ReflectionSpecularFalloffExponent, 0.01);
			std::max(m_RenderPipeline.m_HDRInfo.exposure_adapt_rate, 0.000001f);
			ImGui::EndColumns();
			ImGui::TreePop();
		}
		ImGui::End();

		ImGui::Begin("Depth of Field");
		ImGui::Checkbox("Auto Focus", (bool*)&m_RenderPipeline.m_DOFInfo.autofocus);
		ImGui::Separator();
		ImGui::Checkbox("Noise Dither", (bool*)&m_RenderPipeline.m_DOFInfo.noise);
		ImGui::Text("Use noise instead of pattern for sample dithering");
		ImGui::Separator();
		ImGui::Checkbox("Show Focus", (bool*)&m_RenderPipeline.m_DOFInfo.showFocus);
		ImGui::Text("Show debug focus point and focal range (red = focal point, green = focal range)");
		ImGui::Separator();
		ImGui::BeginColumns("##DOF", 2);
		ImGui::Spacing();
		ImGui::Text("Focal Depth");
		ImGui::Spacing();
		ImGui::Text("Focal Length");
		ImGui::Spacing();
		ImGui::Text("f_Stop");
		ImGui::Spacing();
		ImGui::Text("Max Blur");
		ImGui::Spacing();
		ImGui::Text("Focus Point");
		ImGui::Spacing();
		ImGui::Text("Dither Amount");
		ImGui::Spacing();
		ImGui::Text("Samples");
		ImGui::Spacing();
		ImGui::Text("Rings");
		ImGui::Spacing();
		ImGui::Text("Circle of Confusion Size(mm)");
		ImGui::Spacing();
		ImGui::Text("Highlight Threshold");
		ImGui::Spacing();
		ImGui::Text("Gain");
		ImGui::Spacing();
		ImGui::Text("Bokeh Edge Bias");
		ImGui::Spacing();
		ImGui::Text("Bokeh Chromatic Aberration/Fringing");
		ImGui::Spacing();
		float offset = ImGui::GetColumnOffset();
		ImGui::NextColumn();
		ImGui::SetColumnWidth(1, ImGui::GetWindowSize().x - offset);
		ImGui::DragFloat("##FD", &m_RenderPipeline.m_DOFInfo.focalDepth, 0.01);
		ImGui::DragFloat("##FL", &m_RenderPipeline.m_DOFInfo.focalLength, 0.01);
		ImGui::DragFloat("##FS", &m_RenderPipeline.m_DOFInfo.fstop, 0.01);
		ImGui::DragFloat("##MB", &m_RenderPipeline.m_DOFInfo.maxblur, 0.01);
		ImGui::DragFloat2("##F", m_RenderPipeline.m_DOFInfo.focus.get(), 0.01, 0.0f, 1.0f);
		ImGui::DragFloat("##NM", &m_RenderPipeline.m_DOFInfo.namount, 0.000001);
		ImGui::InputInt("##S", &m_RenderPipeline.m_DOFInfo.samples);
		m_RenderPipeline.m_DOFInfo.samples = m_RenderPipeline.m_DOFInfo.samples < 0 ? 0 : m_RenderPipeline.m_DOFInfo.samples;
		ImGui::InputInt("##R", &m_RenderPipeline.m_DOFInfo.rings);
		m_RenderPipeline.m_DOFInfo.rings = m_RenderPipeline.m_DOFInfo.rings < 0 ? 0 : m_RenderPipeline.m_DOFInfo.rings;
		ImGui::DragFloat("##COC", (float*)&m_RenderPipeline.m_DOFInfo.CoC, 0.0001);
		ImGui::DragFloat("##T", (float*)&m_RenderPipeline.m_DOFInfo.threshold, 0.01);
		ImGui::DragFloat("##G", (float*)&m_RenderPipeline.m_DOFInfo.gain, 0.01);
		ImGui::DragFloat("##B", (float*)&m_RenderPipeline.m_DOFInfo.bias, 0.01);
		ImGui::DragFloat("##FRINGE", (float*)&m_RenderPipeline.m_DOFInfo.fringe, 0.01);
		ImGui::EndColumns();
		ImGui::Separator();
		ImGui::End();

		/*if (igfd::ImGuiFileDialog::Instance()->FileDialog("fobj10", 32, {600, 250}))
		{
			if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
			{
				vgl::Mesh3DComponent* model = new vgl::Mesh3DComponent();
				model->mesh = new vgl::MeshData();
				vgl::EntityNameComponent* name = new vgl::EntityNameComponent();
				vgl::Transform3DComponent* new_transform = new vgl::Transform3DComponent();

				std::string fileName = igfd::ImGuiFileDialog::Instance()->GetCurrentFileName();
				std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath() + "/";

				vgl::OBJ_Loader::loadModel(filePath.c_str(), fileName.c_str(), model->mesh, false);
				//name->Name = stripExtension(model->mesh->m_FileName, ".obj").c_str();
				name->Name = model->mesh->m_FileName;

				m_Scene.addEntity(*model, *name, *new_transform);
			}
			igfd::ImGuiFileDialog::Instance()->CloseDialog("fobj10");
		}*/

		auto& atmosphere_info = m_Scene.getComponent<vgl::SkyboxComponent>(m_SkyBoxEntity)->_AtmosphericScatteringInfo;

		ImGui::Begin("Sky");
		if (ImGui::TreeNodeEx("Atmospheric Scattering##A", vgl::EditorColorScheme::TreeNodeFlags)) {
			ImGui::BeginColumns("##ASConfig", 2);
			ImGui::Text("Sun Intensity");
			ImGui::Spacing();
			ImGui::Text("Scattering Scale");
			ImGui::Spacing();
			ImGui::Text("Mie Scale");
			ImGui::Spacing();
			ImGui::Text("Rayleigh Height");
			ImGui::Spacing();
			ImGui::Text("Mie Height");
			float offset = ImGui::GetColumnOffset();
			ImGui::NextColumn();
			ImGui::SetColumnWidth(1, ImGui::GetWindowSize().x - offset);
			ImGui::DragFloat("##113", &atmosphere_info.p_SunIntensity, 0.01, '%.8f');
			ImGui::DragFloat("##2", &atmosphere_info.p_Scale, 0.01, '%.8f');
			ImGui::DragFloat("##4", &atmosphere_info.p_MieSC, 1, '%.8f');
			ImGui::DragFloat("##5", &atmosphere_info.p_RayleighSHeight, 1, '%.8f');
			ImGui::DragFloat("##6", &atmosphere_info.p_MieSHeight, 0.0001, '%.8f');
			ImGui::EndColumns();

			ImGui::Text("Planet Radius");
			ImGui::DragFloat("##3", &atmosphere_info.p_PlanetRadius, 10);
			static float height = atmosphere_info.p_AtmosphereRadius - atmosphere_info.p_PlanetRadius;
			ImGui::Text("Atmosphere Height");
			ImGui::DragFloat("##Atmosphere Height", &height, 10, 0);
			atmosphere_info.p_AtmosphereRadius = atmosphere_info.p_PlanetRadius + height;
			ImGui::Text("Rayleigh Scale R");
			ImGui::DragFloat("##Rayleigh Scale R", &atmosphere_info.p_RayleighSC.r, 0.0000001f, '%.8f');
			ImGui::Text("Rayleigh Scale G");
			ImGui::DragFloat("##Rayleigh Scale G", &atmosphere_info.p_RayleighSC.g, 0.0000001f, '%.8f');
			ImGui::Text("Rayleigh Scale B");
			ImGui::DragFloat("##Rayleigh Scale B", &atmosphere_info.p_RayleighSC.b, 0.0000001f, '%.8f');
			ImGui::TreePop();
		}
		ImGui::End();

		sceneDisplay();
	}

	void transformEdit(vgl::Transform3DComponent* p_TransformComponent, const int p_ID, bool p_Translation = true, bool p_Rotation = true, bool p_Scaling = true)
	{
		if (!p_Translation && !p_Rotation && !p_Scaling)
			return;
		if (ImGui::TreeNodeEx(("Transform##" + std::to_string(p_ID)).c_str(), vgl::EditorColorScheme::TreeNodeFlagsFramed)) {
			static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
			static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

			if (p_Translation) {
				if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
					mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
				ImGui::SameLine();
			}
			if (p_Rotation) {
				if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
					mCurrentGizmoOperation = ImGuizmo::ROTATE;
				ImGui::SameLine();
			}
			if (p_Scaling) {
				if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
					mCurrentGizmoOperation = ImGuizmo::SCALE;
			}

			static bool useSnap(false);
			Vector3f snap;

			Vector3f matrixTranslation, matrixRotation, matrixScale;
			ImGuizmo::DecomposeMatrixToComponents(&p_TransformComponent->transform.model[0], matrixTranslation.get(), matrixRotation.get(), matrixScale.get());
			if (p_Translation) vgl::DrawVec3Control("Translation", matrixTranslation);
			if (p_Rotation) vgl::DrawVec3Control("Rotation", matrixRotation);
			if (p_Scaling) vgl::DrawVec3Control("Scale", matrixScale, 1.0f);

			if (p_Translation || p_Rotation) {
				if (mCurrentGizmoOperation != ImGuizmo::SCALE)
				{
					if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
						mCurrentGizmoMode = ImGuizmo::LOCAL;
					ImGui::SameLine();
					if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
						mCurrentGizmoMode = ImGuizmo::WORLD;
				}
			}
			ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation.get(), matrixRotation.get(), matrixScale.get(), &p_TransformComponent->transform.model[0]);

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(m_ViewportData.m_Position.x, m_ViewportData.m_Position.y, m_ViewportData.m_Size.x, m_ViewportData.m_Size.y);
			ImGuizmo::Manipulate(&m_Camera.getViewMatrix()[0], &m_Camera.getPerspectiveMatrix()[0],
				mCurrentGizmoOperation, mCurrentGizmoMode, &p_TransformComponent->transform.model[0], NULL, (useSnap ? &snap.x : NULL));

			ImGui::TreePop();
		}
	}

	void sceneDisplay()
	{
		ImGui::Begin("Scene");
		ImGui::BeginChild("SCENE-HIERARCHY");
		vgl::ImGuiRowsBackground();
		static int32_t C0unt = 0;
		if (ImGui::BeginPopupContextWindow("CNE")) {
			if (ImGui::MenuItem(("Create New Entity##" + std::to_string(C0unt)).c_str())) {
				vgl::EntityNameComponent* name = new vgl::EntityNameComponent();
				name->Name += "Entity " + std::to_string(C0unt);
				C0unt++;
				m_Scene.addEntity(*name);
			}
			ImGui::EndPopup();
		}
		//if (ImGui::BeginPopupContextWindow("CNPL")) {
		//	if (ImGui::MenuItem(("Create New Point Light##p" + std::to_string(C0unt)).c_str())) {
		//		vgl::EntityNameComponent* name = new vgl::EntityNameComponent();
		//		vgl::PointLight3DComponent* light = new vgl::PointLight3DComponent();
		//
		//		name->Name = "Point Light " + std::to_string(C0unt);
		//		C0unt++;
		//		m_Scene.addEntity(*name, *light);
		//	}
		//	ImGui::EndPopup();
		//}

		static bool openGridEdit = false;

		static int entity_idx = -1;

		for (int i = 0; i < m_Scene.getEntities().size(); i++) {
			auto* mesh = m_Scene.getComponent<vgl::Mesh3DComponent>(m_Scene.getEntities()[i]);
			auto* name = m_Scene.getComponent<vgl::EntityNameComponent>(m_Scene.getEntities()[i]);
			auto* transform = m_Scene.getComponent<vgl::Transform3DComponent>(m_Scene.getEntities()[i]);
			auto* directional_light = m_Scene.getComponent<vgl::DirectionalLight3DComponent>(m_Scene.getEntities()[i]);
			auto* point_light = m_Scene.getComponent<vgl::PointLight3DComponent>(m_Scene.getEntities()[i]);

			//float y = ImGui::GetTextLineHeight();
			//ImGuiContext::Image(m_EntityIcon, { y,y });
			//ImGui::SameLine();
			bool open = ImGui::TreeNodeEx(name->Name.c_str());

			if (open) {
				if (directional_light) {
					ImGui::Begin("Component Properties");
					if (ImGui::TreeNodeEx(name->Name.c_str(), vgl::EditorColorScheme::TreeNodeFlags)) {
						ImGui::ColorPicker3("Color", directional_light->Color.get(), ImGuiColorEditFlags_DisplayRGB);
						ImGui::DragFloat("Light Intensity", &directional_light->LightIntensity, 0.1f);
						ImGui::DragFloat("Mie Scatter Constant (Volumetric Lighting)", &directional_light->VolumetricLightDensity, 1e-2);
						directional_light->VolumetricLightDensity = 0.00001 + std::abs(directional_light->VolumetricLightDensity);
						ImGui::TreePop();
					}
					ImGui::End();
				}
				if (point_light) {
					ImGui::Begin("Component Properties");
					if (ImGui::TreeNodeEx(name->Name.c_str(), vgl::EditorColorScheme::TreeNodeFlags)) {

						ImGui::ColorPicker3("Color", point_light->Color.get(), ImGuiColorEditFlags_DisplayRGB);
						ImGui::DragFloat("Light Intensity", &point_light->LightIntensity, 0.1f);

						static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
						static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

						Vector3f matrixTranslation, matrixRotation, matrixScale;
						ImGuizmo::DecomposeMatrixToComponents(&point_light->Transform[0], matrixTranslation.get(), matrixRotation.get(), matrixScale.get());
						vgl::DrawVec3Control("Translation", matrixTranslation);
						ImGui::DragFloat("Radius", &point_light->Radius, 0.1f);

						point_light->Position = matrixTranslation;

						ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation.get(), matrixRotation.get(), matrixScale.get(), &point_light->Transform[0]);

						ImGuiIO& io = ImGui::GetIO();
						ImGuizmo::SetRect(m_ViewportData.m_Position.x, m_ViewportData.m_Position.y, m_ViewportData.m_Size.x, m_ViewportData.m_Size.y);
						ImGuizmo::Manipulate(&m_Camera.getViewMatrix()[0], &m_Camera.getPerspectiveMatrix()[0],
							mCurrentGizmoOperation, mCurrentGizmoMode, &point_light->Transform[0], NULL, NULL);

						ImGui::TreePop();
					}
					ImGui::End();
				}
				if (transform) {
					//ImGuiContext::Image(m_TransformIcon, { y,y });
					//ImGui::SameLine();
					if (ImGui::TreeNodeEx("Transform", vgl::EditorColorScheme::TreeNodeFlags)) {
						ImGui::Begin("Component Properties");
						transformEdit(transform, i);
						ImGui::End();
						ImGui::TreePop();
					}
				}
			}

			if (mesh && mesh->mesh) {
				if (open) {
					if (ImGui::BeginPopupContextWindow(std::to_string(i).c_str())) {
						if (ImGui::MenuItem(("Delete##" + std::to_string(i)).c_str())) {
							mesh->mesh->destroy();
							m_Scene.deleteEntity(m_Scene.getEntities()[i]);
							ImGui::EndPopup();
							ImGui::TreePop();
							continue;
						}
						ImGui::EndPopup();
					}

					for (int j = 0; j < mesh->mesh->m_SubMeshIndices.size(); j++) {
						//ImGuiContext::Image(m_MaterialIcon, { y,y });
						//ImGui::SameLine();
						std::string name = mesh->mesh->getMaterial(j).m_Name != "" ? mesh->mesh->getMaterial(j).m_Name : "Material " + std::to_string(j);
						if (ImGui::TreeNodeEx(name.c_str(), vgl::EditorColorScheme::TreeNodeFlags)) {
							ImGui::Begin("Component Properties");

							ImGui::Checkbox(("##" + name).c_str(), &mesh->mesh->render(j));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip((std::string("Render=") + (mesh->mesh->render(j) ? "on" : "off")).c_str());

							ImGui::SameLine();
							if (ImGui::TreeNodeEx((name + "##" + std::to_string(i)).c_str(), vgl::EditorColorScheme::TreeNodeFlagsFramed)) {
								ImGui::Text("Metallic");
								ImGui::SliderFloat("##4", &mesh->mesh->getMaterial(j).config.m_Metallic, 0.02f, 1.0f);
								ImGui::Text("Roughness");
								ImGui::SliderFloat("##5", &mesh->mesh->getMaterial(j).config.m_Roughness, 0.02f, 1.0f);
								ImGui::Text("PDM Depth");
								ImGui::SliderFloat("##6", &mesh->mesh->getMaterial(j).config.m_PDMDepth, 0.02f, 1.0f);

								ImGui::PushItemWidth(128);

								if (ImGui::TreeNodeEx("Albedo Color", vgl::EditorColorScheme::TreeNodeFlags)) {
									ImGui::ColorPicker3("Albedo", mesh->mesh->getMaterial(j).config.m_Albedo.get());
									ImGui::TreePop();
								}

								if (ImGui::TreeNodeEx("Albedo Map", vgl::EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_AlbedoMap.isValid())
										vgl::ImGuiContext::Image(mesh->mesh->getMaterial(j).m_AlbedoMap, { 64, 64 });
									else
										vgl::ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									//if (ImGui::BeginDragDropTarget() && vgl::Input::mouseButtonIsReleased(0)) {
									//	if (ImGui::AcceptDragDropPayload("Image")) {
									//		if (mesh->mesh->getMaterial(j).m_AlbedoMap.isValid())
									//			mesh->mesh->getMaterial(j).m_AlbedoMap.destroy();
									//
									//		vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_AlbedoMap, m_Draggable.c_str(), SamplerMode::Repeat);
									//
									//		mesh->mesh->m_MTLRecreateFlag = true;
									//	}
									//	ImGui::EndDragDropTarget();
									//}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_AlbedoMap.isValid())
											mesh->mesh->getMaterial(j).m_AlbedoMap.destroy();

										vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_AlbedoMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), vgl::ColorSpace::SRGB, vgl::SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_AlbedoMap.isValid()) {
											mesh->mesh->getMaterial(j).m_AlbedoMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								if (ImGui::TreeNodeEx("Normal Map", vgl::EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_NormalMap.isValid())
										vgl::ImGuiContext::Image(mesh->mesh->getMaterial(j).m_NormalMap, { 64, 64 });
									else
										vgl::ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									//if (ImGui::BeginDragDropTarget() && vgl::Input::mouseButtonIsReleased(0)) {
									//	if (ImGui::AcceptDragDropPayload("Image")) {
									//		if (mesh->mesh->getMaterial(j).m_NormalMap.isValid())
									//			mesh->mesh->getMaterial(j).m_NormalMap.destroy();
									//
									//		vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_NormalMap, m_Draggable.c_str(), SamplerMode::Repeat);
									//
									//		mesh->mesh->m_MTLRecreateFlag = true;
									//	}
									//	ImGui::EndDragDropTarget();
									//}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_NormalMap.isValid())
											mesh->mesh->getMaterial(j).m_NormalMap.destroy();

										vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_NormalMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), vgl::ColorSpace::SRGB, vgl::SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_NormalMap.isValid()) {
											mesh->mesh->getMaterial(j).m_NormalMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								if (ImGui::TreeNodeEx("Displacement Map", vgl::EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_DisplacementMap.isValid())
										vgl::ImGuiContext::Image(mesh->mesh->getMaterial(j).m_DisplacementMap, { 64, 64 });
									else
										vgl::ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									//if (ImGui::BeginDragDropTarget() && vgl::Input::mouseButtonIsReleased(0)) {
									//	if (ImGui::AcceptDragDropPayload("Image")) {
									//		if (mesh->mesh->getMaterial(j).m_DisplacementMap.isValid())
									//			mesh->mesh->getMaterial(j).m_DisplacementMap.destroy();
									//
									//		vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_DisplacementMap, m_Draggable.c_str(), SamplerMode::Repeat);
									//
									//		mesh->mesh->m_MTLRecreateFlag = true;
									//	}
									//	ImGui::EndDragDropTarget();
									//}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_DisplacementMap.isValid())
											mesh->mesh->getMaterial(j).m_DisplacementMap.destroy();

										vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_DisplacementMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), vgl::ColorSpace::SRGB, vgl::SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_DisplacementMap.isValid()) {
											mesh->mesh->getMaterial(j).m_DisplacementMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								if (ImGui::TreeNodeEx("Metallic Map", vgl::EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_MetallicMap.isValid())
										vgl::ImGuiContext::Image(mesh->mesh->getMaterial(j).m_MetallicMap, { 64, 64 });
									else
										vgl::ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									//if (ImGui::BeginDragDropTarget() && vgl::Input::mouseButtonIsReleased(0)) {
									//	if (ImGui::AcceptDragDropPayload("Image")) {
									//		if (mesh->mesh->getMaterial(j).m_MetallicMap.isValid())
									//			mesh->mesh->getMaterial(j).m_MetallicMap.destroy();
									//
									//		vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_MetallicMap, m_Draggable.c_str(), SamplerMode::Repeat);
									//
									//		mesh->mesh->m_MTLRecreateFlag = true;
									//	}
									//	ImGui::EndDragDropTarget();
									//}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_MetallicMap.isValid())
											mesh->mesh->getMaterial(j).m_MetallicMap.destroy();

										vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_MetallicMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), vgl::ColorSpace::SRGB, vgl::SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_MetallicMap.isValid()) {
											mesh->mesh->getMaterial(j).m_MetallicMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								if (ImGui::TreeNodeEx("Roughness Map", vgl::EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_RoughnessMap.isValid())
										vgl::ImGuiContext::Image(mesh->mesh->getMaterial(j).m_RoughnessMap, { 64, 64 });
									else
										vgl::ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									//if (ImGui::BeginDragDropTarget() && vgl::Input::mouseButtonIsReleased(0)) {
									//	if (ImGui::AcceptDragDropPayload("Image")) {
									//		if (mesh->mesh->getMaterial(j).m_RoughnessMap.isValid())
									//			mesh->mesh->getMaterial(j).m_RoughnessMap.destroy();
									//
									//		vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_RoughnessMap, m_Draggable.c_str(), SamplerMode::Repeat);
									//
									//		mesh->mesh->m_MTLRecreateFlag = true;
									//	}
									//	ImGui::EndDragDropTarget();
									//}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_RoughnessMap.isValid())
											mesh->mesh->getMaterial(j).m_RoughnessMap.destroy();

										vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_RoughnessMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), vgl::ColorSpace::SRGB, vgl::SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_RoughnessMap.isValid()) {
											mesh->mesh->getMaterial(j).m_RoughnessMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								if (ImGui::TreeNodeEx("AO Map", vgl::EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_AOMap.isValid())
										vgl::ImGuiContext::Image(mesh->mesh->getMaterial(j).m_AOMap, { 64, 64 });
									else
										vgl::ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									//if (ImGui::BeginDragDropTarget() && vgl::Input::mouseButtonIsReleased(0)) {
									//	if (ImGui::AcceptDragDropPayload("Image")) {
									//		if (mesh->mesh->getMaterial(j).m_AOMap.isValid())
									//			mesh->mesh->getMaterial(j).m_AOMap.destroy();
									//
									//		vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_AOMap, m_Draggable.c_str(), SamplerMode::Repeat);
									//
									//		mesh->mesh->m_MTLRecreateFlag = true;
									//	}
									//	ImGui::EndDragDropTarget();
									//}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_AOMap.isValid())
											mesh->mesh->getMaterial(j).m_AOMap.destroy();

										vgl::ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_AOMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), vgl::ColorSpace::SRGB, vgl::SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_AOMap.isValid()) {
											mesh->mesh->getMaterial(j).m_AOMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								ImGui::TreePop();
							}
							ImGui::End();
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
			else if (open) {
				if (ImGui::BeginPopupContextWindow(std::to_string(i).c_str())) {
					if (ImGui::MenuItem(("Delete##" + std::to_string(i)).c_str())) {
						m_Scene.deleteEntity(m_Scene.getEntities()[i]);
						ImGui::EndPopup();
						ImGui::TreePop();
						continue;
					}
					else if (ImGui::BeginMenu(("Add Component##" + std::to_string(i)).c_str())) {
						if (ImGui::MenuItem("Sphere")) {
							vgl::Mesh3DComponent* new_mesh = new vgl::Mesh3DComponent();
							vgl::Transform3DComponent* new_transform = new vgl::Transform3DComponent();
							new_mesh->mesh = new vgl::MeshData();
							new_mesh->mesh->m_FileName = "Sphere";

							generateUVSphere(50, 50, *new_mesh->mesh);
							new_transform->transform.setScale(100);

							m_Scene.addComponent(m_Scene.getEntities()[i], new_mesh);
							m_Scene.addComponent(m_Scene.getEntities()[i], new_transform);
							ImGui::EndPopup();
						}
						else if (ImGui::MenuItem("Cube")) {
							vgl::Mesh3DComponent* new_mesh = new vgl::Mesh3DComponent();
							vgl::Transform3DComponent* new_transform = new vgl::Transform3DComponent();
							new_mesh->mesh = new vgl::MeshData();
							new_mesh->mesh->m_FileName = "Cube";

							generateCube(*new_mesh->mesh);
							new_transform->transform.setScale(100);

							m_Scene.addComponent(m_Scene.getEntities()[i], new_mesh);
							m_Scene.addComponent(m_Scene.getEntities()[i], new_transform);
							ImGui::EndPopup();
						}
						else if (ImGui::MenuItem("Grid")) {
							openGridEdit = true;
							entity_idx = i;
							ImGui::EndPopup();
						}
						else ImGui::EndPopup();
						ImGui::EndMenu();
					}
					else ImGui::EndPopup();
				}
				ImGui::TreePop();
			}

			//ImGui::Begin("Entity Names");
			//ImGui::Text(("Entity Name " + std::to_string(i)).c_str());
			//ImGui::SameLine();
			//ImGui::InputText(("##" + std::to_string(i)).c_str(), const_cast<char*>(name->name.c_str()), 30);
			//ImGui::End();

			if (openGridEdit && entity_idx == i) {
				static int settings[4];
				ImGui::Begin("Grid settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
				ImGui::DragInt2("Tesselations", settings, 0);
				ImGui::DragInt2("Grid size", &settings[2], 0);

				if (ImGui::Button("Create")) {
					vgl::Mesh3DComponent* model = new vgl::Mesh3DComponent();
					vgl::Transform3DComponent* new_transform = new vgl::Transform3DComponent();
					model->mesh = new vgl::MeshData();
					model->mesh->m_FileName = "Grid";

					generateGrid(settings[0], settings[1], { (float)settings[2], (float)settings[3] }, *model->mesh);
					openGridEdit = false;
					m_Scene.addComponent(m_Scene.getEntities()[i], model);
					m_Scene.addComponent(m_Scene.getEntities()[i], new_transform);
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
					openGridEdit = false;

				ImGui::End();

			}
		}

		ImGui::EndChild();
		//if (ImGui::BeginDragDropTarget() && vgl::Input::mouseButtonIsReleased(0)) {
		//	if (ImGui::AcceptDragDropPayload("OBJ")) {
		//		vgl::Mesh3DComponent* model = new vgl::Mesh3DComponent();
		//		model->mesh = new vgl::MeshData();
		//		vgl::EntityNameComponent* name = new vgl::EntityNameComponent;
		//		name->Name = stripExtension(m_DraggableFileName, ".obj").c_str();
		//		vgl::Transform3DComponent* new_transform = new vgl::Transform3DComponent();
		//
		//		vgl::OBJ_Loader::loadModel((m_DraggableDirectory + "/").c_str(), m_DraggableFileName.c_str(), model->mesh, false);
		//
		//		m_Scene.addEntity(*model, *name, *new_transform);
		//	}
		//	ImGui::EndDragDropTarget();
		//}

		ImGui::End();
	}

	bool onMouseMove(Event::MouseMovedEvent& p_Move) { m_CameraController.onMouseMovedEvent(p_Move); return false; };
	bool onMousePressed(Event::MouseButtonPressedEvent& p_Press) { return false; };
	bool onMouseReleased(Event::MouseButtonReleasedEvent& p_Released) { return false; };
	bool onScrollEvent(Event::MouseScrolledEvent& p_Scroll) {
		//if (windowFocused)
		m_CameraController.onMouseScrolledEvent(p_Scroll);
		return false;
	};

	bool onKeyPressed(Event::KeyPressedEvent& p_Press) { return false; };
	bool onKeyReleased(Event::KeyReleasedEvent& p_Released) { return false; };
	bool onKeyRepeated(Event::KeyRepeatedEvent& p_Repeated) { return false; };
	bool onKeyTyped(Event::KeyTypedEvent& p_Typed) { return false; };

private:
	friend class TestApp;

	vgl::Image img;
	vgl::Image m_DefaultImage;

	vgl::CameraController3D m_CameraController;
	vgl::Window* m_WindowPtr;
	vgl::Renderer* m_RendererPtr;

	vgl::RenderPipeline_Deferred m_RenderPipeline;

	vgl::Scene m_Scene;

	vgl::Viewport m_ViewportData;
	bool m_ViewportFocused = false;

	vgl::ecs::EntityHandle m_PointLightEntity = nullptr;
	vgl::ecs::EntityHandle m_DirectionalLightEntity = nullptr;
	vgl::ecs::EntityHandle m_SkyBoxEntity = nullptr;
};

class TestApp : public vgl::Application
{
public:
	TestApp(vgl::AppConfig& p_Config) : vgl::Application(p_Config) {
		m_Window.setIcon("data/CMakeResources/Vulkan.png");
	};
	~TestApp() {};

	void setup() override {
		m_TestLayer->m_WindowPtr = &m_Window;
		m_TestLayer->m_RendererPtr = &m_Renderer;

		m_Layers.pushLayer(m_TestLayer);
	}

private:
	TestLayer* m_TestLayer = new TestLayer;
};

int main()
{
	vgl::AppConfig config;
	config.DefaultWindowSize = { 1280, 720 };
	config.MSAASamples = 8;
	config.Title = "VGL";
	config.ImGui_Font_Path = "../data/Fonts/OpenSans-Regular.ttf";
	auto app = new TestApp(config);
	//auto app = new vgl::Editor(config);

	app->run();

}
