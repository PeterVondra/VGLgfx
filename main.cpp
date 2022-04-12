#include <ostream>
#define VGL_USING_VULKAN
#define IMGUI_VK_IMPL
#include "include/Application/Application.h"
#include "include/Math/Camera.h"
#include "include/RenderPipelines.h"
#include "include/Scene.h"

class TestLayer : public vgl::Layer
{
	public:
		void onAttach() {
			m_Camera = vgl::Camera(Vector3f(0.0f, 1.0f, 0.0f));
			m_Camera.setView(vgl::View::First_Person);
			m_CameraController.bind(m_Camera);
			m_CameraController.setDeltaTime(m_WindowPtr->getDeltaTime());
			m_CameraController.enable(false);

			m_RenderPipeline.setup(m_RendererPtr, m_WindowPtr, &m_Scene);
		}
		void onDetach();
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

		vgl::CameraController3D m_CameraController;
		vgl::Window* m_WindowPtr;
		vgl::Renderer* m_RendererPtr;

		vgl::RenderPipeline_Deferred m_RenderPipeline;

		vgl::Scene m_Scene;
};

class TestApp : public vgl::Application
{
	public
		TestApp(vgl::AppConfig& p_Config) : vgl::Application(p_Config) {};
		~TestApp() {};

		void setup() override {
			m_Layers.pushLayer(m_TestLayer);

			m_TestLayer->m_WindowPtr = &m_Window;
			m_TestLayer->m_RendererPtr = &m_Renderer;
		}

	private:
		TestLayer* m_TestLayer = new TestLayer;
};

int main()
{
	vgl::AppConfig config;
	config.DefaultWindowSize = { 1200, 700 };
	config.MSAASamples = 8;
	config.Title = "VGL2";
	auto app = new TestApp(config);

	app->run();

}
