#include "MouseEvent.h"

namespace Event
{
	//Mouse button
	int MouseButtonEvent::getMouseButtonCode() const
	{
		return m_MouseButtonCode;
	}
	void MouseButtonEvent::resetDef()
	{
		m_MouseButtonCode = -1;
	}

	//Mouse button pressed 
	EventType MouseButtonPressedEvent::getStaticEventType()
	{
		return EventType::MouseButtonPressed;
	}
	EventType MouseButtonPressedEvent::getEventType() const
	{
		return EventType::MouseButtonPressed;
	}
	EventCategory MouseButtonPressedEvent::getCategoryFlags() const
	{
		return EventCategory::MouseButtonEventCategory;
	}
	std::string MouseButtonPressedEvent::getEventName() const
	{
		return "MouseButtonPressedEvent";
	}
	void MouseButtonPressedEvent::reset()
	{
		resetDef();
	}

	//Mouse button released
	EventType MouseButtonReleasedEvent::getStaticEventType()
	{
		return EventType::MouseButtonReleased;
	}
	EventType MouseButtonReleasedEvent::getEventType() const
	{
		return EventType::MouseButtonReleased;
	}
	EventCategory MouseButtonReleasedEvent::getCategoryFlags() const
	{
		return EventCategory::MouseButtonEventCategory;
	}
	std::string MouseButtonReleasedEvent::getEventName() const
	{
		return "MouseButtonReleasedEvent";
	}
	void MouseButtonReleasedEvent::reset()
	{
		resetDef();
	}

	//Mouse moved
	EventType MouseMovedEvent::getStaticEventType()
	{
		return EventType::MouseMoved;
	}
	EventType MouseMovedEvent::getEventType() const
	{
		return EventType::MouseMoved;
	}
	EventCategory MouseMovedEvent::getCategoryFlags() const
	{
		return EventCategory::MouseEventCategory;
	}
	std::string MouseMovedEvent::getEventName() const
	{
		return "MouseMovedEvent";
	}

	double MouseMovedEvent::getX() const
	{
		return m_PositionX;
	}
	double MouseMovedEvent::getY() const
	{
		return m_PositionY;
	}

	void MouseMovedEvent::reset()
	{
		m_PositionX = 0;
		m_PositionY = 0;
	}
	//Mouse scrolled event
	EventType MouseScrolledEvent::getStaticEventType()
	{
		return EventType::MouseScrolled;
	}
	EventType MouseScrolledEvent::getEventType() const
	{
		return EventType::MouseScrolled;
	}
	EventCategory MouseScrolledEvent::getCategoryFlags() const
	{
		return EventCategory::MouseEventCategory;
	}
	std::string MouseScrolledEvent::getEventName() const
	{
		return "MouseScrolledEvent";
	}

	double MouseScrolledEvent::getOffsetX() const
	{
		return m_OffsetX;
	}
	double MouseScrolledEvent::getOffsetY() const
	{
		return m_OffsetY;
	}
	void MouseScrolledEvent::reset()
	{
		m_OffsetX = 0;
		m_OffsetY = 0;
	}
}