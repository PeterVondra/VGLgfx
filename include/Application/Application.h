#pragma once

#include "LayerStack.h"
#include "../VGL_Internal.h"

namespace vgl
{
	struct AppConfig
	{
		AppConfig() {};
		std::string Title;
		Vector2i DefaultWindowSize;
		Vector2i RenderResolution;
		unsigned int MSAASamples;
	};

	struct GraphicsConfig
	{
		GraphicsConfig() {};

		//vgl::vk::CubeMap* irradiance;
		//vgl::vk::CubeMap* preFiltered;
		//vgl::vk::Image* brdflut;
	};

	class Application
	{
		public:
			Application(AppConfig& p_AppConfig);
			virtual ~Application() {};

			virtual void setup();
			void run();

			void setWindowIcon(std::string p_Path);

			void pushLayer(Layer* p_Layer);
			void pushOverlay(Layer* p_Overlay);
			void popLayer(Layer* p_Layer);
			void popOverlay(Layer* p_Overlay);

		protected:
			LayerStack m_Layers;

			virtual void destroy();
			Window m_Window;
			Renderer m_Renderer;

			GraphicsConfig gConfig;
		private:
			AppConfig* m_AppConfig;

			bool m_Running;

			bool onEvent(Event::Event& p_Event);
	};
}