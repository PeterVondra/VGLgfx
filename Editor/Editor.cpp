#include "Editor.h"

#include "../lib/ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h"

namespace vgl
{
	std::string stripExtension(std::string& path, std::string p_Ext)
	{
		std::string stripped = path;
		int dot = stripped.rfind(p_Ext);
		if (dot != std::string::npos)
			stripped.resize(dot);
		return stripped;
	}

	Editor::Editor(AppConfig& p_AppConfig) : Application(p_AppConfig)
	{
		m_Window.setIcon("data/CMakeResources/Vulkan.png");
		
	}
	void Editor::setup()
	{
		e3Dlayer->m_WindowPtr = &m_Window;
		e3Dlayer->m_RendererPtr = &m_Renderer;

		m_Layers.pushLayer(e3Dlayer);
	}
	void E3DLayer::onAttach()
	{
		asset_directory = std::filesystem::path("projects");
		current_directory = std::filesystem::path(asset_directory);
		
		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".cpp", ImVec4(0.5, 1, 0, 1));
		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".h", ImVec4(0, 1, 0, 1));
		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".hpp", ImVec4(0, 0, 1, 1));
		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".md", ImVec4(1, 0, 1, 1));
		igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".txt", ImVec4(1, 0.5, 0.8, 1));

		m_Camera = Camera(Vector3f(0.0f, 1.0f, 0.0f));
		m_Camera.setView(View::First_Person);
		m_CameraController.bind(m_Camera);
		m_CameraController.setDeltaTime(m_WindowPtr->getDeltaTime());
		m_CameraController.enable(false);

		ImageLoader::getImageFromPath(m_BRDFImage, "data/textures/ibl_brdf_lut.png", SamplerMode::ClampToEdge);

		ImageLoader::getImageFromPath(m_DefaultImage, "data/textures/default.png", SamplerMode::ClampToBorder, Filter::Nearest);
		ImageLoader::getImageFromPath(m_EntityIcon, "data/textures/cube.png", SamplerMode::ClampToBorder, Filter::Nearest);
		ImageLoader::getImageFromPath(m_MaterialIcon, "data/textures/mtl.png", SamplerMode::ClampToBorder, Filter::Nearest);
		ImageLoader::getImageFromPath(m_TransformIcon, "data/textures/transform.png", SamplerMode::ClampToBorder, Filter::Nearest);

		ImageLoader::getImageFromPath(m_OBJIcon, "data/textures/obj.png", SamplerMode::ClampToBorder, Filter::Nearest);
		ImageLoader::getImageFromPath(m_PNGIcon, "data/textures/png.png", SamplerMode::ClampToBorder, Filter::Nearest);
		ImageLoader::getImageFromPath(m_JPGIcon, "data/textures/jpg.png", SamplerMode::ClampToBorder, Filter::Nearest);
		ImageLoader::getImageFromPath(m_HDRIcon, "data/textures/hdr.png", SamplerMode::ClampToBorder, Filter::Nearest);
		ImageLoader::getImageFromPath(m_FOLDERIcon, "data/textures/folder.png", SamplerMode::ClampToBorder, Filter::Nearest);
		ImageLoader::getImageFromPath(m_SCENEIcon, "data/textures/scene.png", SamplerMode::ClampToBorder, Filter::Nearest);
		ImageLoader::getImageFromPath(m_FILEIcon, "data/textures/file.png", SamplerMode::ClampToBorder, Filter::Nearest);

		// Scene

		auto directional_light = new DirectionalLight3DComponent;
		directional_light->Color = { 7, 7, 7 };
		directional_light->Direction = { 0.333, 1, 0.333 };

		auto shadow_map = new DShadowMapComponent;

		//shadow_map->ShadowMap.m_DepthBiasConstant = 1.00005;
		//shadow_map->ShadowMap.m_DepthBiasSlope = 1.0005;
		//shadow_map->ShadowMap.m_Direction = directional_light->Direction;
		//shadow_map->ShadowMap.m_Resolution = { int32_t(4096*1.5), int32_t(4096*1.5) };
		//shadow_map->ShadowMap.m_Projection = Matrix4f::orthoRH_ZO(-3000, 3000, -3000, 3000, 1, 4800);
		////shadow_map->ShadowMap.m_Projection = Matrix4f::perspectiveRH_ZO(45, 1, 1, 4800);
		//
		//// Create shadow map framebuffer
		//shadow_map->ShadowMap.m_Attachment.m_FramebufferAttachmentInfo.p_RectangleData = const_cast<VertexArray*>(&GraphicsContext::getRecVao());
		//shadow_map->ShadowMap.m_Attachment.m_FramebufferAttachmentInfo.p_Size = shadow_map->ShadowMap.m_Resolution;
		//shadow_map->ShadowMap.m_Attachment.m_FramebufferAttachmentInfo.p_AttachmentDescriptors.emplace_back(
		//	shadow_map->ShadowMap.m_Resolution,
		//	ImageFormat::D16UN_1C,
		//	Layout::DepthR,
		//	BorderColor::OpaqueWhite,
		//	SamplerMode::ClampToEdge, true, true
		//);
		//shadow_map->ShadowMap.m_Attachment.create();

		m_DirectionalLightEntity = m_Scene.addEntity(*directional_light, *shadow_map, *(new EntityNameComponent("Directional Light")));

		m_SkyBoxEntity = m_Scene.addEntity(*(new SkyboxComponent), *(new EntityNameComponent("SkyBox")));

		m_Scene.getComponent<SkyboxComponent>(m_SkyBoxEntity)->_AtmosphericScatteringInfo = {};
		m_Scene.getComponent<SkyboxComponent>(m_SkyBoxEntity)->_AtmosphericScattering = true;
		//GraphicsContext::generateAtmosphericScatteringCubeMap({ 1024, 1024 }, m_Scene.getComponent<SkyboxComponent>(m_SkyBoxEntity)->AtmosphericScatteringInfo);

		m_RenderPipeline.setup(m_RendererPtr, m_WindowPtr, &m_Scene);

	}
	void E3DLayer::onDetach()
	{
		m_Scene.serialize_yaml("sandbox.vgls");

	}
	void E3DLayer::onUpdate(Renderer& p_Renderer)
	{
		if (vgl::Input::keyIsPressed(vgl::Key::F11))
			m_WindowPtr->toggleFullscreen();

		float rotSpeed = m_WindowPtr->getDeltaTime()/1.2;

		auto shadow_map = m_Scene.getComponent<DShadowMapComponent>(m_DirectionalLightEntity);

		//if (Input::keyIsDown(Key::Left))
		//	shadow_map->ShadowMap.m_Direction = Math::rotate(shadow_map->ShadowMap.m_Direction, { 0, 1, 0 }, rotSpeed);
		//else if (Input::keyIsDown(Key::Right))
		//	shadow_map->ShadowMap.m_Direction = Math::rotate(shadow_map->ShadowMap.m_Direction, { 0, 1, 0 }, -rotSpeed);
		//if (Input::keyIsDown(Key::Up))
		//	shadow_map->ShadowMap.m_Direction = Math::rotate(shadow_map->ShadowMap.m_Direction, { 0, 0, 1 }, rotSpeed);
		//else if (Input::keyIsDown(Key::Down))
		//	shadow_map->ShadowMap.m_Direction = Math::rotate(shadow_map->ShadowMap.m_Direction, { 0, 0, 1 }, -rotSpeed);

		//shadow_map->ShadowMap.m_View = Matrix4f::lookAtRH(shadow_map->ShadowMap.m_Direction*3000, { 0,0,0 }, { 0, 1, 0 });

		//if(Input::keyIsPressed(Key::R))
		//	shadow_map->ShadowMap.m_Direction = { 0.333, 1, 0.333 };

		m_CameraController.setDeltaTime(m_WindowPtr->getDeltaTime());
		m_CameraController.update();

		//m_Scene.getComponent<SkyboxComponent>(m_SkyBoxEntity)->AtmosphericScatteringInfo.p_SunPos = shadow_map->ShadowMap.m_Direction;

		RenderInfo info;
		info.p_AlphaBlending = true;
		info.p_CullMode = CullMode::BackBit;
		info.p_IATopology = IATopoogy::TriList;
		info.p_PolygonMode = PolygonMode::Fill;

		m_RenderPipeline.render(info);
	}
	void E3DLayer::onImGuiUpdate()
	{
		static std::string time = "";

		if (m_WindowPtr->onTick())
			time = "FPS: " + std::to_string((int)(1 / (m_WindowPtr->getDeltaTime()))) + "   MS: " + std::to_string(m_WindowPtr->getDeltaTime() * 1000);

		ImGui::DockSpaceOverViewport();
		if (m_ShaderHierarchyUI) {
			ImGui::Begin("Shaders", &m_ShaderHierarchyUI);
			ImGuiRowsBackground();
			if (ImGui::TreeNode("UberShaderPBR")) {
				if (ImGui::BeginPopupContextWindow("UberShaderPBR"))
					if (ImGui::MenuItem("Refresh"))
						for (auto& entity : m_Scene.getEntities()) {
							Mesh3DComponent* mesh = m_Scene.getComponent<Mesh3DComponent>(entity);
							if (!mesh) continue;
							if (mesh->mesh)
								mesh->mesh->m_RecreateFlag = true;
						}
				ImGui::TreePop();
			}
			ImGui::End();
		}

		auto shadow_map = m_Scene.getComponent<DShadowMapComponent>(m_DirectionalLightEntity);

		ImGui::Begin("Shadow Map", nullptr, ImGuiWindowFlags_NoDecoration);
		//ImGuiContext::Image(shadow_map->ShadowMap.m_Attachment.getImageAttachment()[m_RendererPtr->getImageIndex()][0].getImage(), ImGui::GetContentRegionAvail());
		//ImGuiContext::Image(m_RenderPipeline.m_SSRFramebuffer.getImageAttachment()[m_RendererPtr->getImageIndex()][0].getImage(), ImGui::GetContentRegionAvail());
		ImGui::End();
	
		ImGui::Begin("GBuffer", nullptr);
		ImGuiContext::Image(m_RenderPipeline.m_GBuffer.getCurrentImageAttachments()[0].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		ImGuiContext::Image(m_RenderPipeline.m_GBuffer.getCurrentImageAttachments()[1].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		ImGuiContext::Image(m_RenderPipeline.m_GBuffer.getCurrentImageAttachments()[2].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		ImGuiContext::Image(m_RenderPipeline.m_GBuffer.getCurrentImageAttachments()[3].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		ImGui::End();

		ImGui::Begin("LightPass", nullptr);
		ImGuiContext::Image(m_RenderPipeline.m_LightPassFramebuffer.getCurrentImageAttachments()[0].getImage(), { (float)ImGui::GetContentRegionAvail().x, (float)ImGui::GetContentRegionAvail().x });
		ImGui::End();

		//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 2.0f, 2.0f });
		ImGui::PushStyleColor(ImGuiCol_ChildBg, { (float)230/256, (float)180/256, (float)66/256, 1 });
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
		ImGuiContext::Image(m_RenderPipeline.m_HDRFramebuffer.getCurrentImageAttachments()[0].getImage(), { (float)m_ViewportData.m_Size.x, (float)m_ViewportData.m_Size.y });
		m_ViewportFocused = ImGui::IsWindowFocused();
		m_Camera.setAspectRatio((float)m_ViewportData.m_Size.x / m_ViewportData.m_Size.y);
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

		cameraEdit();

		if (igfd::ImGuiFileDialog::Instance()->FileDialog("fobj10", 32, { 600, 250 }))
		{
			if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
			{
				Mesh3DComponent* model = new Mesh3DComponent();
				model->mesh = new MeshData();
				EntityNameComponent* name = new EntityNameComponent();
				Transform3DComponent* new_transform = new Transform3DComponent();

				std::string fileName = igfd::ImGuiFileDialog::Instance()->GetCurrentFileName();
				std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath() + "/";

				OBJ_Loader::loadModel(filePath.c_str(), fileName.c_str(), model->mesh, false);
				name->Name = stripExtension(model->mesh->m_FileName, ".obj").c_str();

				m_Scene.addEntity(*model, *name, *new_transform);
			}
			igfd::ImGuiFileDialog::Instance()->CloseDialog("fobj10");
		}
		if (igfd::ImGuiFileDialog::Instance()->FileDialog("fobj10InvUV", 32, { 600, 250 }))
		{
			if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
			{
				Mesh3DComponent* model = new Mesh3DComponent();
				model->mesh = new MeshData();
				EntityNameComponent* name = new EntityNameComponent();
				Transform3DComponent* new_transform = new Transform3DComponent();

				std::string fileName = igfd::ImGuiFileDialog::Instance()->GetCurrentFileName();
				std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath() + "/";

				OBJ_Loader::loadModel(filePath.c_str(), fileName.c_str(), model->mesh, true);
				name->Name = stripExtension(model->mesh->m_FileName, ".obj").c_str();

				m_Scene.addEntity(*model, *name, *new_transform);
			}
			igfd::ImGuiFileDialog::Instance()->CloseDialog("fobj10InvUV");
		}
		if (igfd::ImGuiFileDialog::Instance()->FileDialog("SceneOPEN", 32, { 600, 250 })) {
			if (igfd::ImGuiFileDialog::Instance()->IsOk == true) {
				m_Scene.deserialize_yaml(igfd::ImGuiFileDialog::Instance()->GetCurrentPath() + "/" + igfd::ImGuiFileDialog::Instance()->GetCurrentFileName());
				for (auto& entity : m_Scene.getEntities()) {
					auto d_light = m_Scene.getComponent<DirectionalLight3DComponent>(entity);
					auto skybox = m_Scene.getComponent<SkyboxComponent>(entity);

					if (d_light)
						m_DirectionalLightEntity = entity;
					if (skybox)
						m_SkyBoxEntity = entity;;
				}
			}
			igfd::ImGuiFileDialog::Instance()->CloseDialog("SceneOPEN");
		}
		if (igfd::ImGuiFileDialog::Instance()->FileDialog("SceneSAVE", 32, { 600, 250 })) {
			if (igfd::ImGuiFileDialog::Instance()->IsOk == true) {
				std::cout << igfd::ImGuiFileDialog::Instance()->GetCurrentPath() << std::endl;
				m_Scene.serialize_yaml(igfd::ImGuiFileDialog::Instance()->GetCurrentPath());
			}
			igfd::ImGuiFileDialog::Instance()->CloseDialog("SceneSAVE");
		}
		
		static std::string fileName;
		static std::string filePath;
		static bool open_text_edit = false;
		
		//if (igfd::ImGuiFileDialog::Instance()->FileDialog("Text", 32, { 600, 250 }))
		//{
		//	if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
		//	{
		//		fileName = igfd::ImGuiFileDialog::Instance()->GetCurrentFileName();
		//		filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath() + "/";
		//
		//		std::string data;
		//		Utils::FileGUI::loadFile(filePath + fileName, data);
		//
		//		m_TextEditor.SetText(data);
		//		open_text_edit = true;
		//	}
		//	igfd::ImGuiFileDialog::Instance()->Close();
		//}
		
		//if (open_text_edit) {
		//	auto cpos = m_TextEditor.GetCursorPosition();
		//	ImGui::Begin("Text Editor", &open_text_edit, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
		//	ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
		//	if (ImGui::BeginMenuBar()) {
		//		if (ImGui::BeginMenu("View"))
		//		{
		//			if (ImGui::MenuItem("Dark palette"))
		//				m_TextEditor.SetPalette(TextEditor::GetDarkPalette());
		//			if (ImGui::MenuItem("Light palette"))
		//				m_TextEditor.SetPalette(TextEditor::GetLightPalette());
		//			if (ImGui::MenuItem("Retro blue palette"))
		//				m_TextEditor.SetPalette(TextEditor::GetRetroBluePalette());
		//			ImGui::EndMenu();
		//		}
		//		ImGui::EndMenuBar();
		//	}
		//
		//	if (Input::keyIsDown(Key::LeftControl) && Input::keyIsPressed(Key::S) && ImGui::IsWindowFocused())
		//		Utils::FileGUI::writeToFile(filePath + fileName, m_TextEditor.GetText());
		//
		//	m_TextEditor.Render("TextEditor");
		//	ImGui::End();
		//}

		//ImGui::BeginMainMenuBar_S("##Main", 25, ImGui::Placing_S::Top);
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open")) {
				igfd::ImGuiFileDialog::Instance()->OpenDialog("Text", "Script File", "{.txt,.cpp,.h,.glsl,.vgls}", ".");
			}
			if (ImGui::MenuItem("Open Scene")) {
				igfd::ImGuiFileDialog::Instance()->OpenModal("SceneOPEN", "Scene File(vgls)", "{.vgls}", ".");
			}
			if (ImGui::MenuItem("Save Scene")) {
				if(!m_ScenePath.empty())
					m_Scene.serialize_yaml(m_ScenePath);
				else
					m_Scene.serialize_yaml(current_directory.string() + "/Untitled.vgls");
			}
			if (ImGui::MenuItem("Save Scene as...")) {
				igfd::ImGuiFileDialog::Instance()->OpenDialog("SceneSAVE", "Directory", "", ".");
			}

			if (ImGui::BeginMenu("Import")) {
				if (ImGui::MenuItem("Wavefront(.obj)"))
					igfd::ImGuiFileDialog::Instance()->OpenModal("fobj10", "Wavefront OBJ", "{.obj}", ".");
				if (ImGui::MenuItem("Wavefront(.obj) - Flipped UV"))
					igfd::ImGuiFileDialog::Instance()->OpenModal("fobj10InvUV", "Wavefront OBJ", "{.obj}", ".");
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Exit")) {}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Shader Hierarchy"))
				m_ShaderHierarchyUI = true;
			if (ImGui::MenuItem("Scene Hierarchy"))
				m_SceneHierarchyUI = true;
			if (ImGui::MenuItem("Content Browser"))
				m_ContentBrowserUI = true;
			if (ImGui::MenuItem("Editor Camera Panel"))
				m_EditorCameraUI = true;
			if (ImGui::BeginMenu("Graphics")) {
				if (ImGui::MenuItem("Post-Processing Panel"))
					m_PostProcessingUI = true;
				if (ImGui::MenuItem("Depth Of Field Panel"))
					m_DepthOfFieldUI = true;
				if (ImGui::MenuItem("Atmospheric Scattering Panel"))
					m_AtmosphereUI = true;
				if (ImGui::MenuItem("Environment Panel"))
					m_SkyBoxUI = true;
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		//ImGui::EndMainMenuBar_S();
		ImGui::EndMainMenuBar();
		

		//ImGui::BeginMainMenuBar_S("##Status", 25, ImGui::Placing_S::Bottom);
		//ImGui::BeginMainMenuBar();
		//ImGui::Text(time.c_str());
		//ImGui::EndMainMenuBar();
		
		if(m_PostProcessingUI) post_process_UI();
		if(m_DepthOfFieldUI) dof_UI();
		if(m_SceneHierarchyUI) sceneDisplay();
		if(m_SkyBoxUI) skybox_UI();
		if(m_ContentBrowserUI) contentBrowser();
	}

	void E3DLayer::transformEdit(Transform3DComponent* p_TransformComponent, const int p_ID, bool p_Translation, bool p_Rotation, bool p_Scaling)
	{
		if (!p_Translation && !p_Rotation && !p_Scaling)
			return;
		if (ImGui::TreeNodeEx(("Transform##" + std::to_string(p_ID)).c_str(), EditorColorScheme::TreeNodeFlagsFramed)) {
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
			if (p_Translation) DrawVec3Control("Translation", matrixTranslation);
			if (p_Rotation) DrawVec3Control("Rotation", matrixRotation);
			if (p_Scaling) DrawVec3Control("Scale", matrixScale, 1.0f);

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

	void E3DLayer::sceneDisplay()
	{
		ImGui::Begin("Scene", &m_SceneHierarchyUI);
		ImGui::BeginChild("SCENE-HIERARCHY");
		ImGuiRowsBackground();
		static int32_t C0unt = 0;
		if (ImGui::BeginPopupContextWindow("CNE")) {
			if (ImGui::MenuItem(("Create New Entity##" + std::to_string(C0unt)).c_str())) {
				EntityNameComponent* name = new EntityNameComponent();
				name->Name += " "+std::to_string(C0unt);
				C0unt++;
				m_Scene.addEntity(*name);
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupContextWindow("CNPL")) {
			if (ImGui::MenuItem(("Create New Point Light##p" + std::to_string(C0unt)).c_str())) {
				EntityNameComponent* name = new EntityNameComponent();
				PointLight3DComponent* light = new PointLight3DComponent();

				name->Name = "Point Light " + std::to_string(C0unt);
				C0unt++;
				m_Scene.addEntity(*name, *light);
			}
			ImGui::EndPopup();
		}

		static bool openGridEdit = false;
		
		static int entity_idx = -1;

		for (int i = 0; i < m_Scene.getEntities().size(); i++) {
			auto* mesh = m_Scene.getComponent<Mesh3DComponent>(m_Scene.getEntities()[i]);
			auto* name = m_Scene.getComponent<EntityNameComponent>(m_Scene.getEntities()[i]);
			auto* transform = m_Scene.getComponent<Transform3DComponent>(m_Scene.getEntities()[i]);
			auto* directional_light = m_Scene.getComponent<DirectionalLight3DComponent>(m_Scene.getEntities()[i]);
			auto* point_light = m_Scene.getComponent<PointLight3DComponent>(m_Scene.getEntities()[i]);

			float y = ImGui::GetTextLineHeight();
			ImGuiContext::Image(m_EntityIcon, { y,y });
			ImGui::SameLine();
			bool open = ImGui::TreeNodeEx(name->Name.c_str());
			
			if (open) {
				if (directional_light) {
					ImGui::Begin("Component Properties");
					if (ImGui::TreeNodeEx(name->Name.c_str(), EditorColorScheme::TreeNodeFlags)) {
						ImGui::ColorPicker3("Color", directional_light->Color.get(), ImGuiColorEditFlags_DisplayRGB);
						ImGui::DragFloat("Mie Scatter Constant (Volumetric Lighting)", &directional_light->VolumetricLightDensity, 1e-2);
						directional_light->VolumetricLightDensity = 0.00001 + std::abs(directional_light->VolumetricLightDensity);
						ImGui::TreePop();
					}
					ImGui::End();
				}
				if (point_light) {
					ImGui::Begin("Component Properties");
					if (ImGui::TreeNodeEx(name->Name.c_str(), EditorColorScheme::TreeNodeFlags)) {

						ImGui::ColorPicker3("Color", point_light->Color.get(), ImGuiColorEditFlags_DisplayRGB);

						static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
						static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

						Vector3f matrixTranslation, matrixRotation, matrixScale;
						ImGuizmo::DecomposeMatrixToComponents(&point_light->Transform[0], matrixTranslation.get(), matrixRotation.get(), matrixScale.get());
						DrawVec3Control("Translation", matrixTranslation);
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
					ImGuiContext::Image(m_TransformIcon, { y,y });
					ImGui::SameLine();
					if (ImGui::TreeNodeEx("Transform", EditorColorScheme::TreeNodeFlags)) {
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
						ImGuiContext::Image(m_MaterialIcon, { y,y });
						ImGui::SameLine();
						std::string name = mesh->mesh->getMaterial(j).m_Name != "" ? mesh->mesh->getMaterial(j).m_Name : "Material " + std::to_string(j);
						if (ImGui::TreeNodeEx(name.c_str(), EditorColorScheme::TreeNodeFlags)) {
							ImGui::Begin("Component Properties");
							
							ImGui::Checkbox(("##" + name).c_str(), &mesh->mesh->render(j));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip((std::string("Render=") + (mesh->mesh->render(j) ? "on" : "off")).c_str());
							
							ImGui::SameLine();
							if (ImGui::TreeNodeEx((name + "##" + std::to_string(i)).c_str(), EditorColorScheme::TreeNodeFlagsFramed)) {
								ImGui::Text("Metallic");
								ImGui::SliderFloat("##4", &mesh->mesh->getMaterial(j).config.m_Metallic, 0.02f, 1.0f);
								ImGui::Text("Roughness");
								ImGui::SliderFloat("##5", &mesh->mesh->getMaterial(j).config.m_Roughness, 0.02f, 1.0f);
								ImGui::Text("PDM Depth");
								ImGui::SliderFloat("##6", &mesh->mesh->getMaterial(j).config.m_PDMDepth, 0.02f, 1.0f);

								ImGui::PushItemWidth(128);

								if (ImGui::TreeNodeEx("Albedo Color", EditorColorScheme::TreeNodeFlags)) {
									ImGui::ColorPicker3("Albedo", mesh->mesh->getMaterial(j).config.m_Albedo.get());
									ImGui::TreePop();
								}
								
								if (ImGui::TreeNodeEx("Albedo Map", EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_AlbedoMap.isValid())
										ImGuiContext::Image(mesh->mesh->getMaterial(j).m_AlbedoMap, { 64, 64 });
									else
										ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									if (ImGui::BeginDragDropTarget() && Input::mouseButtonIsReleased(0)) {
										if (ImGui::AcceptDragDropPayload("Image")) {
											if (mesh->mesh->getMaterial(j).m_AlbedoMap.isValid())
												mesh->mesh->getMaterial(j).m_AlbedoMap.destroy();

											ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_AlbedoMap, m_Draggable.c_str(), SamplerMode::Repeat);

											mesh->mesh->m_MTLRecreateFlag = true;
										}
										ImGui::EndDragDropTarget();
									}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_AlbedoMap.isValid())
											mesh->mesh->getMaterial(j).m_AlbedoMap.destroy();

										ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_AlbedoMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_AlbedoMap.isValid()) {
											mesh->mesh->getMaterial(j).m_AlbedoMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								if (ImGui::TreeNodeEx("Normal Map", EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_NormalMap.isValid())
										ImGuiContext::Image(mesh->mesh->getMaterial(j).m_NormalMap, { 64, 64 });
									else
										ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									if (ImGui::BeginDragDropTarget() && Input::mouseButtonIsReleased(0)) {
										if (ImGui::AcceptDragDropPayload("Image")) {
											if (mesh->mesh->getMaterial(j).m_NormalMap.isValid())
												mesh->mesh->getMaterial(j).m_NormalMap.destroy();

											ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_NormalMap, m_Draggable.c_str(), SamplerMode::Repeat);

											mesh->mesh->m_MTLRecreateFlag = true;
										}
										ImGui::EndDragDropTarget();
									}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_NormalMap.isValid())
											mesh->mesh->getMaterial(j).m_NormalMap.destroy();

										ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_NormalMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_NormalMap.isValid()) {
											mesh->mesh->getMaterial(j).m_NormalMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								if (ImGui::TreeNodeEx("Displacement Map", EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_DisplacementMap.isValid())
										ImGuiContext::Image(mesh->mesh->getMaterial(j).m_DisplacementMap, { 64, 64 });
									else
										ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									if (ImGui::BeginDragDropTarget() && Input::mouseButtonIsReleased(0)) {
										if (ImGui::AcceptDragDropPayload("Image")) {
											if (mesh->mesh->getMaterial(j).m_DisplacementMap.isValid())
												mesh->mesh->getMaterial(j).m_DisplacementMap.destroy();

											ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_DisplacementMap, m_Draggable.c_str(), SamplerMode::Repeat);

											mesh->mesh->m_MTLRecreateFlag = true;
										}
										ImGui::EndDragDropTarget();
									}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_DisplacementMap.isValid())
											mesh->mesh->getMaterial(j).m_DisplacementMap.destroy();

										ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_DisplacementMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_DisplacementMap.isValid()) {
											mesh->mesh->getMaterial(j).m_DisplacementMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								if (ImGui::TreeNodeEx("Metallic Map", EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_MetallicMap.isValid())
										ImGuiContext::Image(mesh->mesh->getMaterial(j).m_MetallicMap, { 64, 64 });
									else
										ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									if (ImGui::BeginDragDropTarget() && Input::mouseButtonIsReleased(0)) {
										if (ImGui::AcceptDragDropPayload("Image")) {
											if (mesh->mesh->getMaterial(j).m_MetallicMap.isValid())
												mesh->mesh->getMaterial(j).m_MetallicMap.destroy();

											ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_MetallicMap, m_Draggable.c_str(), SamplerMode::Repeat);

											mesh->mesh->m_MTLRecreateFlag = true;
										}
										ImGui::EndDragDropTarget();
									}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_MetallicMap.isValid())
											mesh->mesh->getMaterial(j).m_MetallicMap.destroy();

										ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_MetallicMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_MetallicMap.isValid()) {
											mesh->mesh->getMaterial(j).m_MetallicMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								if (ImGui::TreeNodeEx("Roughness Map", EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_RoughnessMap.isValid())
										ImGuiContext::Image(mesh->mesh->getMaterial(j).m_RoughnessMap, { 64, 64 });
									else
										ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									if (ImGui::BeginDragDropTarget() && Input::mouseButtonIsReleased(0)) {
										if (ImGui::AcceptDragDropPayload("Image")) {
											if (mesh->mesh->getMaterial(j).m_RoughnessMap.isValid())
												mesh->mesh->getMaterial(j).m_RoughnessMap.destroy();

											ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_RoughnessMap, m_Draggable.c_str(), SamplerMode::Repeat);

											mesh->mesh->m_MTLRecreateFlag = true;
										}
										ImGui::EndDragDropTarget();
									}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_RoughnessMap.isValid())
											mesh->mesh->getMaterial(j).m_RoughnessMap.destroy();

										ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_RoughnessMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), SamplerMode::Repeat);

										mesh->mesh->m_MTLRecreateFlag = true;
									}
									if (ImGui::Button("Remove"))
										if (mesh->mesh->getMaterial(j).m_RoughnessMap.isValid()) {
											mesh->mesh->getMaterial(j).m_RoughnessMap.destroy();
											mesh->mesh->m_MTLRecreateFlag = true;
										}

									ImGui::TreePop();
								}
								if (ImGui::TreeNodeEx("AO Map", EditorColorScheme::TreeNodeFlagsFramed)) {
									if (mesh->mesh->getMaterial(j).m_AOMap.isValid())
										ImGuiContext::Image(mesh->mesh->getMaterial(j).m_AOMap, { 64, 64 });
									else
										ImGuiContext::Image(m_DefaultImage, { 64, 64 });

									if (ImGui::BeginDragDropTarget() && Input::mouseButtonIsReleased(0)) {
										if (ImGui::AcceptDragDropPayload("Image")) {
											if (mesh->mesh->getMaterial(j).m_AOMap.isValid())
												mesh->mesh->getMaterial(j).m_AOMap.destroy();

											ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_AOMap, m_Draggable.c_str(), SamplerMode::Repeat);

											mesh->mesh->m_MTLRecreateFlag = true;
										}
										ImGui::EndDragDropTarget();
									}

									ImGui::SameLine();

									if (ImGui::Button("...")) {
										if (mesh->mesh->getMaterial(j).m_AOMap.isValid())
											mesh->mesh->getMaterial(j).m_AOMap.destroy();

										ImageLoader::getImageFromPath(mesh->mesh->getMaterial(j).m_AOMap, Utils::FileGUI::getFileDialog({ "jpg", "png", "bmp" }), SamplerMode::Repeat);

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
							Mesh3DComponent* new_mesh = new Mesh3DComponent();
							Transform3DComponent* new_transform = new Transform3DComponent();
							new_mesh->mesh = new MeshData();
							new_mesh->mesh->m_FileName = "Sphere";

							generateUVSphere(50, 50, *new_mesh->mesh);
							new_transform->transform.setScale(100);

							m_Scene.addComponent(m_Scene.getEntities()[i], new_mesh);
							m_Scene.addComponent(m_Scene.getEntities()[i], new_transform);
							ImGui::EndPopup();
						}
						else if (ImGui::MenuItem("Cube")) {
							Mesh3DComponent* new_mesh = new Mesh3DComponent();
							Transform3DComponent* new_transform = new Transform3DComponent();
							new_mesh->mesh = new MeshData();
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
						}else ImGui::EndPopup();
						ImGui::EndMenu();
					} else ImGui::EndPopup();
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
					Mesh3DComponent* model = new Mesh3DComponent();
					Transform3DComponent* new_transform = new Transform3DComponent();
					model->mesh = new MeshData();
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
		if (ImGui::BeginDragDropTarget() && Input::mouseButtonIsReleased(0)) {
			if (ImGui::AcceptDragDropPayload("OBJ")) {
				Mesh3DComponent* model = new Mesh3DComponent();
				model->mesh = new MeshData();
				EntityNameComponent* name = new EntityNameComponent;
				name->Name = stripExtension(m_DraggableFileName, ".obj").c_str();
				Transform3DComponent* new_transform = new Transform3DComponent();

				OBJ_Loader::loadModel((m_DraggableDirectory + "/").c_str(), m_DraggableFileName.c_str(), model->mesh, false);

				m_Scene.addEntity(*model, *name, *new_transform);
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::End();
	}
	void E3DLayer::contentBrowser()
	{
		/*ImGui::Begin("Content Browser", &m_ContentBrowserUI, ImGuiWindowFlags_MenuBar);
		static std::filesystem::path prev_directory(asset_directory);
		static std::filesystem::path relative_directory;

		static std::pair<Image, std::string> image;

		ImGui::BeginMenuBar();
		if (ImGui::Button("<")) {
			if (current_directory != std::filesystem::path(asset_directory)) {
				prev_directory = current_directory;
				current_directory = current_directory.parent_path();
			}
		}
		else if (ImGui::Button(">")) {
			current_directory = prev_directory;
		}
		ImGui::Spacing();
		ImGui::Text(std::filesystem::relative(current_directory, asset_directory).string().c_str());
		ImGui::EndMenuBar();
		
		int ID = 0;
		for (auto& p : std::filesystem::directory_iterator(current_directory))
		{
			relative_directory = std::filesystem::relative(p.path(), asset_directory);
			std::string filename = relative_directory.filename().string();

			ImVec2 button_sz(96, 96);
			ImGui::ItemWrapBegin(ID);

			if (p.is_directory()) {
				if (ImGui::ImageButtonWithText(&m_FOLDERIcon, filename.c_str(), button_sz)) {
					
				}
				if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {
					current_directory /= p.path().filename();
					prev_directory = current_directory;
				}
			}
			else {
				std::string extension = p.path().filename().extension().string();

				bool dragged = false;

				if (extension == ".jpg") {
					if (ImGui::ImageButtonWithText(&m_JPGIcon, filename.c_str(), button_sz)) {

					}
					if (ImGui::BeginDragDropSource()) {
						dragged = true;
						ImGui::ImageButtonWithText(&m_JPGIcon, filename.c_str(), button_sz);
						m_Draggable = p.path().string();
						ImGui::SetDragDropPayload("Image", &m_Draggable, sizeof(m_Draggable));
						ImGui::EndDragDropSource();
					}
				}
				else if (extension == ".png") {
					if (ImGui::ImageButtonWithText(&m_PNGIcon, filename.c_str(), button_sz)) {
					
					}
					if (ImGui::BeginDragDropSource()) {
						dragged = true;
						ImGui::ImageButtonWithText(&m_PNGIcon, filename.c_str(), button_sz);
						m_Draggable = p.path().string();
						ImGui::SetDragDropPayload("Image", &m_Draggable, sizeof(m_Draggable));
						ImGui::EndDragDropSource();
					}
				}
				else if (extension == ".hdr") {
					if (ImGui::ImageButtonWithText(&m_HDRIcon, filename.c_str(), button_sz)) {

					}
					if (ImGui::BeginDragDropSource()) {
						dragged = true;
						ImGui::ImageButtonWithText(&m_HDRIcon, filename.c_str(), button_sz);
						m_Draggable = p.path().string();
						ImGui::SetDragDropPayload("Image", &m_Draggable, sizeof(m_Draggable));
						ImGui::EndDragDropSource();
					}
				}
				else if (extension == ".obj") {
					if (ImGui::ImageButtonWithText(&m_OBJIcon, filename.c_str(), button_sz)) {

					}
					if (ImGui::BeginDragDropSource()) {
						ImGui::ImageButtonWithText(&m_OBJIcon, filename.c_str(), button_sz);
						m_Draggable = p.path().string();
						m_DraggableFileName = p.path().filename().string();
						m_DraggableDirectory = p.path().parent_path().string();
						ImGui::SetDragDropPayload("OBJ", &m_Draggable, sizeof(m_Draggable));
						ImGui::EndDragDropSource();
					}
				}
				else if (extension == ".vgls") {
					if (ImGui::ImageButtonWithText(&m_SCENEIcon, filename.c_str(), button_sz)) {

					}
					if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {
						m_ScenePath = p.path().string();
						m_Scene.deserialize_yaml(p.path().string());
						for (auto& entity : m_Scene.getEntities()) {
							auto d_light = m_Scene.getComponent<DirectionalLight3DComponent>(entity);
							auto skybox = m_Scene.getComponent<SkyboxComponent>(entity);
							
							if (d_light) m_DirectionalLightEntity = entity;
							if (skybox) m_SkyBoxEntity = entity;;
						}
					}
					//if (ImGui::BeginDragDropSource()) {
					//	ImGui::ImageButtonWithText(&m_SCENEIcon, filename.c_str(), button_sz);
					//	m_Draggable = p.path().string();
					//	m_DraggableFileName = p.path().filename().string();
					//	m_DraggableDirectory = p.path().parent_path().string();
					//	ImGui::SetDragDropPayload("OBJ", &m_Draggable, sizeof(m_Draggable));
					//	ImGui::EndDragDropSource();
					//}
				}else if (ImGui::ImageButtonWithText(&m_FILEIcon, filename.c_str(), button_sz)) {

				}
				
				if (ImGui::BeginPopupContextWindow(p.path().string().c_str())) {
					if (ImGui::MenuItem(("Delete##" + p.path().string()).c_str())) {
						std::filesystem::remove(p.path());
					}
					ImGui::EndPopup();
				}else if (!dragged && (extension == ".png" || extension == ".jpg" || extension == ".bmp" || extension == ".hdr")) {
					if (ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();

						if (image.second != p.path().string()) {
							if (image.first.isValid()) {
								image.first.destroy();
								image.second = "";
							}

							unsigned char* imgData = nullptr;

							imgData = ImageLoader::getImageDataFromPath(image.first, p.path().string().c_str());
							image.second = p.path().string();
							if (imgData != nullptr) {
								image.first.init();
								image.first.freeImageData();
							}
						}

						if (image.first.isValid())
							ImGuiContext::Image(image.first, { 128, 128 });
						ImGui::EndTooltip();
					}
				}
			}

			ImGui::ItemWrapEnd(ID, button_sz);
			ID++;
		}

		ImGui::End();*/
	}
	void E3DLayer::post_process_UI()
	{
		ImGui::Begin("Post Processing", &m_PostProcessingUI);
		if (ImGui::TreeNodeEx("HDR##S", EditorColorScheme::TreeNodeFlags)) {
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
			float offset = ImGui::GetColumnOffset();
			ImGui::NextColumn();
			ImGui::SetColumnWidth(1, ImGui::GetWindowSize().x - offset);
			ImGui::DragFloat("##113", &m_RenderPipeline.m_HDRInfo.filmicStrength, 1);
			ImGui::DragFloat("##2", &m_RenderPipeline.m_HDRInfo.filmicScale, 1);
			ImGui::DragFloat("##3", &m_RenderPipeline.m_HDRInfo.gamma, 0.01);
			ImGui::DragFloat("##4", &m_RenderPipeline.m_HDRInfo.exposure, 0.01);
			ImGui::DragFloat("##42", &m_RenderPipeline.m_HDRInfo.exposure_adapt_rate, 0.0001, 0.0001);
#undef max
#undef min
			std::max(m_RenderPipeline.m_HDRInfo.exposure_adapt_rate, 0.000001f);
			ImGui::EndColumns();
			ImGui::TreePop();
		}
		ImGui::Separator();
		if (ImGui::TreeNodeEx("FXAA##S", EditorColorScheme::TreeNodeFlags)) {
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
			static float fxaaBias = 1 / m_RenderPipeline.m_HDRInfo.FXAA_REDUCTION_BIAS;
			static float fxaaMin = 1 / m_RenderPipeline.m_HDRInfo.FXAA_REDUCTION_MIN;
			if (ImGui::DragFloat("##5", &fxaaBias, 2))
				m_RenderPipeline.m_HDRInfo.FXAA_REDUCTION_BIAS = 1.0f / fxaaBias;
			if (ImGui::DragFloat("##6", &fxaaMin, 2))
				m_RenderPipeline.m_HDRInfo.FXAA_REDUCTION_MIN = 1.0f / fxaaMin;

			ImGui::DragFloat("##7", &m_RenderPipeline.m_HDRInfo.FXAA_Span_Max, 2);
			ImGui::EndColumns();
			ImGui::TreePop();
		}
		ImGui::Separator();
		if (ImGui::TreeNodeEx("SSAO##S", EditorColorScheme::TreeNodeFlags)) {
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
		if (ImGui::TreeNodeEx("SSR##SSR", EditorColorScheme::TreeNodeFlags)) {
			ImGui::BeginColumns("##SSR", 2);
			ImGui::Text("Max Distance");
			ImGui::Spacing();
			ImGui::Text("Resolution");
			ImGui::Spacing();
			ImGui::Text("Steps");
			ImGui::Spacing();
			ImGui::Text("Thickness");
			ImGui::Spacing();
			float offset = ImGui::GetColumnOffset();
			ImGui::NextColumn();
			ImGui::SetColumnWidth(1, ImGui::GetWindowSize().x - offset);
			ImGui::DragFloat("##SSR1", &m_RenderPipeline.SSR_Data.SSR_MaxDistance, 0.01);
			ImGui::DragFloat("##SSR2", &m_RenderPipeline.SSR_Data.SSR_Resolution, 0.01);
			ImGui::DragFloat("##SSR3", &m_RenderPipeline.SSR_Data.SSR_Steps, 1);
			ImGui::DragFloat("##SSR4", &m_RenderPipeline.SSR_Data.SSR_Thickness, 0.01);
			std::max(m_RenderPipeline.m_HDRInfo.exposure_adapt_rate, 0.000001f);
			ImGui::EndColumns();
			ImGui::TreePop();
		}
		ImGui::End();
	}
	void E3DLayer::dof_UI()
	{
		ImGui::Begin("Depth of Field", &m_DepthOfFieldUI);
		ImGui::Checkbox("Auto Focus", (bool*)&m_RenderPipeline.m_DOFInfo.autofocus);
		ImGui::Separator();
		ImGui::Checkbox("Noise Dither", (bool*)&m_RenderPipeline.m_DOFInfo.noise);
		ImGui::Text("Use noise instead of pattern for sample dithering");
		ImGui::Separator();
		ImGui::Checkbox("Show Focus", (bool*)&m_RenderPipeline.m_DOFInfo.showFocus);
		ImGui::Text("Show debug focus point and focal range (red = focal point, green = focal range)");
		ImGui::Separator();
		ImGui::Checkbox("Vignette", (bool*)&m_RenderPipeline.m_DOFInfo.vignetting);
		ImGui::Text("Use optical lens vignetting?");
		ImGui::DragFloat("Vignette Out##VO", &m_RenderPipeline.m_DOFInfo.vignout, 0.01);
		ImGui::DragFloat("Vignette In##VI", &m_RenderPipeline.m_DOFInfo.vignin, 0.01);
		ImGui::DragFloat("Vignette Fade##VF", &m_RenderPipeline.m_DOFInfo.vignfade, 0.1);
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
	}
	void E3DLayer::skybox_UI()
	{
		ImGui::Begin("Environment", &m_SkyBoxUI);
		if (ImGui::Button("Load HDR image")) {
			if (m_HDRImage.isValid())
				m_HDRImage.destroy();
			if (m_HDRCubeMap.isValid())
				m_HDRCubeMap.destroy();
			if (m_IrradianceImage.isValid())
				m_IrradianceImage.destroy();
			if (m_PrefilteredImage.isValid())
				m_PrefilteredImage.destroy();

			m_CubeMapPath += Utils::FileGUI::getFileDialog({ "hdr", "png", "jpg", "bmp" }, m_CubeMapPath);

			ImageLoader::getImageFromPath(m_HDRImage, m_CubeMapPath.c_str(), SamplerMode::ClampToEdge);

			auto skybox = m_Scene.getComponent<SkyboxComponent>(m_SkyBoxEntity);
			skybox->HDR_Image_Path = m_CubeMapPath;

			//m_HDRCubeMap = GraphicsContext::getCubeMapFromHDR(m_HDRImage, { 1176, 1176 });
			//m_IrradianceImage = GraphicsContext::generateIrradianceMap(m_HDRCubeMap, { 32, 32 });
			//m_PrefilteredImage = GraphicsContext::generatePreFilteredMaps(m_HDRCubeMap, { 512, 512 });
			//skybox->EnvironmentData.p_IrradianceMap = &m_IrradianceImage;
			//skybox->EnvironmentData.p_PrefilteredMap = &m_PrefilteredImage;
			//skybox->EnvironmentData.p_BRDFLut = &m_BRDFImage;
			//
			//for (auto& entity : m_Scene.getEntities()) {
			//	Mesh3DComponent* mesh = m_Scene.getComponent<Mesh3DComponent>(entity);
			//	if (!mesh) continue;
			//	if (mesh->mesh)
			//		mesh->mesh->m_MTLRecreateFlag = true;
			//}
			//
			//skybox->SkyBox.create(m_HDRCubeMap);
		}
		/*if (ImGui::Button("Refresh")) {
			if (m_HDRImage.isValid())
				m_HDRImage.destroy();
			if (m_HDRCubeMap.isValid())
				m_HDRCubeMap.destroy();
			if (m_IrradianceImage.isValid())
				m_IrradianceImage.destroy();
			if (m_PrefilteredImage.isValid())
				m_PrefilteredImage.destroy();

			auto skybox = m_Scene.getComponent<SkyboxComponent>(m_SkyBoxEntity);
			m_HDRCubeMap = GraphicsContext::generateAtmosphericScatteringCubeMap({ 2048, 2048 }, skybox->AtmosphericScatteringInfo);

			m_IrradianceImage = GraphicsContext::generateIrradianceMap(m_HDRCubeMap, { 32, 32 });

			m_PrefilteredImage = GraphicsContext::generatePreFilteredMaps(m_HDRCubeMap, { 512, 512 });

			skybox->EnvironmentData.p_IrradianceMap = &m_IrradianceImage;
			skybox->EnvironmentData.p_PrefilteredMap = &m_PrefilteredImage;
			skybox->EnvironmentData.p_BRDFLut = &m_BRDFImage;

			for (auto& entity : m_Scene.getEntities()) {
				Mesh3DComponent* mesh = m_Scene.getComponent<Mesh3DComponent>(entity);
				if (!mesh) continue;
				if (mesh->mesh)
					mesh->mesh->m_MTLRecreateFlag = true;
			}

			skybox->SkyBox.create();
		}*/
		ImGui::TextWrapped(m_CubeMapPath.c_str());
		if (m_HDRImage.isValid())
			ImGuiContext::Image(m_HDRImage, { 256, 128 });
		else
			ImGuiContext::Image(m_DefaultImage, { 128, 128 });
		ImGui::End();
	}
	void E3DLayer::atmosphere_UI()
	{
		auto& atmosphere_info = m_Scene.getComponent<SkyboxComponent>(m_SkyBoxEntity)->_AtmosphericScatteringInfo;

		if (m_AtmosphereUI) {
			ImGui::Begin("Sky", &m_AtmosphereUI);
			if (ImGui::TreeNodeEx("Atmospheric Scattering##A", EditorColorScheme::TreeNodeFlags)) {
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
				ImGui::DragFloat("##4", &atmosphere_info.p_MieSC, 0.0000001, '%.8f');
				ImGui::DragFloat("##5", &atmosphere_info.p_RayleighSHeight, 0.01, '%.8f');
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
		}
	}
	void E3DLayer::cameraEdit()
	{
		if (m_Camera.getView() == View::First_Person) {
			if (Input::keyIsPressed(Key::Tab)) {
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
		else if (m_Camera.getView() == View::Third_Person) {
			if ((Input::mouseButtonIsDown(0) || Input::mouseIsScrolling()) && m_ViewportFocused) {
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

		if (m_EditorCameraUI) {
			ImGui::Begin("Editor Camera", &m_EditorCameraUI);
			if (ImGui::TreeNodeEx("Camera", EditorColorScheme::TreeNodeFlags)) {

				if (m_Camera.getView() == View::First_Person) {
					if (ImGui::Button("Third Person View"))
						m_Camera.setView(View::Third_Person);
				}
				else if (m_Camera.getView() == View::Third_Person) {
					if (ImGui::Button("First Person View"))
						m_Camera.setView(View::First_Person);
				}
				ImGui::Text("[Press tab to enable/disable first person viewing]");
				ImGui::Text("Field Of View");
				ImGui::DragFloat("##32", &const_cast<float&>(m_Camera.getFieldOfView()), 1, 0);
				ImGui::Text("Movement Speed");
				ImGui::DragFloat("##33", &m_CameraController.m_MovementSpeed, 1);
				ImGui::TreePop();
			}
			ImGui::End();
		}
	}
}
