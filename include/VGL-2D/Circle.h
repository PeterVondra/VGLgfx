#pragma once

#include <iostream>
#include "Rectangle.h"

namespace vgl
{
	class Circle : public Rectangle
	{
		public:
			Circle();
			Circle(const float p_Radius, const Vector2f p_Position, const Vector3f p_Color);

			void setRadius(const float radius);
			float getRadius();

		protected:
		private:
			friend class vk::Renderer;

			using Rectangle::setSize;

			float m_Radius;
	};
}