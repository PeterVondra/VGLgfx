#include "InputManager.h"
#include <functional>

namespace vgl
{
	BaseWindow* InputManager::m_CurrentWindowPtr = nullptr;

	std::function<bool(Event::Event&)> InputManager::m_CustomKeyCallback;
	std::function<bool(Event::Event&)> InputManager::m_CustomMouseCallback;
	std::function<bool(Event::Event&)> InputManager::m_CustomCursorCallback;

	Event::Event* InputManager::m_CurrentKeyEvent = nullptr;
	Event::Event* InputManager::m_CurrentMouseEvent = nullptr;
	Event::Event* InputManager::m_CurrentCursorEvent = nullptr;
	bool InputManager::m_CurrentKeys[1024];
	int  InputManager::m_CurrentKey = -1;
	bool InputManager::m_CurrentMouseButtons[100];
	char InputManager::m_CurrentMouseButton;

	// Keyboard events
	Event::KeyPressedEvent		InputManager::m_PrevKeyPressed = Event::KeyPressedEvent();
	Event::KeyPressedEvent		InputManager::m_KeyPressed = Event::KeyPressedEvent();
	Event::KeyReleasedEvent		InputManager::m_PrevKeyReleased = Event::KeyReleasedEvent();
	Event::KeyReleasedEvent		InputManager::m_KeyReleased = Event::KeyReleasedEvent();
	Event::KeyRepeatedEvent		InputManager::m_KeyRepeated;
	Event::KeyTypedEvent		InputManager::m_KeyTyped;

	// Mouse events
	Event::MouseButtonPressedEvent		InputManager::m_MouseButtonPressed;
	Event::MouseButtonReleasedEvent		InputManager::m_MouseButtonReleased;
	Event::MouseButtonPressedEvent		InputManager::m_PrevMouseButtonPressed;
	Event::MouseButtonReleasedEvent		InputManager::m_PrevMouseButtonReleased;
	Event::MouseMovedEvent				InputManager::m_MouseMoved;
	Event::MouseScrolledEvent			InputManager::m_MouseScrolled;
	Event::MouseScrolledEvent			InputManager::m_PrevMouseScrolled;

	float InputManager::m_ScrollingResetTime;

	InputManager::InputManager()
	{

	}
	InputManager::~InputManager()
	{

	}

	void InputManager::setWindow(BaseWindow& p_Window)
	{
		m_CurrentWindowPtr = &p_Window;
		m_CurrentWindowPtr->setKeyEventCallback(m_KeyEventCallback);
		m_CurrentWindowPtr->setMouseEventCallback(m_MouseEventCallback);
		m_CurrentWindowPtr->setCursorEventCallback(m_CursorEventCallback);
		m_CurrentWindowPtr->m_MouseScrolledFlag.second = &m_ScrollingResetTime;
	}
	void InputManager::setWindow(BaseWindow* p_Window)
	{
		m_CurrentWindowPtr = p_Window;
		m_CurrentWindowPtr->setKeyEventCallback(m_KeyEventCallback);
		m_CurrentWindowPtr->setMouseEventCallback(m_MouseEventCallback);
		m_CurrentWindowPtr->setCursorEventCallback(m_CursorEventCallback);
		m_CurrentWindowPtr->m_MouseScrolledFlag.second = &m_ScrollingResetTime;
	}

	void InputManager::setKeyEventCallback(std::function<bool(Event::Event&)> p_Callback)
	{
		m_CustomKeyCallback = p_Callback;
	}
	void InputManager::setMouseEventCallback(std::function<bool(Event::Event&)> p_Callback)
	{
		m_CustomMouseCallback = p_Callback;
	}
	void InputManager::setCursorEventCallback(std::function<bool(Event::Event&)> p_Callback)
	{
		m_CustomCursorCallback = p_Callback;
	}
	void InputManager::setEventDispatchCallback(std::function<bool(Event::Event&)> p_Callback)
	{
		m_CustomKeyCallback = p_Callback;
		m_CustomMouseCallback = p_Callback;
		m_CustomCursorCallback = p_Callback;
	}
	void InputManager::setCursorEventCallback(Event::MouseMovedEvent& p_Callback)
	{
		m_MouseMoved = p_Callback;
	}
	void InputManager::setScrollEventCallback(std::function<bool(Event::Event&)> p_Callback)
	{
		m_CustomMouseCallback = p_Callback;
	}
	void InputManager::setScrollEventCallback(Event::MouseScrolledEvent& p_Callback)
	{
		m_MouseScrolled = p_Callback;
	}

