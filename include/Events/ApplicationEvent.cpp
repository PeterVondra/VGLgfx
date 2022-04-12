#include "ApplicationEvent.h"

namespace Event
{
	EventType WindowResizedEvent::getEventType() const
	{
		return EventType::WindowResize;
	}
	EventCategory WindowResizedEvent::getCategoryFlags() const
	{
		return EventCategory::ApplicationEventCategory;
	}
	std::string WindowResizedEvent::getEventName() const
	{
		return "WindowResizedEvent";
	}

	int WindowResizedEvent::getWidth() const
	{
		return m_Width;
	}
	int WindowResizedEvent::getHeight() const
	{
		return m_Height;
	}
	void WindowResizedEvent::reset()
	{
		m_Width = 0;
		m_Height = 0;
	}

	EventType WindowClosedEvent::getEventType() const
	{
		return EventType::WindowClose;
	}
	EventCategory WindowClosedEvent::getCategoryFlags() const
	{
		return EventCategory::ApplicationEventCategory;
	}
	std::string WindowClosedEvent::getEventName() const
	{
		return "WindowClosedEvent";
	}
	void WindowClosedEvent::reset()
	{
	}
}