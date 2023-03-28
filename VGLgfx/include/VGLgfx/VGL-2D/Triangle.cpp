#include "Triangle.h"

namespace vgl
{
	Triangle::Triangle()
	{

	}
	Triangle::Triangle(const float p_Length, const float p_Height, const Vector2f p_Position, const Vector3f p_Color)
	{
		position = Vector2f(0);
		scalar = Vector2f(1);
		rotation = 0;
		model.identity();

		m_Origin = Vector2f(0);

		m_RawVertices =
		{
			Vector2f(0.0f,	-1.0f),
			Vector2f(1.0f,	-1.0f),
			Vector2f(1.0f,	 1.0f)
		};

		vgl::BufferLayout layout = { {{ vgl::ShaderDataType::Vec2f, 0, "in_Position"}},0 };

		//m_Vertices.setLayout(layout);
		//m_Vertices.fill(m_RawVertices);
		//m_Vao.fill(m_Vertices);

		setSize(p_Length, p_Height);
		setPosition(p_Position);
		setColor(p_Color);

		//m_Shader.setShader("resources/shaders/2DShape/vert.spv", "resources/shaders/2DShape/frag.spv");
	}
	Triangle::~Triangle()
	{

	}

	void Triangle::setSize(const float p_Length, const float p_Height)
	{
		setScale(p_Length, p_Height);
	}
	void Triangle::setColor(const Vector3f p_Color)
	{
		m_Color = p_Color;
		//m_UniformManager.setUniform(vk::ShaderStage::VertexBit, "ubo", m_Color, sizeof(Matrix4f));
	}
	void Triangle::reset()
	{

	}

	void Triangle::setOrigin(const float x, const float y)
	{
		m_Origin = Vector2f(x, y);
	}
	void Triangle::setOrigin(const Vector2f p_Origin)
	{
		m_Origin = p_Origin;
	}
	void Triangle::setOutlineColor(Vector3f p_Color)
	{
		m_OutlineColor = p_Color;
	}
	void Triangle::setOutlineThickness(const float p_Thickness)
	{
		m_OutlineThickness = p_Thickness;
	}
	const Vector3f Triangle::getColor()
	{
		return m_Color;
	}
	const Vector2f Triangle::getPosition()
	{
		return position;
	}
	const Vector3f Triangle::getOutlineColor()
	{
		return m_OutlineColor;
	}
	const float Triangle::getOutlineThickness()
	{
		return m_OutlineThickness;
	}

	const Vector2f Triangle::getOrigin()
	{
		return m_Origin;
	}

	const float Triangle::getRotation()
	{
		return rotation;
	}
	const Vector2f Triangle::getScalar()
	{
		return scalar;
	}
	const Vector2f Triangle::getSize()
	{
		return m_Size;
	}
}