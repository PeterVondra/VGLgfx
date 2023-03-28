#include <ostream>
#define VGL_RENDER_API_VULKAN
#include <VGLgfx/VGLgfx.h>

class VGLgfxUILayer : public vgl::Layer
{
	public:
		void onAttach();
		void onDetach();
		void onUpdate();
		void onImGuiUpdate();

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
		friend class VGLgfxEditorApp;

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

class VGLgfxEditorApp : public vgl::Application
{
public:
	VGLgfxEditorApp(vgl::AppConfig& p_Config) : vgl::Application(p_Config) {
		m_Window.setIcon("data/CMakeResources/Vulkan.png");
	};
	~VGLgfxEditorApp() {};

	void setup() override {
		m_UILayer->m_WindowPtr = &m_Window;
		m_UILayer->m_RendererPtr = &m_Renderer;

		m_Layers.pushLayer(m_UILayer);
	}

private:
	VGLgfxUILayer* m_UILayer = new VGLgfxUILayer;
};