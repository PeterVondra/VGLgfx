#pragma once

#include <iostream>

#include "../../VGL-Core/Math/Math.h"
#include "../../VGL-Core/Math/Vector.h"
#include "../../VGL-2D/Rectangle.h"

namespace vgl
{
	namespace Physics2D
	{
		enum class Resolution
		{
			none = -1,
			stop = 0,
			push = 1
		};
		struct Rect2D
		{
			Rect2D(Vector2f p_Position, Vector2i p_Size) : position(p_Position), size(p_Size) {}
			Rect2D(Vector2f p_Position, Vector2f p_Size) : position(p_Position), size(p_Size.x, p_Size.y) {}
			Vector2f position = 0;
			Vector2i size = 0;
		};
		class AABBCollider
		{
			public:
				AABBCollider();
				~AABBCollider();

				static bool checkIntersection(Rectangle& aabb_box1, Rectangle& aabb_box2, Resolution p_Resolution);
				static bool checkIntersection(Rectangle& aabb_box1, Rectangle& aabb_box2);
				static bool checkIntersection(Vector2f position, Rectangle& aabb_box2);
				static bool checkIntersection(Vector2f position, Rect2D p_Rect2D);

				static bool isTouching(Rectangle& aabb_box1, Rectangle& aabb_box2);
				static bool isTouching(Vector2f position, Rectangle& aabb_box2);

			protected:
			private:
		};
	}
}