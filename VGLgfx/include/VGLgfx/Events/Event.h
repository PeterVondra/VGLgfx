#pragma once

#define BIT(x) (1 << x)

#include <functional>
#include <string>

namespace Event
{
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyRepeated, AnyKeyEvent, VoidEvent,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

    enum class EventCategory
    {
        None = 0,
        ApplicationEventCategory = BIT(1),
        InputEventCategory       = BIT(2),
        KeyboardEventCategory    = BIT(3),
        MouseEventCategory       = BIT(4),
        MouseButtonEventCategory = BIT(5)
    };

    class Event
    {
        public:
            Event();
            ~Event();

			virtual EventType getEventType() const;
            virtual EventCategory getCategoryFlags() const;
            virtual std::string getEventName() const;

            bool isInEventCategory(EventCategory event);
    
			virtual void reset();
        protected:
            bool m_EventHandled = false;
        private:
			friend class EventDispatcher;

			static EventType getStaticEventType();
			static EventCategory getStaticCategoryFlags();
    };

	template<typename T, class C> 
	using EventCallbackC = bool(C::*)(T&);
	template<typename T> 
	using EventCallback = bool(*)(T&);

    class EventDispatcher
    {
        public:
			EventDispatcher(Event& p_Event) : m_Event(&p_Event) {};
			
			template<typename T>
			bool dispatch(EventCallback<T> p_Func)
			{				
				if (m_Event->getEventType() == T::getStaticEventType() && !m_Event->m_EventHandled)
				{
					//type casting type T* to &m_Event
					//which should return the T type but as a pointer, so we then dereference it to pass it to the callback function.
					m_Event->m_EventHandled = p_Func(*(T*)m_Event);

					//m_Event.m_EventHandled = p_Func(*(T*)& m_Event);
					return true;
				}
				return false;
			}
			template<typename T, class C>
			bool dispatch(EventCallbackC<T, C> p_Func, C* p_Instance)
			{			
				if (m_Event) {
					if (m_Event->getEventType() == T::getStaticEventType() && !m_Event->m_EventHandled)
					{
						//type casting type T* to &m_Event
						//which should return the T type but as a pointer, so we then dereference it to pass it to the callback function.
						m_Event->m_EventHandled = (p_Instance->*p_Func)(*(T*)m_Event);

						//m_Event.m_EventHandled = p_Func(*(T*)& m_Event);
						return true;
					}
				}
				return false;
			}

			const Event& getEvent() const
			{
				return *m_Event;
			}

        protected:
        private:
			Event* m_Event;

    };
};