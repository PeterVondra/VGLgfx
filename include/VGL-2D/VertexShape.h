#pragma once

#include <vector>
#include "../Math/Transform.h"
#include "VGL_Internal.h"

namespace vgl
{
	class VertexShape : public Transform2D
	{
		public:
			VertexShape();
			VertexShape(std::initializer_list<float> p_Vertices);
			VertexShape(std::vector<float> p_Vertices);

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
			const Vector2f getOrigin();

		protected:
			vk::Shader m_Shader;
		private:
			friend class vk::Renderer;
			friend class TextBox;

			std::vector<Vector2f> m_RawVertices;
			std::vector<unsigned int> m_RawIndices;

			Vector2f m_Position;
			Vector3f m_Color;

			float m_OutlineThickness;
			Vector3f m_OutlineColor;

			vk::UniformManager m_UniformManager;
			vk::VertexArray m_Vao;
			vk::VertexBuffer m_Vertices;

	};
}