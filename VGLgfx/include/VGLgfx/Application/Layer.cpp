#include "Layer.h"

namespace vgl
{
	uint32_t Layer::ID = 0;

	//void Layer::onKeyEvent(Event::Event& p_Event)
	//{
	//	Event::EventDispatcher dispatcher(p_Event);
	//	
	//	dispatcher.dispatch<Event::KeyPressedEvent>(&Layer::onKeyPressed, this);
	//	dispatcher.dispatch<Event::KeyReleasedEvent>(&Layer::onKeyReleased, this);
	//	dispatcher.dispatch<Event::KeyRepeatedEvent>(&Layer::onKeyRepeated, this);
	//	dispatcher.dispatch<Event::KeyTypedEvent>(&Layer::onKeyTyped, this);
	//}
	//void Layer::onMouseEvent(Event::Event& p_Event)
	//{
	//	Event::EventDispatcher dispatcher(p_Event);
	//
	//	dispatcher.dispatch<Event::MouseButtonPressedEvent>(&Layer::onMousePressed, this);
	//	dispatcher.dispatch<Event::MouseButtonReleasedEvent>(&Layer::onMouseReleased, this);
	//	dispatcher.dispatch<Event::MouseScrolledEvent>(&Layer::onScrollEvent, this);
	//}
	//void Layer::onCursorEvent(Event::Event& p_Event)
	//{
	//	Event::EventDispatcher dispatcher(p_Event);
	//			
	//	dispatcher.dispatch<Event::MouseMovedEvent>(&Layer::onMouseMove, this);
	//}
	//
	//void Layer::setAsEventHandler(){
	//	std::function<void(Event::Event&)> keyFun(std::bind(&Layer::onKeyEvent, this, std::placeholders::_1));
	//	std::function<void(Event::Event&)> mouseFun(std::bind(&Layer::onMouseEvent, this, std::placeholders::_1));
	//	std::function<void(Event::Event&)> cursorFun(std::bind(&Layer::onCursorEvent, this, std::placeholders::_1));
	//
	//	InputManager::setKeyEventCallback(keyFun);
	//	InputManager::setMouseEventCallback(mouseFun);
	//	InputManager::setCursorEventCallback(cursorFun);
	//}
}
