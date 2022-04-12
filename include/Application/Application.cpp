#include "Application.h"

namespace vgl
{
	Application::Application(AppConfig& p_AppConfig)
		: m_AppConfig(&p_AppConfig),
		m_Window(p_AppConfig.DefaultWindowSize, p_AppConfig.Title.c_str(), p_AppConfig.MSAASamples, false),
		m_Renderer(m_Window, p_AppConfig.RenderResolution)
	{
		vgl::InputManager::setWindow(m_Window);

		std::function<bool(Event::Event&)> onEventFun(std::bind(&Application::onEvent, this, std::placeholders::_1));
		InputManager::setEventDispatchCallback(onEventFun);
	}
	
	void Application::setup()
	{
		
	}
	void Application::run()
	{
		static bool flag = true;
		if (flag) {
			setup();
			//m_Renderer.m_EnvData.p_BRDFLut = gConfig.brdflut;
			//m_Renderer.m_EnvData.p_IrradianceMap = gConfig.irradiance;
			//m_Renderer.m_EnvData.p_PreFilteredMap = gConfig.preFiltered;
			flag = false;
		}

		m_Running = true;
		while (!m_Window.closed())
		{
			m_Renderer.beginScene();

			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			for (Layer* layer : m_Layers.m_Layers) {
				if (layer && layer->m_Enabled) {
					if (layer->m_CameraEnabled) {
						m_Renderer.submit(layer->m_Camera);
						layer->m_Camera.update();
					}

					layer->onImGuiUpdate();

					layer->onUpdate(m_Renderer);
				}
			}
			ImGui::EndFrame();
			ImGui::UpdatePlatformWindows();
			ImGui::Render();

			m_Renderer.updateImGuiDrawData();
			m_Renderer.endScene();

			m_Window.update();
		}
		m_Running = false;
	}

	void Application::setWindowIcon(std::string p_Path)
	{
		m_Window.setIcon(p_Path.c_str());
	}

	void Application::destroy()
	{

	}

	bool Application::onEvent(Event::Event& p_Event)
	{
		Event::EventDispatcher dispatcher(p_Event);

		for (auto it = m_Layers.end(); it != m_Layers.begin();) {
			std::advance(it, -1);
			if (*it && (*it)->m_Enabled) {
				dispatcher.dispatch<Event::KeyTypedEvent>(&Layer::onKeyTyped,		*it);
				dispatcher.dispatch<Event::KeyPressedEvent>(&Layer::onKeyPressed,	*it);
				dispatcher.dispatch<Event::KeyReleasedEvent>(&Layer::onKeyReleased, *it);
				dispatcher.dispatch<Event::KeyRepeatedEvent>(&Layer::onKeyRepeated, *it);

				dispatcher.dispatch<Event::MouseMovedEvent>(&Layer::onMouseMove,				*it);
				dispatcher.dispatch<Event::MouseScrolledEvent>(&Layer::onScrollEvent,			*it);
				dispatcher.dispatch<Event::MouseButtonPressedEvent>(&Layer::onMousePressed,		*it);
				dispatcher.dispatch<Event::MouseButtonReleasedEvent>(&Layer::onMouseReleased,	*it);
			}
		}

		return false;
	}

	void Application::pushLayer(Layer* p_Layer) 
	{ 
		m_Layers.pushLayer(p_Layer); 
	}
	void Application::pushOverlay(Layer* p_Overlay) 
	{ 
		m_Layers.pushOverlay(p_Overlay);
	}
	void Application::popLayer(Layer* p_Layer) 
	{
		m_Layers.popLayer(p_Layer); 
	}
	void Application::popOverlay(Layer* p_Overlay) 
	{ 
		m_Layers.popOverlay(p_Overlay);
	}
}

