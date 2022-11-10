#pragma once

#include <iostream>
#include <string>
#include <fstream>

#include<GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#ifdef __UNIX__
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include<GLFW/glfw3native.h>
#include<SOIL/stb_image_aug.h>

#include "../VGL_Logging.h"
#include "../Events/Event.h"
#include "../Events/ApplicationEvent.h"
#include "../Events/KeyEvent.h"
#include "../Events/MouseEvent.h"
#include "../Math/Vector.h"

namespace vgl
{
	namespace
	{
		using EventCallback = bool(*)(Event::Event&);

		struct EventCallbacks
		{
			EventCallback keyEventCallback		= nullptr;
			EventCallback mouseEventCallback	= nullptr;
			EventCallback cursorEventCallback	= nullptr;
			EventCallback windowEventCallback	= nullptr;
			Vector2i* winSize;
			Vector2i oldWinSize;
			Vector2i prevCursorPos;
			Vector2i CursorPos;
			bool resized;
		};
	}

	namespace vk { class ImGuiContext; }

	enum class Cursor
	{
		Standard = 0,
		Hand,
		HResize,
		VResize,
		TextInput
	};

	class BaseWindow
	{
		public:
			BaseWindow();
			BaseWindow(const Vector2i p_Size, const char* p_Title);
			BaseWindow(const Vector2i p_Size, const char* p_Title, const bool p_NoBorder);
			~BaseWindow();

			void	setIcon(const char* m_Path);
			void	toggleFullscreen();

			void	setWindowSize(const Vector2i p_Size);
			void	setWindowSize(const int p_Width, const int p_Height);

			void	setPosition(const Vector2i p_Position);
			void	setPosition(const int p_PosX, const int p_PosY);

			void	setCursorPosition(const Vector2f p_Position);
			void	setCursorPosition(const double p_PosX, const double p_PosY);

			void	setWinTitle(const char* p_Title);

			void	enableCursor();
			void	disableCursor();
			void	hideCursor();

			void	enableVSync();
			void	disableVSync();

			void setPPi(const int p_Pixels);
			int getPPi();


			Vector2i		getWindowSize();
			float			getAspectRatio();
			const double&	getDeltaTime();
			Vector2i		getWindowPixelSize();
			float			getFramesPerSecond();

			void	clear(const Vector3f p_Clear);
			void	clear(const float p_ColorR, const float p_ColorG, const float p_ColorB);

			void	update();

			bool	closed();
			void	close();

			bool	onTick();
			bool	onTick5ms();
			bool	windowResized();

			void	setPixel(Vector2i p_Position, Vector3f p_Color);
			void	getPixel(Vector2i p_Position);

			void	setKeyEventCallback(EventCallback    p_Callback);
			void	setMouseEventCallback(EventCallback  p_Callback);
			void	setCursorEventCallback(EventCallback p_Callback);
			void	setWindowEventCallback(EventCallback p_Callback);

			void	pollEvents() { glfwPollEvents(); };

			void	setCursor(Cursor p_Cursor);

			std::string getClipboard();
			void setClipboard(std::string p_Text);

			GLFWwindow* getGLFWWindow() { return m_Window; }

			// Only works for OpenGL
			void swapBuffers();

		protected:
			friend class InputManager;
			friend class vk::ImGuiContext;

			GLFWwindow*		m_Window;
			GLFWimage		m_Icon;

			GLFWcursor* m_StandardCursor;
			GLFWcursor* m_HandCursor;
			GLFWcursor* m_HResizeCursor;
			GLFWcursor* m_VResizeCursor;
			GLFWcursor* m_TextInputCursor;

			#ifdef _WIN32
				HWND m_Win32Handle;
			#endif

			Vector2i		m_WindowSize;
			Vector2i		m_OldWindowSize;
			Vector2i		m_WindowPosition;

			bool          m_VSync;
			const char*   m_Title;

			float m_Fps;
			float m_Frames;
			float m_Tick;
			float m_Tick5ms;

			double m_DeltaTime;
			double m_PreviousTime;

			bool m_WindowResized;

			EventCallbacks m_EventCallbacks;
			void initCallbacks();

			bool m_Fullscreen = false;

			int m_PPi = 1;

			bool m_KeyPressedFlag = false;
			bool m_MouseButtonFlag = false;
			std::pair<bool, float*> m_MouseScrolledFlag = { false, nullptr }; // { is scrolling == true/false, pointer to time since last scrolled } 
			float m_ScrollingResetTimeConstraint = 0.07;
			bool m_KeyRepeatedFlag = false;

		private:

	};
}