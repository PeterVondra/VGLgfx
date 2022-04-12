#pragma once

#include <iostream>
#include "../Platform/BaseWindow.h"

namespace vgl
{
	enum class Key;
	typedef class InputManager
	{	
		public:
			InputManager();
			~InputManager();

			static void		setWindow(BaseWindow* p_Window);
			static void		setWindow(BaseWindow& p_Window);

			static void		setEventDispatchCallback(std::function<bool(Event::Event&)>	p_Callback);

			static void		setKeyEventCallback(std::function<bool(Event::Event&)>		p_Callback);
			static void		setMouseEventCallback(std::function<bool(Event::Event&)>	p_Callback);
			static void		setCursorEventCallback(std::function<bool(Event::Event&)>	p_Callback);
			static void		setScrollEventCallback(std::function<bool(Event::Event&)>	p_Callback);
			static void		setCursorEventCallback(Event::MouseMovedEvent&	p_Callback);
			static void		setScrollEventCallback(Event::MouseScrolledEvent&	p_Callback);

			static void		disableInput();
			static void		enableInput();

			// Keyboard
			static bool		keyIsDown(Key		p_Key);
			static bool		keyIsPressed(Key	p_Key);
			static bool		keyIsReleased(Key	p_Key);
			static bool		keyIsRepeated(Key	p_Key);
			static bool		anyKeyIsPressed();
			static int		getCurrentKey();

			// Cursor
			static Vector2f getCursorPosition();
			static Vector2f getPrevCursorPosition();
			static bool		cursorIsMoving();

			// Mouse
			static Vector2i getMouseScrollOffset();
			static int		getCurrentMouseButton();
			static bool		mouseIsScrolling();
			static bool		mouseButtonIsPressed(const int p_MouseButton);
			static bool		mouseButtonIsDown(const int p_MouseButton);
			static bool		mouseButtonIsReleased(const int p_MouseButton);

		protected:
		private:

			static std::function<bool(Event::Event&)> m_CustomKeyCallback;
			static std::function<bool(Event::Event&)> m_CustomMouseCallback;
			static std::function<bool(Event::Event&)> m_CustomCursorCallback;

			static bool m_KeyEventCallback(Event::Event&		p_CurrentEvent);
			static bool m_MouseEventCallback(Event::Event&		p_CurrentEvent);
			static bool m_CursorEventCallback(Event::Event&		p_CurrentEvent);

			// Keyboard event callbacks
			static bool m_HandleKeyPressedEvent(Event::KeyPressedEvent&		p_KeyPressed);
			static bool m_HandleKeyReleasedEvent(Event::KeyReleasedEvent&	p_KeyReleased);
			static bool m_HandleKeyRepeatedEvent(Event::KeyRepeatedEvent&	p_KeyRepeated);
			static bool m_HandleKeyTypedEvent(Event::KeyTypedEvent&			p_KeyTyped);

			// Mouse event callbacks
			static bool m_HandleMouseButtonPressedEvent(Event::MouseButtonPressedEvent&		p_MouseButtonPressedEvent);
			static bool m_HandleMouseButtonReleasedEvent(Event::MouseButtonReleasedEvent&	p_MouseButtonReleasedEvent);
			static bool m_HandleMouseMovedEvent(Event::MouseMovedEvent&			p_MouseMoved);
			static bool m_HandleMouseScrolledEvent(Event::MouseScrolledEvent&	p_MouseScrolled);

			static BaseWindow*		m_CurrentWindowPtr;
			static Event::Event*	m_CurrentKeyEvent;
			static Event::Event*	m_CurrentMouseEvent;
			static Event::Event*	m_CurrentCursorEvent;

			Vector2f m_PrevCursorPos;

			static bool m_CurrentKeys[1024];
			static int m_CurrentKey;

			static bool m_CurrentMouseButtons[100];
			static char m_CurrentMouseButton;

			// Keyboard events
			static Event::KeyPressedEvent		m_KeyPressed;
			static Event::KeyPressedEvent		m_PrevKeyPressed;
			static Event::KeyReleasedEvent		m_KeyReleased;
			static Event::KeyReleasedEvent		m_PrevKeyReleased;
			static Event::KeyRepeatedEvent		m_KeyRepeated;
			static Event::KeyTypedEvent			m_KeyTyped;
			
			// Mouse events
			static Event::MouseButtonPressedEvent		m_MouseButtonPressed;
			static Event::MouseButtonPressedEvent		m_PrevMouseButtonPressed;
			static Event::MouseButtonReleasedEvent		m_MouseButtonReleased;
			static Event::MouseButtonReleasedEvent		m_PrevMouseButtonReleased;
			static Event::MouseMovedEvent				m_MouseMoved;
			static Event::MouseScrolledEvent			m_MouseScrolled;
			static Event::MouseScrolledEvent			m_PrevMouseScrolled;

			static float m_ScrollingResetTime;
	}Input;

