#pragma once

#include "../../lib/glew-cmake/include/GL/glew.h"
#include "../BaseWindow.h"

namespace vgl 
{
	namespace gl
	{
		class Window : public ::vgl::BaseWindow
		{
		public:
			Window();
			Window(const Vector2i p_Size, const char* p_Title, const int p_MSAASamples);
			~Window();

			void destroy();

		protected:
		private:
			uint32_t m_MSAASamples;

			bool m_Complete = false;

		};
	}
}
