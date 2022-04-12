#pragma once

#include <iostream>

#include "../../VGL-2D/Circle.h"
#include "../../VGL-2D/Rectangle.h"
#include "AABBCollider.h"

namespace vgl
{
	namespace Physics2D
	{
		class CircleCollider
		{
			public:
				CircleCollider();
				~CircleCollider();

				static bool checkIntersection(Circle& aabb_box1, Rectangle& aabb_box2, Resolution resolution);
				static bool checkIntersection(Rectangle& aabb_box1, Circle& aabb_box2, Resolution resolution);
				static bool checkIntersection(Circle& aabb_box1, Circle& aabb_box2, Resolution resolution);
				static bool checkIntersection(Vector2f position, Circle& aabb_box2);

				static bool checkIntersection(Circle& aabb_box1, Rectangle& aabb_box2);
				static bool checkIntersection(Circle& aabb_box1, Circle& aabb_box2);

				static bool isTouching(Rectangle& aabb_box1, Rectangle& aabb_box2);
				static bool isTouching(Vector2f position, Rectangle& aabb_box2);

			protected:
			private:

		};
	}
}