	void InputManager::disableInput()
	{

	}
	void InputManager::enableInput()
	{

	}

	// Keyboard
	bool InputManager::keyIsDown(Key p_Key)
	{
		return m_CurrentKeys[(int)p_Key];
	}
	bool InputManager::keyIsPressed(Key p_Key)
	{
		return m_CurrentWindowPtr->m_KeyPressedFlag && m_CurrentKeys[(int)p_Key];
	}
	bool InputManager::keyIsReleased(Key p_Key)
	{
		return m_KeyReleased.keyReleased() == true && m_KeyReleased.getKeyCode() == (int)p_Key;
	}
	bool InputManager::keyIsRepeated(Key p_Key)
	{
		return m_KeyRepeated.keyRepeated() == true && m_CurrentKeys[(int)p_Key] && m_CurrentWindowPtr->m_KeyRepeatedFlag;
	}
	bool InputManager::anyKeyIsPressed()
	{
		return m_CurrentWindowPtr->m_KeyPressedFlag && m_KeyPressed.keyPressed() == true;
	}
	int  InputManager::getCurrentKey()
	{
		return m_CurrentKey;
	}

	// Cursor
	Vector2f InputManager::getCursorPosition()
	{
		return Vector2f(m_MouseMoved.getX(), m_CurrentWindowPtr->getWindowSize().y - m_MouseMoved.getY());
	}
	Vector2f InputManager::getPrevCursorPosition()
	{
		return Vector2f(m_CurrentWindowPtr->m_EventCallbacks.prevCursorPos.x, m_CurrentWindowPtr->m_WindowSize.y - m_CurrentWindowPtr->m_EventCallbacks.prevCursorPos.y);
	}
	bool InputManager::cursorIsMoving()
	{
		return m_MouseMoved.mouseMoved();
	}

	// Mouse
	Vector2i InputManager::getMouseScrollOffset()
	{
		return Vector2i(0);
	}
	int InputManager::getCurrentMouseButton()
	{
		return 1;
	}
	bool InputManager::mouseIsScrolling()
	{
		return m_CurrentWindowPtr->m_MouseScrolledFlag.first;
	}
	bool InputManager::mouseButtonIsPressed(const int p_MouseButton)
	{
		return m_CurrentMouseButtons[p_MouseButton] && m_CurrentWindowPtr->m_MouseButtonFlag;
	}
	bool InputManager::mouseButtonIsDown(const int p_MouseButton)
	{
		return m_CurrentMouseButtons[p_MouseButton];
	}
	bool InputManager::mouseButtonIsReleased(const int p_MouseButton)
	{
		return m_CurrentMouseButtons[p_MouseButton] == false;
	}

	// Keyboard event callbacks
	bool InputManager::m_HandleKeyPressedEvent(Event::KeyPressedEvent& p_KeyPressed)
	{
		m_PrevKeyPressed = m_KeyPressed;
		m_KeyPressed = p_KeyPressed;
		//if (!m_PrevKeyPressed.keyPressed())
			m_CurrentWindowPtr->m_KeyPressedFlag = true;
		m_CurrentKeys[p_KeyPressed.getKeyCode()] = true;
		return false;
	}
	bool InputManager::m_HandleKeyReleasedEvent(Event::KeyReleasedEvent& p_KeyReleased)
	{
		m_CurrentWindowPtr->m_KeyPressedFlag = false;
		m_CurrentWindowPtr->m_KeyRepeatedFlag = false;
		m_PrevKeyPressed = Event::KeyPressedEvent(-1, false);
		m_PrevKeyReleased = m_KeyReleased;
		m_KeyReleased = p_KeyReleased;
		m_CurrentKey = -1;
		m_CurrentKeys[p_KeyReleased.getKeyCode()] = false;
		return false;
	}
	bool InputManager::m_HandleKeyRepeatedEvent(Event::KeyRepeatedEvent& p_KeyRepeated)
	{
		m_KeyRepeated = p_KeyRepeated;
		m_CurrentWindowPtr->m_KeyRepeatedFlag = true;
		return false;
	}
	bool InputManager::m_HandleKeyTypedEvent(Event::KeyTypedEvent& p_KeyTyped)
	{
		m_KeyTyped = p_KeyTyped;
		m_CurrentKey = -1;
		m_CurrentKey = p_KeyTyped.getKeyCode();
		return false;
	}

