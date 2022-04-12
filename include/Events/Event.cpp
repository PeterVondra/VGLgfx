#include "Event.h"

namespace Event
{
	Event::Event()
	{
		//def
	}

	Event::~Event()
	{
		//def
	}

	EventType Event::getEventType() const
	{
		return EventType::VoidEvent;
	}
	EventCategory Event::getCategoryFlags() const
	{
		return EventCategory::None;
	}
	std::string Event::getEventName() const
	{
		return "Void Event";
	}

	bool Event::isInEventCategory(EventCategory event)
	{
		return 1;
	}

	EventType Event::getStaticEventType()
	{
		return EventType::VoidEvent;
	}
	EventCategory Event::getStaticCategoryFlags()
	{
		return EventCategory::None;
	}

	void Event::reset() {}
}