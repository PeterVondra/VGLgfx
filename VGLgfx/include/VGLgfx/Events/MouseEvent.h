#pragma once

#include "Event.h"

namespace Event
{
	class MouseButtonEvent : public Event
	{
		public:

			int getMouseButtonCode() const;

			void resetDef();

		protected:
			MouseButtonEvent(const int p_MouseButtonCode)
				: m_MouseButtonCode(p_MouseButtonCode) {};

		private:
			int m_MouseButtonCode;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
		public:
			MouseButtonPressedEvent()
				: MouseButtonEvent(0),
				m_MouseButtonPressed(false) {};
			MouseButtonPressedEvent(const int p_MouseButtonCode)
				: MouseButtonEvent(p_MouseButtonCode),
				m_MouseButtonPressed(true) {};
			MouseButtonPressedEvent(const int p_MouseButtonCode, const bool p_MouseButtonPressed)
				: MouseButtonEvent(p_MouseButtonCode),
				m_MouseButtonPressed(p_MouseButtonPressed) {};

			static EventType getStaticEventType();
			EventType getEventType() const override;
			EventCategory getCategoryFlags() const override;
			std::string getEventName() const override;

			bool getMouseButtonPressed() { return m_MouseButtonPressed; }

			void reset() override;

			bool mouseButtonPressed() { return m_MouseButtonPressed; }

		protected:

		private:
			bool m_MouseButtonPressed;
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
		public:
			MouseButtonReleasedEvent()
				: MouseButtonEvent(0),
				m_MouseButtonReleased(false) {};
			MouseButtonReleasedEvent(const int p_MouseButtonCode)
				: MouseButtonEvent(p_MouseButtonCode),
				m_MouseButtonReleased(true) {};
			MouseButtonReleasedEvent(const int p_MouseButtonCode, const bool p_MouseButtonReleased)
				: MouseButtonEvent(p_MouseButtonCode),
				m_MouseButtonReleased(p_MouseButtonReleased) {};

			static EventType getStaticEventType();
			EventType getEventType() const override;
			EventCategory getCategoryFlags() const override;
			std::string getEventName() const override;

			void reset() override;

			bool mouseButtonReleased() { return m_MouseButtonReleased; }

		protected:

		private:
			bool m_MouseButtonReleased;
	};

	class MouseMovedEvent : public Event
	{
		public:
			MouseMovedEvent()
				: m_PositionX(0),
				m_PositionY(0),
				m_MouseMoved(false) {};
			MouseMovedEvent(const double p_PositionX, const double p_PositionY)
				: m_PositionX(p_PositionX),
				m_PositionY(p_PositionY),
				m_MouseMoved(false) {};
			MouseMovedEvent(const double p_PositionX, const double p_PositionY, const bool p_MouseMoved)
				: m_PositionX(p_PositionX),
				m_PositionY(p_PositionY),
				m_MouseMoved(p_MouseMoved) {};

			static EventType getStaticEventType();
			EventType getEventType() const override;
			EventCategory getCategoryFlags() const override;
			std::string getEventName() const override;

			double getX() const;
			double getY() const;

			void reset() override;

			bool mouseMoved() { return m_MouseMoved; }

		protected:

		private:
			double m_PositionX;
			double m_PositionY;
			bool m_MouseMoved;
	};

	class MouseScrolledEvent : public Event
	{
		public:
			MouseScrolledEvent()
				: m_OffsetX(0),
				m_OffsetY(0),
				m_MouseScrolled(false) {};
			MouseScrolledEvent(const double p_OffsetX, const double p_OffsetY)
				: m_OffsetX(p_OffsetX),
				m_OffsetY(p_OffsetY),
				m_MouseScrolled(true) {};
			MouseScrolledEvent(const double p_OffsetX, const double p_OffsetY, const bool p_MouseScrolled)
				: m_OffsetX(p_OffsetX),
				m_OffsetY(p_OffsetY),
				m_MouseScrolled(p_MouseScrolled) {};

			static EventType getStaticEventType();
			EventType getEventType() const override;
			EventCategory getCategoryFlags() const override;
			std::string getEventName() const override;

			double getOffsetX() const;
			double getOffsetY() const;

			void reset() override;

			bool mouseScrolled() { return m_MouseScrolled; }

		protected:

		private:
			double m_OffsetX;
			double m_OffsetY;
			bool m_MouseScrolled;
	};
}
