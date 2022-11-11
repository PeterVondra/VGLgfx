#include "BaseWindow.h"
#include "../Utils/Logger.h"

namespace vgl
{
	BaseWindow::BaseWindow() :
		m_WindowSize(Vector2i(0, 0)),
		m_VSync(false),
		m_Title(NULL),
		m_DeltaTime(0.0f),
		m_Fps(0),
		m_Frames(0),
		m_Tick(0),
		m_WindowResized(false),
		m_Icon(GLFWimage()),
#ifdef _WIN32
		m_Win32Handle(HWND()),
#endif		
	m_Window(nullptr)
	{
		//Empty ctor

		m_StandardCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_HandCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		m_HResizeCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		m_VResizeCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		m_TextInputCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	}
	BaseWindow::BaseWindow(const Vector2i p_Size, const char* p_Title) :
		m_WindowSize(p_Size),
		m_VSync(false),
		m_Title(p_Title),
		m_DeltaTime(0.0f),
		m_Fps(0),
		m_Frames(0),
		m_Tick(0),
		m_WindowResized(false),
		m_Icon(GLFWimage()),
#ifdef _WIN32		
		m_Win32Handle(HWND()),
#endif
		m_Window(nullptr)
	{
		//Initialize GLFW
		glfwInit();

#ifdef VGL_RENDER_API_OPENGL
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
		//Create context ourselves, so set glfw to use no API
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_Window = glfwCreateWindow(m_WindowSize.x, m_WindowSize.y, m_Title, nullptr, nullptr);

		#ifdef _WIN32
			m_Win32Handle = glfwGetWin32Window(m_Window);
		#endif	

		m_OldWindowSize = m_WindowSize;

		m_StandardCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_HandCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		m_HResizeCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		m_VResizeCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		m_TextInputCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);

		glfwSetWindowUserPointer(m_Window, this);

