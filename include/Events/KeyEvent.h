#pragma once

#include "Event.h"

namespace Event
{
	class KeyEvent : public Event
	{
		public:
			KeyEvent(const int p_KeyCode) : m_KeyCode(p_KeyCode) {};

			static EventType getStaticEventType();
			EventType getEventType() const override;

			static EventCategory getStaticCategoryFlags();
			EventCategory getCategoryFlags() const override;

			std::string getEventName() const override;

			int getKeyCode() const;
			void resetDef();
			void reset() override { resetDef(); }

		protected:

		private:
			int m_KeyCode;
	};

	class KeyTypedEvent : public KeyEvent
	{
		public:
			KeyTypedEvent() 
				: KeyEvent(0),
				m_KeyTyped(false) {};
			KeyTypedEvent(const int p_KeyCode) 
				: KeyEvent(p_KeyCode),
				m_KeyTyped(true) {};
			KeyTypedEvent(const int p_KeyCode, const bool p_KeyTyped) 
				: KeyEvent(p_KeyCode),
				m_KeyTyped(p_KeyTyped) {};

			static EventType getStaticEventType();
			EventType getEventType() const override;

			static EventCategory getStaticCategoryFlags();
			EventCategory getCategoryFlags() const override;

			std::string getEventName() const override;

			void reset() override;

			bool keyTyped() { return m_KeyTyped; };

		protected:
		private:
			bool m_KeyTyped;
	};

	class KeyPressedEvent : public KeyEvent
	{
		public:
			KeyPressedEvent() 
				: KeyEvent(0),
				m_KeyPressed(false) {};
			KeyPressedEvent(const int p_KeyCode) 
				: KeyEvent(p_KeyCode),
				m_KeyPressed(true) {};
			KeyPressedEvent(const int p_KeyCode, const bool p_KeyPressed) 
				: KeyEvent(p_KeyCode),
				m_KeyPressed(p_KeyPressed) {};

			static EventType getStaticEventType();
			EventType getEventType() const override;

			static EventCategory getStaticCategoryFlags();
			EventCategory getCategoryFlags() const override;

			std::string getEventName() const override;

			void reset() override;

			bool keyPressed() { return m_KeyPressed; }

		protected:

		private:
			bool m_KeyPressed;
	};

	class KeyReleasedEvent : public KeyEvent
	{
		public:
			KeyReleasedEvent()
				: KeyEvent(0),
				m_KeyReleased(false) {};
			KeyReleasedEvent(const int p_KeyCode)
				: KeyEvent(p_KeyCode),
				m_KeyReleased(true) {};
			KeyReleasedEvent(const int p_KeyCode, const bool p_KeyReleased)
				: KeyEvent(p_KeyCode),
				m_KeyReleased(p_KeyReleased) {};

			static EventType getStaticEventType();
			EventType getEventType() const override;

			static EventCategory getStaticCategoryFlags();
			EventCategory getCategoryFlags() const override;

			std::string getEventName() const override;

			void reset() override;

			bool keyReleased() { return m_KeyReleased; }

		protected:

		private:
			bool m_KeyReleased;
	};

	class KeyRepeatedEvent : public KeyEvent
	{
		public:
			KeyRepeatedEvent()
				: KeyEvent(0),
				m_KeyRepeated(false) {};
			KeyRepeatedEvent(const int p_KeyCode)
				: KeyEvent(p_KeyCode),
				m_KeyRepeated(true) {};
			KeyRepeatedEvent(const int p_KeyCode, const bool p_KeyRepeated)
				: KeyEvent(p_KeyCode),
				m_KeyRepeated(p_KeyRepeated) {};

			static EventType getStaticEventType();
			EventType getEventType() const override;
			static EventCategory getStaticCategoryFlags();
			EventCategory getCategoryFlags() const override;
			std::string getEventName() const override;

			void reset() override;

			bool keyRepeated() { return m_KeyRepeated; }
		protected:

		private:
			bool m_KeyRepeated;
	};
}