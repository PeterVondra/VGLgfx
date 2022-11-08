#pragma once

#include "Event.h"
#include<string>

namespace Event
{
	class WindowResizedEvent : public Event
	{
		public:
			WindowResizedEvent()
				: m_Width(0),
				m_Height(0),
				m_WindowResized(false) {};
			WindowResizedEvent(const int p_Width, const int p_Height)
				: m_Width(p_Width),
				m_Height(p_Height),
				m_WindowResized(true) {};
			WindowResizedEvent(const int p_Width, const int p_Height, const bool p_WindowResized)
				: m_Width(p_Width),
				m_Height(p_Height),
				m_WindowResized(p_WindowResized) {};

			EventType getEventType() const override;
			EventCategory getCategoryFlags() const override;
			std::string getEventName() const override;

			int getWidth() const;
			int getHeight() const;

			void reset() override;

			bool windowResized() { return m_WindowResized; }

		protected:
		private:
			int m_Width;
			int m_Height;
			bool m_WindowResized;
	};
	class WindowClosedEvent : public Event
	{
		public:
			WindowClosedEvent() : m_WindowClosed(false) {};
			WindowClosedEvent(const bool p_WindowClosed) : m_WindowClosed(p_WindowClosed) {};
		
			EventType getEventType() const override;
			EventCategory getCategoryFlags() const override;
			std::string getEventName() const override;

			void reset() override;

		protected:

		private:
			bool m_WindowClosed;
	};
}