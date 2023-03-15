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

		auto point_light = new vgl::PointLight3DComponent;

		m_Scene.addEntity(*mesh, *transform, *(new vgl::EntityNameComponent("Sponza")));

		m_RenderPipeline.setup(m_RendererPtr, m_WindowPtr, &m_Scene);
		vgl::ImageLoader::getImageFromPath(img, "data/CMakeResources/Vulkan.png", vgl::ColorSpace::RGB);
		vgl::ImageLoader::getImageFromPath(m_DefaultImage, "data/textures/default.png", vgl::ColorSpace::RGB);
	}
	void onDetach() {}
	void onUpdate(vgl::Renderer& p_Renderer) {

		if (vgl::Input::keyIsPressed(vgl::Key::F11))
			m_WindowPtr->toggleFullscreen();

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

		ImGui::Begin("Editor Camera");
		ImGui::Text(("Position: [" + std::to_string(m_Camera.getPosition().x) + ", " + std::to_string(m_Camera.getPosition().y) + "]").c_str());
		ImGui::Text(("Rotation: [" + std::to_string(m_Camera.getRotation().x) + ", " + std::to_string(m_Camera.getRotation().y) + std::to_string(m_Camera.getRotation().z) + "]").c_str());

		ImGui::BeginMainMenuBar();
		ImGui::Text(time.c_str());
		ImGui::EndMainMenuBar();
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

	vgl::Ortho2DCameraController m_CameraController;
	vgl::Window* m_WindowPtr;
	vgl::Renderer* m_RendererPtr;

	vgl::RenderPipeline_Deferred m_RenderPipeline;

	vgl::Scene m_Scene;

	vgl::Viewport m_ViewportData;
	bool m_ViewportFocused = false;
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