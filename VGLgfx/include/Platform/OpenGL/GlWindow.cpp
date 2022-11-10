#include "GlWindow.h"

#include "../../VGL_Logging.h"

namespace vgl
{
	namespace gl
	{
		Window::Window() : vgl::BaseWindow()
		{

		}
		Window::Window(const Vector2i p_Size, const char* p_Title, const int p_MSAASamples)
		{
			glfwMakeContextCurrent(m_Window);

			glewExperimental = true;

			if (GLEW_OK == glewInit())
				VGL_LOG_MSG("Failed to initialize GLEW", "GlWindow", Utils::Result::Failed);

			glViewport(0, 0, p_Size.x, p_Size.y);

		}
		Window::~Window()
		{

		}

		void Window::destroy()
		{
			glfwTerminate();
		}
	}
}