	// Mouse event callbacks
	bool InputManager::m_HandleMouseButtonPressedEvent(Event::MouseButtonPressedEvent& p_MouseButtonPressedEvent)
	{
		m_PrevMouseButtonPressed = m_MouseButtonPressed;
		m_MouseButtonPressed = p_MouseButtonPressedEvent;
		m_CurrentWindowPtr->m_MouseButtonFlag = true;
		m_CurrentMouseButtons[p_MouseButtonPressedEvent.getMouseButtonCode()] = true;
		return false;
	}
	bool InputManager::m_HandleMouseButtonReleasedEvent(Event::MouseButtonReleasedEvent& p_MouseButtonReleasedEvent)
	{
		m_CurrentWindowPtr->m_MouseButtonFlag = false;
		m_PrevMouseButtonReleased = m_MouseButtonReleased;
		m_MouseButtonReleased = p_MouseButtonReleasedEvent;
		m_CurrentMouseButton = -1;
		m_CurrentMouseButtons[p_MouseButtonReleasedEvent.getMouseButtonCode()] = false;
		return false;
	}
	bool InputManager::m_HandleMouseMovedEvent(Event::MouseMovedEvent& p_MouseMovedEvent)
	{
		vgl::Input::setCursorEventCallback(p_MouseMovedEvent);
		if (Vector2i(m_MouseMoved.getX(), m_MouseMoved.getY()) == m_CurrentWindowPtr->m_EventCallbacks.prevCursorPos)
			m_MouseMoved = Event::MouseMovedEvent(p_MouseMovedEvent.getX(), p_MouseMovedEvent.getY(), false);

		return false;
	}
	bool InputManager::m_HandleMouseScrolledEvent(Event::MouseScrolledEvent& p_MouseScrolledEvent)
	{
		m_PrevMouseScrolled = m_MouseScrolled;
		m_MouseScrolled = p_MouseScrolledEvent;
		m_CurrentWindowPtr->m_MouseScrolledFlag.first = true;
		m_ScrollingResetTime = Utils::Logger::getTimePoint();

		return false;
	}

	bool InputManager::m_KeyEventCallback(Event::Event& p_CurrentEvent)
	{
		m_CurrentKeyEvent = &p_CurrentEvent;

		Event::EventDispatcher eventDispatcher(p_CurrentEvent);

		// Dispatch key event callbacks
		eventDispatcher.dispatch<Event::KeyPressedEvent>(m_HandleKeyPressedEvent);
		eventDispatcher.dispatch<Event::KeyReleasedEvent>(m_HandleKeyReleasedEvent);
		eventDispatcher.dispatch<Event::KeyRepeatedEvent>(m_HandleKeyRepeatedEvent);
		eventDispatcher.dispatch<Event::KeyTypedEvent>(m_HandleKeyTypedEvent);

		if (m_CustomKeyCallback != nullptr)
			return m_CustomKeyCallback(p_CurrentEvent);

		return false;
	}
	bool InputManager::m_MouseEventCallback(Event::Event& p_CurrentEvent)
	{
		m_CurrentMouseEvent = &p_CurrentEvent;

		Event::EventDispatcher eventDispatcher(p_CurrentEvent);

		// Dispatch mouse event callbacks
		eventDispatcher.dispatch<Event::MouseButtonPressedEvent>(m_HandleMouseButtonPressedEvent);
		eventDispatcher.dispatch<Event::MouseButtonReleasedEvent>(m_HandleMouseButtonReleasedEvent);
		eventDispatcher.dispatch<Event::MouseScrolledEvent>(m_HandleMouseScrolledEvent);

		if (m_CustomMouseCallback != nullptr) 
			return m_CustomMouseCallback(p_CurrentEvent);

		return false;
	}
	bool InputManager::m_CursorEventCallback(Event::Event& p_CurrentEvent)
	{
		m_CurrentCursorEvent = &p_CurrentEvent;

		Event::EventDispatcher eventDispatcher(p_CurrentEvent);

		// Dispatch mouse event callbacks
		eventDispatcher.dispatch<Event::MouseMovedEvent>(m_HandleMouseMovedEvent);

		if (m_CustomCursorCallback != nullptr)
			return m_CustomCursorCallback(p_CurrentEvent);

		return false;
	}
}