	enum class Key
	{
		None =					GLFW_KEY_UNKNOWN,
		N0 =                    GLFW_KEY_0,
		N1 =                    GLFW_KEY_1,
		N2 =                    GLFW_KEY_2,
		N3 =                    GLFW_KEY_3,
		N4 =                    GLFW_KEY_4,
		N5 =                    GLFW_KEY_5,
		N6 =                    GLFW_KEY_6,
		N7 =                    GLFW_KEY_7,
		N8 =                    GLFW_KEY_8,
		N9 =                    GLFW_KEY_9,

		A =                     GLFW_KEY_A,
		B =                     GLFW_KEY_B,
		C =                     GLFW_KEY_C,
		D =                     GLFW_KEY_D,
		E =                     GLFW_KEY_E,
		F =                     GLFW_KEY_F,
		G =                     GLFW_KEY_G,
		H =                     GLFW_KEY_H,
		I =                     GLFW_KEY_I,
		J =                     GLFW_KEY_J,
		K =                     GLFW_KEY_K,
		L =                     GLFW_KEY_L,
		M =                     GLFW_KEY_M,
		N =                     GLFW_KEY_N,
		O =                     GLFW_KEY_O,
		P =                     GLFW_KEY_P,
		Q =                     GLFW_KEY_Q,
		R =                     GLFW_KEY_R,
		S =                     GLFW_KEY_S,
		T =                     GLFW_KEY_T,
		U =                     GLFW_KEY_U,
		V =                     GLFW_KEY_V,
		W =                     GLFW_KEY_W,
		X =                     GLFW_KEY_X,
		Y =                     GLFW_KEY_Y,
		Z =                     GLFW_KEY_Z,

		Tab =                   GLFW_KEY_TAB,
		Space =                 GLFW_KEY_SPACE,
		Apostrophe =            GLFW_KEY_APOSTROPHE,
		Comma =                 GLFW_KEY_COMMA,
		Minus =                 GLFW_KEY_MINUS,
		Period =                GLFW_KEY_PERIOD,
		Slash =                 GLFW_KEY_SLASH,
		Semicolon =             GLFW_KEY_SEMICOLON,
		Equal =                 GLFW_KEY_EQUAL,
		LeftBracket =			GLFW_KEY_LEFT_BRACKET,
		RightBracket =			GLFW_KEY_RIGHT_BRACKET,
		Backslash =             GLFW_KEY_BACKSLASH,
		GraveAccent =			GLFW_KEY_GRAVE_ACCENT,
		World1 =                GLFW_KEY_WORLD_1,
		World2 =                GLFW_KEY_WORLD_2,
		Escape =                GLFW_KEY_ESCAPE,
		Enter =                 GLFW_KEY_ENTER,
		Backspace =             GLFW_KEY_BACKSPACE,
		Insert =                GLFW_KEY_INSERT,
		Delete_ =               GLFW_KEY_DELETE,
		Right =                 GLFW_KEY_RIGHT,
		Left =                  GLFW_KEY_LEFT,
		Up =                    GLFW_KEY_UP,
		Down =                  GLFW_KEY_DOWN,
		PageUp =				GLFW_KEY_PAGE_UP,
		PageDown =				GLFW_KEY_PAGE_DOWN,
		Home =                  GLFW_KEY_HOME,
		End =                   GLFW_KEY_END,
		CapsLock =				GLFW_KEY_CAPS_LOCK,
		ScrollLock =			GLFW_KEY_SCROLL_LOCK,
		NumLock =				GLFW_KEY_NUM_LOCK,
		PrintScreen =			GLFW_KEY_PRINT_SCREEN,
		Pause =                 GLFW_KEY_PAUSE,

