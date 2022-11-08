#include "Camera.h"

namespace vgl
{
	Camera::~Camera()
	{

	}

	void Camera::update()
	{
		if (m_View == View::Third_Person) {
			#ifdef VGL_USING_VULKAN
				m_ViewMatrix = Matrix4f::lookAtRH(m_ViewZ * m_ViewZModifier + m_LookAtPoint, m_LookAtPoint, m_UpDirection);
			#endif
			#ifdef VGL_USING_OPENGL
				m_ViewMatrix = Matrix4f::lookAtLH(m_ViewZ + m_LookAtPoint, m_LookAtPoint, m_UpDirection);
			#endif

			position = m_ViewZ * m_ViewZModifier;
		}
		else if (m_View == View::First_Person) {
			#ifdef VGL_USING_VULKAN
				m_ViewMatrix = Matrix4f::lookAtRH(position + m_LookAtPoint, position + m_ViewZ + m_LookAtPoint, m_UpDirection);
			#endif
			#ifdef VGL_USING_OPENGL
				m_ViewMatrix = Matrix4f::lookAtLH(position + m_LookAtPoint, position + m_ViewZ + m_LookAtPoint, m_UpDirection);
			#endif
		}
	}

	Matrix4f& Camera::getViewMatrix()
	{
		return m_ViewMatrix;
	}
	Matrix4f& Camera::getPerspectiveMatrix()
	{
		return m_PerspectiveMatrix;
	}
	Matrix4f& Camera::getOrthographicMatrix()
	{
		return m_OrthographicMatrix;
	}

	Vector3f& Camera::getPosition()
	{
		return position;
	}

	const Matrix4f& Camera::setPerspectiveMatrix(const float p_AspectRatio)
	{
		m_AspectRatio = p_AspectRatio;

		#ifdef VGL_USING_VULKAN
			m_PerspectiveMatrix = Matrix4f::perspectiveRH_ZO(m_FieldOfView, m_AspectRatio, m_NearPlane, m_FarPlane);
		#endif
		#ifdef VGL_USING_OPENGL
			m_PerspectiveMatrix = Matrix4f::perspective(m_FieldOfView, m_AspectRatio, m_NearPlane, m_FarPlane);
		#endif

		return m_PerspectiveMatrix;
	}
	const Matrix4f& Camera::setOrthographicMatrix(
		const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar
	)
	{
		m_OrthographicMatrix = Matrix4f::orthoRH_ZO(left, right, bottom, top, zNear, zFar);
		return m_OrthographicMatrix;
	}

	// Is in Radian
	void Camera::setYaw(const float p_Yaw)
	{
		m_Yaw = p_Yaw;
		setViewDirection(m_Yaw, m_Pitch, m_Roll);
	}
	void Camera::setPitch(const float p_Pitch)
	{
		m_Pitch = p_Pitch;
		setViewDirection(m_Yaw, m_Pitch, m_Roll);
	}
	void Camera::setRoll(const float p_Roll)
	{
		m_Roll = p_Roll;
		setViewDirection(m_Yaw, m_Pitch, m_Roll);
	}

	void Camera::setViewDirection(const float p_Yaw, const float p_Pitch, const float p_Roll)
	{
		m_Yaw = p_Yaw;
		m_Pitch = p_Pitch;
		m_Roll = p_Roll;

		m_ViewZ.x = std::cos(m_Yaw) * std::cos(m_Pitch);
		m_ViewZ.y = std::sin(m_Pitch);
		m_ViewZ.z = std::sin(m_Yaw) * std::cos(m_Pitch);

		m_ViewZ = Math::normalize(m_ViewZ);
		m_ViewX = Math::normalize(Math::cross(m_ViewZ, m_UpDirection));
		m_ViewY = Math::normalize(Math::cross(m_ViewX, m_ViewZ));
	}

	void Camera::setView(const View p_View)
	{
		m_View = p_View;
	}
	const float& Camera::getFieldOfView()
	{
		return m_FieldOfView;
	}
	void Camera::lookAt(const Vector3f p_LookAtPoint)
	{
		m_LookAtPoint = p_LookAtPoint;
	}
}

