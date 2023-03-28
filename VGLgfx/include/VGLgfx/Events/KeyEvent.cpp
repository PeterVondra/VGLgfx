#include "KeyEvent.h"

namespace Event
{

	EventType KeyEvent::getStaticEventType()
	{
		return EventType::AnyKeyEvent;
	}
	EventType KeyEvent::getEventType() const
	{
		return EventType::AnyKeyEvent;
	}
	EventCategory KeyEvent::getStaticCategoryFlags()
	{
		return EventCategory::KeyboardEventCategory;
	}
	EventCategory KeyEvent::getCategoryFlags() const
	{
		return EventCategory::KeyboardEventCategory;
	}
	std::string KeyEvent::getEventName() const
	{
		return "AnyKeyEvent";
	}
	int KeyEvent::getKeyCode() const
	{
		return m_KeyCode;
	};
	void KeyEvent::resetDef()
	{
		m_KeyCode = -1;
	}


	EventType KeyTypedEvent::getStaticEventType()
	{
		return EventType::KeyPressed;
	}
	EventType KeyTypedEvent::getEventType() const
	{
		return EventType::KeyPressed;
	}
	EventCategory KeyTypedEvent::getStaticCategoryFlags()
	{
		return EventCategory::KeyboardEventCategory;
	}
	EventCategory KeyTypedEvent::getCategoryFlags() const
	{
		return EventCategory::KeyboardEventCategory;
	}
	std::string KeyTypedEvent::getEventName() const
	{
		return "KeyPressedEvent";
	}

	void KeyTypedEvent::reset()
	{
		resetDef();
	}

	EventType KeyPressedEvent::getStaticEventType()
	{
		return EventType::KeyPressed;
	}
	EventType KeyPressedEvent::getEventType() const
	{
		return EventType::KeyPressed;
	}
	EventCategory KeyPressedEvent::getStaticCategoryFlags()
	{
		return EventCategory::KeyboardEventCategory;
	}
	EventCategory KeyPressedEvent::getCategoryFlags() const
	{
		return EventCategory::KeyboardEventCategory;
	}
	std::string KeyPressedEvent::getEventName() const
	{
		return "KeyPressedEvent";
	}

	void KeyPressedEvent::reset()
	{
		resetDef();
	}

	EventType KeyReleasedEvent::getStaticEventType()
	{
		return EventType::KeyReleased;
	}
	EventType KeyReleasedEvent::getEventType() const
	{
		return EventType::KeyReleased;
	}
	EventCategory KeyReleasedEvent::getStaticCategoryFlags()
	{
		return EventCategory::KeyboardEventCategory;
	}
	EventCategory KeyReleasedEvent::getCategoryFlags() const
	{
		return EventCategory::KeyboardEventCategory;
	}
	std::string KeyReleasedEvent::getEventName() const
	{
		return "KeyReleasedEvent";
	}
	void KeyReleasedEvent::reset()
	{
		resetDef();
	}

	EventType KeyRepeatedEvent::getStaticEventType()
	{
		return EventType::KeyRepeated;
	}
	EventType KeyRepeatedEvent::getEventType() const
	{
		return EventType::KeyRepeated;
	}
	EventCategory KeyRepeatedEvent::getStaticCategoryFlags()
	{
		return EventCategory::KeyboardEventCategory;
	}
	EventCategory KeyRepeatedEvent::getCategoryFlags() const
	{
		return EventCategory::KeyboardEventCategory;
	}
	std::string KeyRepeatedEvent::getEventName() const
	{
		return "KeyRepeatedEvent";
	}
	void KeyRepeatedEvent::reset()
	{
		resetDef();
	}
}