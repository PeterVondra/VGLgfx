#include "Circle.h"

namespace vgl
{
	Circle::Circle()
	{

	}
	Circle::Circle(const float p_Radius, const Vector2f p_Position, const Vector3f p_Color)
		: Rectangle::Rectangle(Vector2f(p_Radius * 2), p_Position, p_Color), m_Radius(p_Radius)
	{
		setRadius(p_Radius);
	}

	void Circle::setRadius(const float p_Radius)
	{
		setSize({ p_Radius * 2, p_Radius * 2 });
	}

	float Circle::getRadius()
	{
		return m_Radius;
	}
}