		F1 =                    GLFW_KEY_F1,
		F2 =                    GLFW_KEY_F2,
		F3 =                    GLFW_KEY_F3,
		F4 =                    GLFW_KEY_F4,
		F5 =                    GLFW_KEY_F5,
		F6 =                    GLFW_KEY_F6,
		F7 =                    GLFW_KEY_F7,
		F8 =                    GLFW_KEY_F8,
		F9 =                    GLFW_KEY_F9,
		F10 =                   GLFW_KEY_F10,
		F11 =                   GLFW_KEY_F11,
		F12 =                   GLFW_KEY_F12,
		F13 =                   GLFW_KEY_F13,
		F14 =                   GLFW_KEY_F14,
		F15 =                   GLFW_KEY_F15,
		F16 =                   GLFW_KEY_F16,
		F17 =                   GLFW_KEY_F17,
		F18 =                   GLFW_KEY_F18,
		F19 =                   GLFW_KEY_F19,
		F20 =                   GLFW_KEY_F20,
		F21 =                   GLFW_KEY_F21,
		F22 =                   GLFW_KEY_F22,
		F23 =                   GLFW_KEY_F23,
		F24 =                   GLFW_KEY_F24,
		F25 =                   GLFW_KEY_F25,

		Kp0 =                  GLFW_KEY_KP_0,
		Kp1 =                  GLFW_KEY_KP_1,
		Kp2 =                  GLFW_KEY_KP_2,
		Kp3 =                  GLFW_KEY_KP_3,
		Kp4 =                  GLFW_KEY_KP_4,
		Kp5 =                  GLFW_KEY_KP_5,
		Kp6 =                  GLFW_KEY_KP_6,
		Kp7 =                  GLFW_KEY_KP_7,
		Kp8 =                  GLFW_KEY_KP_8,
		Kp9 =                  GLFW_KEY_KP_9,

		KpDecimal =            GLFW_KEY_KP_DECIMAL,
		KpDivide =             GLFW_KEY_KP_DIVIDE,
		KpMultiply =           GLFW_KEY_KP_MULTIPLY,
		KpSubtract =           GLFW_KEY_KP_SUBTRACT,
		KpAdd =                GLFW_KEY_KP_ADD,
		KpEnter =              GLFW_KEY_KP_ENTER,
		KpEqual =              GLFW_KEY_KP_EQUAL,
		LeftShift =            GLFW_KEY_LEFT_SHIFT,
		LeftControl =          GLFW_KEY_LEFT_CONTROL,
		LeftAlt =              GLFW_KEY_LEFT_ALT,
		LeftSuper =            GLFW_KEY_LEFT_SUPER,
		RightShift =           GLFW_KEY_RIGHT_SHIFT,
		RightControl =         GLFW_KEY_RIGHT_CONTROL,
		RightAlt =             GLFW_KEY_RIGHT_ALT,
		RightSuper =           GLFW_KEY_RIGHT_SUPER,
		Menu =                 GLFW_KEY_MENU,
		Last =                 GLFW_KEY_LAST
	};
}
