#pragma once

#include "../Math/Camera.h"

#include "InputManager.h"
#include "../Events/Event.h"
#include "../Events/ApplicationEvent.h"
#include "../Events/KeyEvent.h"
#include "../Events/MouseEvent.h"
#include "RenderAPI.h"

#define USE_IMGUI_API
#include "../../libs/ImGuizmo/ImGuizmo.h"

namespace vgl
{
	class Layer
	{
		public:
			Layer(const std::string p_Name = "Layer " + std::to_string(ID), bool p_Enabled = true, bool p_CameraEnabled = true)
				: m_Name(p_Name), m_Enabled(p_Enabled), m_CameraEnabled(p_CameraEnabled) { ID++; }
			virtual ~Layer() {};

			void useCamera(const bool p_Camera) { m_CameraEnabled = p_Camera; }

			virtual void onAttach() {}
			virtual void onDetach() {}
			virtual void onUpdate(Renderer& p_Renderer) {}
			virtual void onImGuiUpdate() {}

			virtual bool onMouseMove(Event::MouseMovedEvent& p_Move) { return false; };
			virtual bool onMousePressed(Event::MouseButtonPressedEvent& p_Press) { return false; };
			virtual bool onMouseReleased(Event::MouseButtonReleasedEvent& p_Released) { return false; };
			virtual bool onScrollEvent(Event::MouseScrolledEvent& p_Scroll) { return false; };

			virtual bool onKeyPressed(Event::KeyPressedEvent& p_Press) { return false; };
			virtual bool onKeyReleased(Event::KeyReleasedEvent& p_Released) { return false; };
			virtual bool onKeyRepeated(Event::KeyRepeatedEvent& p_Repeated) { return false; };
			virtual bool onKeyTyped(Event::KeyTypedEvent& p_Typed) { return false; };

			inline const std::string& getName() { return m_Name; }
		protected:
			friend class Application;

			Camera m_Camera;

			std::string m_Name;
			static uint32_t ID;

			bool m_Enabled;
			bool m_CameraEnabled;
		private:
			friend class Application;
	};

	
}