		initCallbacks();
	}

	BaseWindow::BaseWindow(const Vector2i p_Size, const char* p_Title, const bool p_NoBorder) :
		m_WindowSize(p_Size),
		m_VSync(false),
		m_Title(p_Title),
		m_DeltaTime(0.0f),
		m_Fps(0),
		m_Frames(0),
		m_Tick(0),
		m_WindowResized(false)
	{
		//Initialize GLFW
		glfwInit();

#ifdef VGL_RENDER_API_OPENGL
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
		//Create context ourselves, so set glfw to use no API
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
		glfwWindowHint(GLFW_DECORATED, !p_NoBorder);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_Window = glfwCreateWindow(m_WindowSize.x, m_WindowSize.y, m_Title, nullptr, nullptr);

		#ifdef _WIN32
			m_Win32Handle = glfwGetWin32Window(m_Window);
		#endif	

		m_OldWindowSize = m_WindowSize;

		m_StandardCursor	= glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_HandCursor		= glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		m_HResizeCursor		= glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		m_VResizeCursor		= glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		m_TextInputCursor	= glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);

		glfwSetWindowUserPointer(m_Window, this);

		initCallbacks();
	}

	void BaseWindow::setPixel(Vector2i p_Position, Vector3f p_Color)
	{
		#ifdef _WIN32
			SetPixel(GetDC(m_Win32Handle), p_Position.x, p_Position.y, RGB(p_Color.x, p_Color.y, p_Color.z));
		#endif
	}

	void BaseWindow::getPixel(Vector2i p_Position)
	{
		#ifdef _WIN32
			GetPixel(GetDC(m_Win32Handle), p_Position.x, p_Position.y);
		#endif
	}

	void BaseWindow::setKeyEventCallback(EventCallback p_Callback)
	{
		m_EventCallbacks.keyEventCallback = p_Callback;

		glfwSetWindowUserPointer(m_Window, &m_EventCallbacks);
	}
	void BaseWindow::setMouseEventCallback(EventCallback p_Callback)
	{
		m_EventCallbacks.mouseEventCallback = p_Callback;

		glfwSetWindowUserPointer(m_Window, &m_EventCallbacks);
	}
	void BaseWindow::setCursorEventCallback(EventCallback p_Callback)
	{
		m_EventCallbacks.cursorEventCallback = p_Callback;

		glfwSetWindowUserPointer(m_Window, &m_EventCallbacks);
	}
	void BaseWindow::setWindowEventCallback(EventCallback p_Callback)
	{
		m_EventCallbacks.windowEventCallback = p_Callback;

		glfwSetWindowUserPointer(m_Window, &m_EventCallbacks);
	}

	void BaseWindow::initCallbacks()
	{
		m_EventCallbacks.winSize = &m_WindowSize;
		m_EventCallbacks.oldWinSize = { 0, 0 };

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* p_Window, int p_Width, int p_Height)
			{
				EventCallbacks& callbacks = *(EventCallbacks*)glfwGetWindowUserPointer(p_Window);
				
				callbacks.oldWinSize = *callbacks.winSize;
				*callbacks.winSize = Vector2i(p_Width, p_Height);
				Event::WindowResizedEvent e(p_Width, p_Height);
				if(callbacks.windowEventCallback != nullptr)
					callbacks.windowEventCallback(e);
			});
		glfwSetKeyCallback(m_Window, [](GLFWwindow* p_Window, int p_KeyCode, int p_ScanCode, int p_Action, int p_Modes)
			{
				EventCallbacks& callbacks = *(EventCallbacks*)glfwGetWindowUserPointer(p_Window);

				switch (p_Action)
				{
					case GLFW_PRESS:
					{
						Event::KeyPressedEvent e(p_KeyCode);
						callbacks.keyEventCallback(e);
						break;
					}
					case GLFW_RELEASE:
					{
						Event::KeyReleasedEvent e(p_KeyCode);
						callbacks.keyEventCallback(e);
						break;
					}
					case GLFW_REPEAT:
					{
						Event::KeyRepeatedEvent e(p_KeyCode);
						callbacks.keyEventCallback(e);
						break;
					}
					default:
					{
						Event::KeyEvent e(p_KeyCode);
						callbacks.keyEventCallback(e);
						break;
					}
				}
			});
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* p_Window, double p_PosX, double p_PosY)
			{
				EventCallbacks& callbacks = *(EventCallbacks*)glfwGetWindowUserPointer(p_Window);

				callbacks.prevCursorPos = callbacks.CursorPos;
				callbacks.CursorPos = Vector2i(p_PosX, p_PosY);

				if (callbacks.prevCursorPos == callbacks.CursorPos)
				{
					Event::MouseMovedEvent e(p_PosX, p_PosY, false);
					callbacks.cursorEventCallback(e);

					return;
				}

				Event::MouseMovedEvent e(p_PosX, p_PosY, true);
				callbacks.cursorEventCallback(e);
			});
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* p_Window, int p_Button, int p_Action, int p_Modes)
			{
				EventCallbacks& callbacks = *(EventCallbacks*)glfwGetWindowUserPointer(p_Window);

				switch (p_Action)
				{
					case GLFW_PRESS:
					{
						Event::MouseButtonPressedEvent e(p_Button);
						callbacks.mouseEventCallback(e);
						break;
					}
					case GLFW_RELEASE:
					{
						Event::MouseButtonReleasedEvent  e(p_Button);
						callbacks.mouseEventCallback(e);
						break;
					}
					default:
					{
						Event::Event e;
						callbacks.mouseEventCallback(e);
						break;
					}
				}
			});
		glfwSetCharCallback(m_Window, [](GLFWwindow* p_Window, unsigned int p_Key)
			{
				EventCallbacks& callbacks = *(EventCallbacks*)glfwGetWindowUserPointer(p_Window);

				Event::KeyTypedEvent e(p_Key);
				callbacks.keyEventCallback(e);
			});
		glfwSetScrollCallback(m_Window, [](GLFWwindow* p_Window, double p_OffsetX, double p_OffsetY)
			{
				EventCallbacks& callbacks = *(EventCallbacks*)glfwGetWindowUserPointer(p_Window);

				Event::MouseScrolledEvent e(p_OffsetX, p_OffsetY);
				callbacks.mouseEventCallback(e);
			});
	}

	void BaseWindow::setIcon(const char* m_Path)
	{
		m_Icon.pixels = stbi_load(m_Path, &m_Icon.width, &m_Icon.height, 0, 4);
		if (m_Icon.pixels != nullptr)
		{
			glfwSetWindowIcon(m_Window, 1, &m_Icon);
			stbi_image_free(m_Icon.pixels);
			return;
		}
		VGL_INTERNAL_WARNING("[BaseWindow]Failed to load icon image for window");
		return;
	}

	void BaseWindow::setCursor(Cursor p_Cursor)
	{
		if(p_Cursor == Cursor::Standard)
			glfwSetCursor(m_Window, m_StandardCursor);
		else if (p_Cursor == Cursor::Hand)
			glfwSetCursor(m_Window, m_HandCursor);
		else if (p_Cursor == Cursor::VResize)
			glfwSetCursor(m_Window, m_VResizeCursor);
		else if (p_Cursor == Cursor::HResize)
			glfwSetCursor(m_Window, m_HResizeCursor);
		else if (p_Cursor == Cursor::TextInput)
			glfwSetCursor(m_Window, m_TextInputCursor);
	}

	std::string BaseWindow::getClipboard()
	{
		return std::string(glfwGetClipboardString(m_Window));
	}
	void BaseWindow::setClipboard(std::string p_Text)
	{
		glfwSetClipboardString(m_Window, p_Text.c_str());
	}

	void BaseWindow::swapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}

	void BaseWindow::toggleFullscreen()
	{
		m_Fullscreen = !m_Fullscreen;

		if (m_Fullscreen)
		{
			glfwGetWindowSize(m_Window, &m_OldWindowSize.x, &m_OldWindowSize.y);
			glfwGetWindowPos(m_Window, &m_WindowPosition.x, &m_WindowPosition.y);

			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
			return;
		}

		glfwSetWindowMonitor(m_Window, nullptr, m_WindowPosition.x, m_WindowPosition.y, m_OldWindowSize.x, m_OldWindowSize.y, GLFW_DONT_CARE);
		return;
	}

	BaseWindow::~BaseWindow()
	{
		close();
	}

	void BaseWindow::setWindowSize(const Vector2i p_Size)
	{
		m_WindowSize = p_Size;
		glfwSetWindowSize(m_Window, p_Size.x, p_Size.y);
	}
	void BaseWindow::setWindowSize(const int p_Width, const int p_Height)
	{
		m_WindowSize = Vector2i(p_Width, p_Height);
		glfwSetWindowSize(m_Window, p_Width, p_Height);
	}

	void BaseWindow::setPosition(const Vector2i POSITION)
	{

	}
	void BaseWindow::setPosition(const int X, const int Y)
	{

	}
	void BaseWindow::setCursorPosition(const Vector2f POSITION)
	{

	}
	void BaseWindow::setCursorPosition(const double X, const double Y)
	{

	}

	void BaseWindow::setPPi(const int p_Pixels)
	{
		m_PPi = p_Pixels;
	}
	int BaseWindow::getPPi()
	{
		return m_PPi;
	}
	void BaseWindow::setWinTitle(const char* p_Title)
	{
		m_Title = p_Title;
		glfwSetWindowTitle(m_Window, m_Title);
	}

	void BaseWindow::enableCursor()
	{
		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	void BaseWindow::disableCursor()
	{
		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	void BaseWindow::hideCursor()
	{
		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}

	void BaseWindow::enableVSync()
	{

	}
	void BaseWindow::disableVSync()
	{

	}

	float BaseWindow::getFramesPerSecond()
	{
		return m_Fps;
	}
	const double& BaseWindow::getDeltaTime()
	{
		return m_DeltaTime;
	}
	Vector2i BaseWindow::getWindowSize()
	{
		return m_WindowSize;
	}

	void BaseWindow::clear(const Vector3f p_Color)
	{
		#ifdef _WIN32
			InvalidateRect(m_Win32Handle, nullptr, TRUE);
			SetBkColor(GetDC(m_Win32Handle), RGB(p_Color.x, p_Color.y, p_Color.z));
			
		#endif
	}
	void BaseWindow::clear(const float R, const float G, const float B)
	{
		#ifdef _WIN32
			InvalidateRect(m_Win32Handle, nullptr, TRUE);
			SetBkColor(GetDC(m_Win32Handle), RGB(R, G, B));
		#endif
	}
	void BaseWindow::update()
	{
		if (onTick())
		{
			m_Fps = (int)(1 / (m_DeltaTime / 1000));
			m_Tick = 0.0f;
		}
		if (onTick5ms())
		{
			m_Fps = (int)(1 / (m_DeltaTime / 1000));
			m_Tick5ms = 0.0f;
		}

		m_DeltaTime = Utils::Logger::getTimePoint() - m_PreviousTime;
		m_Tick += m_DeltaTime;
		m_Tick5ms += m_DeltaTime;
		m_PreviousTime = Utils::Logger::getTimePoint();
		

		m_WindowResized = false;

		if (m_WindowSize != m_EventCallbacks.oldWinSize)
		{
			m_WindowResized = true;
			m_EventCallbacks.oldWinSize = m_WindowSize;
		}

		m_KeyPressedFlag = false;
		m_KeyRepeatedFlag = false;
		m_MouseButtonFlag = false;
		if (Utils::Logger::getTimePoint() - (*m_MouseScrolledFlag.second) >= m_ScrollingResetTimeConstraint)
			m_MouseScrolledFlag.first = false;
		glfwPollEvents();
	}
	float BaseWindow::getAspectRatio()
	{
		return (float)m_WindowSize.x / (float)m_WindowSize.y;
	}

	Vector2i BaseWindow::getWindowPixelSize()
	{
		return m_WindowSize * m_PPi;
	}

	bool BaseWindow::closed()
	{
		return glfwWindowShouldClose(m_Window);
	}
	void BaseWindow::close()
	{
		glfwDestroyWindow(m_Window);
	}

	bool BaseWindow::onTick()
	{
		if (m_Tick <= 1.0f)
			return false;
		return true;
	}
	bool BaseWindow::onTick5ms()
	{
		if (m_Tick5ms <= 0.5f)
			return false;
		return true;
	}

	bool BaseWindow::windowResized()
	{
		return m_WindowResized;
	}
}
