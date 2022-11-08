#pragma once

#include <iostream>

#include "../Math/Transform.h"
#include "../VGL_Internal.h"

namespace vgl
{
	namespace vk
	{
		class Renderer;
	}

	class Rectangle : public Transform2D
	{
		public:
			Rectangle();
			Rectangle(const Rectangle& p_Rectangle);
			Rectangle(const Vector2f size, const Vector2f position, const Vector3f color);
			Rectangle(const Vector2f size, const Vector2f position, const Vector3f color, bool p_Init);
			~Rectangle();

			void init(const Vector2f size, const Vector2f position, const Vector3f color);

			virtual void setSize(const Vector2f size);
			void setColor(const Vector3f p_Color);
			void setOpacity(const float p_Opacity);
			void reset();
			void init();

			void setOrigin(const float x, const float y);
			void setOrigin(const Vector2f p_Origin);
			void setOutlineColor(Vector3f p_Color);
			void setOutlineThickness(const float p_Thickness);

			const Vector3f getColor();
			const float	   getOpacity();
			const Vector2f getPosition();
			const Vector3f getOutlineColor();
			const float    getOutlineThickness();

			const float	   getRotation();
			const Vector2f getScalar();
			const Vector2f getSize();
			const Vector2f getOrigin();

	protected:
		void setVertexPosition(const unsigned int index, Vector2f newVertexPosition);
	private:
		friend class vk::Renderer;
		friend class TextBox;

		bool NewRT = true;

		Vector2f m_Size;
		Vector2f m_Position;
		Vector3f m_Color = { 0,0,0 };

		float m_OutlineThickness = 0;
		Vector3f m_OutlineColor = { 0, 0, 0 };

		float m_Opacity = 1.0f;
		
		ShaderDescriptor m_Descriptor;
	};
}
