#include "Rectangle.h"

namespace vgl
{
	Rectangle::Rectangle()
	{

	}
	Rectangle::Rectangle(const Rectangle& p_Rectangle)
	{
		*this = p_Rectangle;
	}
	Rectangle::Rectangle(const Vector2f p_Size, const Vector2f p_Position, const Vector3f p_Color)
	{
		position = Vector2f(0);
		scalar = Vector2f(1);
		rotation = 0;
		model.identity();

		m_Origin = Vector2f(0);

		setSize(p_Size);
		setPosition(p_Position);
		setColor(p_Color);
	}
	Rectangle::~Rectangle()
	{
		//m_RawVertices.clear();
		//m_RawIndices.clear();
	}

	Rectangle::Rectangle(const Vector2f p_Size, const Vector2f p_Position, const Vector3f p_Color, bool p_Init)
	{
		position = Vector2f(0);
		scalar = Vector2f(1);
		rotation = 0;
		model.identity();

		m_Origin = Vector2f(0);

		setSize(p_Size);
		setPosition(p_Position);
		setColor(p_Color);
	}
	void Rectangle::init(const Vector2f p_Size, const Vector2f p_Position, const Vector3f p_Color)
	{
		position = Vector2f(0);
		scalar = Vector2f(1);
		rotation = 0;
		model.identity();

		m_Origin = Vector2f(0);

		setSize(p_Size);
		setPosition(p_Position);
		setColor(p_Color);
	}
	void Rectangle::init()
	{
		
	}

	void Rectangle::setSize(const Vector2f p_Size)
	{
		m_Size = p_Size;
		setScale(p_Size.x, p_Size.y);
	}
	void Rectangle::setColor(const Vector3f p_Color)
	{
		m_Color = p_Color;
		m_Descriptor.copy(ShaderStage::VertexBit, m_Color, sizeof(Matrix4f));
	}
	void Rectangle::setOpacity(const float p_Opacity)
	{
		m_Opacity = p_Opacity;
		m_Descriptor.copy(ShaderStage::VertexBit, m_Color, sizeof(Matrix4f));
	}
	void Rectangle::reset()
	{

	}

	void Rectangle::setOrigin(const float x, const float y)
	{
		m_Origin = Vector2f(x, y);
	}
	void Rectangle::setOrigin(const Vector2f p_Origin)
	{
		m_Origin = p_Origin;
	}
	void Rectangle::setOutlineColor(Vector3f p_Color)
	{
		m_OutlineColor = p_Color;
	}
	void Rectangle::setOutlineThickness(const float p_Thickness)
	{
		m_OutlineThickness = p_Thickness;
	}
	const Vector3f Rectangle::getColor()
	{
		return m_Color;
	}
	const float	Rectangle::getOpacity()
	{
		return m_Opacity;
	}
	const Vector2f Rectangle::getPosition()
	{
		return position;
	}
	const Vector3f Rectangle::getOutlineColor()
	{
		return m_OutlineColor;
	}
	const float Rectangle::getOutlineThickness()
	{
		return m_OutlineThickness;
	}

	const Vector2f Rectangle::getOrigin()
	{
		return m_Origin;
	}

	const float Rectangle::getRotation()
	{
		return rotation;
	}
	const Vector2f Rectangle::getScalar()
	{
		return scalar;
	}
	const Vector2f Rectangle::getSize()
	{
		return m_Size;
	}
}