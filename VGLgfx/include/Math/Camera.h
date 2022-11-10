#pragma once

#include <iostream>
#include <cmath>
#include "Matrix.h"
#include "Vector.h"
#include "Math.h"
#include "Transform.h"
#include "../Events/Event.h"
#include "../Events/KeyEvent.h"
#include "../Events/MouseEvent.h"
#include "../Application/InputManager.h"

namespace vgl
{
	enum class View { Third_Person, First_Person };

	// #ifdef VGL_USING_VULKAN
	// Camera will use 0 to 1 depth
	// #ifdef VGL_USING_OPENGL
	// Camera will use -1 to 1 depth

	/* 
	Camera works for both 2D and 3D
	(Yaw, Pitch, Roll) is in radians, use Math::DEG2RAD to convert to degrees
	*/
	class Camera : public Transform3D
	{
		public:
			Camera(const Vector3f p_UpDirection = { 0, 1, 0 }, const Vector3f p_LookAtPoint = { 0, 0, 0 }) 
				: m_UpDirection(p_UpDirection),
				m_LookAtPoint(p_LookAtPoint),
				m_FieldOfView(90.0f),
				m_ViewX(1, 0, 0), m_ViewY(0, 1, 0), m_ViewZ(0.0f, 0.0f, 1.0f),
				m_AspectRatio(1.0f),
				m_Yaw(0.0f), m_Pitch(0.0f), m_Roll(0.0f), 
				m_ViewZModifier(1.0f),
				m_NearPlane(1.0f), m_FarPlane(10.0f)
			{
				setView(View::Third_Person);
			}
			~Camera();

			void update();

			Matrix4f& getViewMatrix();
			Matrix4f& getPerspectiveMatrix();
			Matrix4f& getOrthographicMatrix();
			Vector3f& getPosition();

      void setPerspectiveMatrix(const Matrix4f& p_Perspective) { m_PerspectiveMatrix = p_Perspective; }
      void setOrthographicMatrix(const Matrix4f& p_Orthographic) { m_OrthographicMatrix = p_Orthographic; }  

			const Matrix4f& setPerspectiveMatrix(const float p_AspectRatio);
			const Matrix4f& setOrthographicMatrix(
				const float left, const float right,
				const float bottom, const float top,
				const float zNear, const float zFar
			);

			// Is in Radian
			void setYaw(const float p_Yaw);
			void setPitch(const float p_Pitch);
			void setRoll(const float p_Roll);
      void setFOV(const float p_FOV){ m_FieldOfView = p_FOV; }
	  void setAspectRatio(const float p_AspectRatio);
      void setViewMatrix(const Matrix4f p_ViewMatrix) { m_ViewMatrix = p_ViewMatrix; }

			void moveFromCameraView(const Vector3f p_Offset)
			{  
				position += m_ViewX * p_Offset.x;
				position += m_ViewY * p_Offset.y;
				position += m_ViewZ * p_Offset.z;
			};

			void setViewDirection(const float p_Yaw, const float p_Pitch, const float p_Roll);
			Vector3f& getViewDirection() { return m_ViewZ; }

			void setView(const View p_View);
			const float& getFieldOfView();
			void lookAt(const Vector3f p_LookAtPoint);
      
      float getViewModifier() { return m_ViewZModifier; }
			void setViewModifier(const float p_Modifier) { m_ViewZModifier = p_Modifier; }
			View getView() { return m_View; }

			float getAspectRatio() { return m_AspectRatio; }

		private:
			View m_View;

			Matrix4f m_ViewMatrix; // LookAt Matrix
			Matrix4f m_PerspectiveMatrix; // Projection
			Matrix4f m_OrthographicMatrix; // Projection

			Vector3f m_LookAtPoint;
			Vector3f m_UpDirection;

			Vector3f m_ViewX; // Normalized
			Vector3f m_ViewY; // Normalized
			Vector3f m_ViewZ; // Normalized

			float m_FieldOfView;
			float m_Yaw;
			float m_Pitch;
			float m_Roll;
			float m_ViewZModifier;

			float m_AspectRatio;
			float m_NearPlane;
			float m_FarPlane;
	};

	/*============================*/
	/* Default Camera Controllers */
	/*============================*/

	enum class CameraMovement { Front, Back, Left, Right, Up, Down };

	// Orthographic 2D Projection
	class Ortho2DCameraController
	{
		public:
			Ortho2DCameraController();
			~Ortho2DCameraController();

			void onMouseMovedEvent(Event::MouseMovedEvent& p_MouseMovedEvent);
			void onMousePressedEvent(Event::MouseButtonPressedEvent& p_MousePressedEvent);
			void onMouseScrolledEvent(Event::MouseScrolledEvent& p_MouseScrolledEvent);
			void onKeyPressedEvent(Event::KeyPressedEvent& p_KeyPressedEvent);

		private:
	};
	// Orthographic 3D Projection
	class Ortho3DCameraController
	{
		public:
			Ortho3DCameraController();
			~Ortho3DCameraController();

			void onMouseMovedEvent(Event::MouseMovedEvent& p_MouseMovedEvent);
			void onMousePressedEvent(Event::MouseButtonPressedEvent& p_MousePressedEvent);
			void onMouseScrolledEvent(Event::MouseScrolledEvent& p_MouseScrolledEvent);
			void onKeyPressedEvent(Event::KeyPressedEvent& p_KeyPressedEvent);

		private:
	};

	// Perspective Projection
	class CameraController3D
	{
		public:
			CameraController3D()
				: m_CameraPtr(nullptr), m_DeltaTimePtr(nullptr), m_Enabled(true), m_ConstrainPitch(true),
				m_Sensitivity(70), m_MovementSpeed(200), m_Zoom(600), m_Yaw(0), m_Pitch(0), m_PrevCursorPos({ 0, 0 }) {}
			~CameraController3D() {};

