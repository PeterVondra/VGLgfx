#pragma once

#include <iostream>
#include "../Math/Vector.h"
#include "../Math/Transform.h"
#include "../BufferLayout.h"

namespace vgl
{
	namespace vk
	{
		class Renderer;
	}

	class Triangle : public Transform2D
	{
		public:
			Triangle();
			Triangle(const float p_Length, const float p_Height, const Vector2f p_Position, const Vector3f p_Color);
			~Triangle();

			void setSize(const float p_Length, const float p_Height);
			void setColor(const Vector3f p_Color);
			void reset();

			void setOrigin(const float x, const float y);
			void setOrigin(const Vector2f p_Origin);
			void setOutlineColor(Vector3f p_Color);
			void setOutlineThickness(const float p_Thickness);

			const Vector3f getColor();
			const Vector2f getPosition();
			const Vector3f getOutlineColor();
			const float    getOutlineThickness();

			const float	   getRotation();
			const Vector2f getScalar();
			const Vector2f getSize();
			const Vector2f getOrigin();

		protected:
		private:
			friend class vk::Renderer;

			std::vector<Vector2f> m_RawVertices;
			std::vector<unsigned int> m_RawIndices;
	
			Vector2f m_Size;
			Vector2f m_Position;
			Vector3f m_Color;
	
			float m_OutlineThickness;
			Vector3f m_OutlineColor;
	};
}
