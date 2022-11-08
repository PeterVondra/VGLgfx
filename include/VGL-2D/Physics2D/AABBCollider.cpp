#include "AABBCollider.h"

namespace vgl
{
	namespace Physics2D
	{
		AABBCollider::AABBCollider()
		{

		}
		AABBCollider::~AABBCollider()
		{

		}

		bool AABBCollider::checkIntersection(Rectangle& aabb_box1, Rectangle& aabb_box2, Resolution p_Resolution)
		{
			Vector2f halfSize1 = aabb_box1.getSize() / 2;
			Vector2f halfSize2 = aabb_box2.getSize() / 2;

			Vector2f deltaPosition = aabb_box2.getPosition() - aabb_box1.getPosition();
			Vector2f penetration = Math::abs(deltaPosition) - (halfSize1 + halfSize2);

			if (penetration.x < 0.0f && penetration.y < 0.0f)
			{
				if (p_Resolution == Resolution::stop)
				{
					if (penetration.x > penetration.y)
					{
						if (deltaPosition.x > 0.0f)
							aabb_box1.move(penetration.x, 0.0f);
						else
							aabb_box1.move(-penetration.x, 0.0f);
					}
					else
					{
						if (deltaPosition.y > 0.0f)
							aabb_box1.move(0.0f, penetration.y);
						else
							aabb_box1.move(0.0f, -penetration.y);
					}
				}
				else if (p_Resolution == Resolution::push)
				{
					if (penetration.x > penetration.y)
					{
						if (deltaPosition.x > 0.0f)
						{
							aabb_box2.move(-penetration.x, 0.0f);
							return true;
						}
						aabb_box2.move(penetration.x, 0.0f);
					}
					else
					{
						if (deltaPosition.y > 0.0f)
						{
							aabb_box2.move(0.0f, -penetration.y);
							return true;
						}
						aabb_box2.move(0.0f, penetration.y);
					}
				}
				return true;
			}
			return false;
		}
		bool AABBCollider::checkIntersection(Rectangle& aabb_box1, Rectangle& aabb_box2)
		{
			Vector2f halfSize1 = aabb_box1.getSize() / 2;
			Vector2f halfSize2 = aabb_box2.getSize() / 2;

			Vector2f deltaPosition = aabb_box2.getPosition() - aabb_box1.getPosition();
			Vector2f penetration = Math::abs(deltaPosition) - (halfSize1 + halfSize2);

			if (penetration.x < 0.0f && penetration.y < 0.0f)
				return true;
			return false;
		}
		bool AABBCollider::checkIntersection(Vector2f position, Rectangle& aabb_box2)
		{
			Vector2f halfSize2 = aabb_box2.getSize() / 2;

			Vector2f temp_deltaPosition = aabb_box2.getPosition() - position;
			Vector2f temp_penetration = Math::abs(temp_deltaPosition) - halfSize2;

			if (temp_penetration.x < 0.0f && temp_penetration.y < 0.0f)
				return true;
			return false;
		}
		bool AABBCollider::checkIntersection(Vector2f position, Rect2D p_Rect2D)
		{
			Vector2f halfSize2 = p_Rect2D.size.getVec2f() / 2;

			Vector2f temp_deltaPosition = (p_Rect2D.position + halfSize2) - position;
			Vector2f temp_penetration = Math::abs(temp_deltaPosition) - halfSize2;

			if (temp_penetration.x < 0.0f && temp_penetration.y < 0.0f)
				return true;
			return false;
		}
		bool AABBCollider::isTouching(Rectangle& aabb_box1, Rectangle& aabb_box2)
		{
			Vector2f halfSize1 = aabb_box1.getSize() / 2;
			Vector2f halfSize2 = aabb_box2.getSize() / 2;

			Vector2f deltaPosition = aabb_box2.getPosition() - aabb_box1.getPosition();
			Vector2f penetration = Math::abs(deltaPosition) - (halfSize1 + halfSize2);

			if (penetration.x == 0.0f || penetration.y == 0.0f)
				return true;
			return false;
		}
		bool AABBCollider::isTouching(Vector2f position, Rectangle& aabb_box2)
		{
			Vector2f halfSize2 = aabb_box2.getSize() / 2;

			Vector2f temp_deltaPosition = aabb_box2.getPosition() - position;
			Vector2f temp_penetration = Math::abs(temp_deltaPosition) - halfSize2;

			if (temp_penetration.x == 0.0f || temp_penetration.y == 0.0f)
				return true;
			return false;
		}
	}
}