			void bind(Camera& p_Camera) 
			{
				m_CameraPtr = &p_Camera;
				m_CameraPtr->setViewModifier(m_Zoom);
			};
			void bind(Camera* p_Camera) 
			{
				m_CameraPtr = p_Camera;
				m_CameraPtr->setViewModifier(m_Zoom);
			};

			void setZoom(const float p_Zoom) { m_Zoom = p_Zoom; }

			void setDeltaTime(const double& p_DeltaTime) { m_DeltaTimePtr = &const_cast<double&>(p_DeltaTime); }

			void update()
			{
				if (m_Enabled) {
					if (vgl::Input::keyIsDown(vgl::Key::W))
						processKeyboard(CameraMovement::Front);
					if (vgl::Input::keyIsDown(vgl::Key::S))
						processKeyboard(CameraMovement::Back);
					if (vgl::Input::keyIsDown(vgl::Key::A))
						processKeyboard(CameraMovement::Left);
					if (vgl::Input::keyIsDown(vgl::Key::D))
						processKeyboard(CameraMovement::Right);
					if (vgl::Input::keyIsDown(vgl::Key::Space))
						processKeyboard(CameraMovement::Up);
					if (vgl::Input::keyIsDown(vgl::Key::LeftShift))
						processKeyboard(CameraMovement::Down);
				}
			}

			void onMouseMovedEvent(Event::MouseMovedEvent& p_MouseMovedEvent)
			{
				if (!m_Enabled) {
					m_PrevCursorPos.x = p_MouseMovedEvent.getX();
					m_PrevCursorPos.y = p_MouseMovedEvent.getY();
				}

				if (m_Enabled)
				{
					if (Input::mouseButtonIsDown(0) && m_CameraPtr->getView() == View::Third_Person || m_CameraPtr->getView() == View::First_Person) {
						float xOffset = m_PrevCursorPos.x - p_MouseMovedEvent.getX();
						float yOffset = p_MouseMovedEvent.getY() - m_PrevCursorPos.y;

						if (std::isinf(xOffset) || std::isinf(yOffset) || std::isnan(xOffset) || std::isnan(yOffset))
							return;

						double d = m_Sensitivity / 1000;
						m_Yaw -= xOffset * d * Math::DEG2RAD;
						m_Pitch -= yOffset * d * Math::DEG2RAD;

						if (m_ConstrainPitch)
						{
							if (m_Pitch > 89.0f * Math::DEG2RAD)
								m_Pitch = 89.0f * Math::DEG2RAD;
							else if (m_Pitch < -89.0f * Math::DEG2RAD)
								m_Pitch = -89.0f * Math::DEG2RAD;
						}
						m_CameraPtr->setViewDirection(m_Yaw, m_Pitch, 0);
						m_CameraPtr->update();
					}
				}
				m_PrevCursorPos.x = p_MouseMovedEvent.getX();
				m_PrevCursorPos.y = p_MouseMovedEvent.getY();
			}

			void onMouseScrolledEvent(Event::MouseScrolledEvent& p_MouseScrolledEvent)
			{
				if (m_Enabled) {
					m_Zoom -= p_MouseScrolledEvent.getOffsetY() * 50;
					m_CameraPtr->setViewModifier(m_Zoom);
				}
			}

			bool isEnabled() { return m_Enabled; }
			void enable(const bool p_Enabled) { m_Enabled = p_Enabled; }
			void constrainPitch(const bool p_ConstrainPitch) { m_ConstrainPitch = p_ConstrainPitch; }
			bool  isPitchConstrained(){ return m_ConstrainPitch; }

			float getSensitivity() { return m_Sensitivity; }
			float getZoom() { return m_Zoom; }
			float getYaw() { return m_Yaw; }
			float getPitch() { return m_Pitch; }

			void setYaw(const float p_Yaw) { m_Yaw = p_Yaw; }
			void setPitch(const float p_Pitch) { m_Pitch = p_Pitch; }

			void setMovementSpeed(const float p_MovementSpeed) { m_MovementSpeed = p_MovementSpeed; }
			void setSensitivity(const float p_Sensitivity) { m_Sensitivity = p_Sensitivity; }


			float m_MovementSpeed;
		private:
			friend class Scene;

			Camera* m_CameraPtr;
			double* m_DeltaTimePtr;

			bool m_Enabled;
			bool m_ConstrainPitch;
			float m_Sensitivity;
			float m_Zoom;

			float m_Yaw;
			float m_Pitch;

			Vector2f m_PrevCursorPos;

			void processKeyboard(CameraMovement direction)
			{
				if (direction == CameraMovement::Front)
					m_CameraPtr->moveFromCameraView({ 0, 0, m_MovementSpeed * (float)(*m_DeltaTimePtr) });
				else if (direction == CameraMovement::Back)
					m_CameraPtr->moveFromCameraView({ 0, 0, -m_MovementSpeed * (float)(*m_DeltaTimePtr) });
				else if (direction == CameraMovement::Left)
					m_CameraPtr->moveFromCameraView({ -m_MovementSpeed * (float)(*m_DeltaTimePtr), 0, 0 });
				else if (direction == CameraMovement::Right)
					m_CameraPtr->moveFromCameraView({ m_MovementSpeed * (float)(*m_DeltaTimePtr), 0, 0 });
				else if (direction == CameraMovement::Up)
					m_CameraPtr->move({ 0, m_MovementSpeed * (float)(*m_DeltaTimePtr), 0 });
				else if (direction == CameraMovement::Down)
					m_CameraPtr->move({ 0, -m_MovementSpeed * (float)(*m_DeltaTimePtr), 0 });
			}
	};
}
