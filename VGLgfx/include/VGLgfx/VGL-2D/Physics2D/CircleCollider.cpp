#include "CircleCollider.h"

namespace vgl
{
	namespace Physics2D
	{
		CircleCollider::CircleCollider()
		{
			
		}

		CircleCollider::~CircleCollider()
		{

		}

		bool CircleCollider::checkIntersection(Circle& aabb_circle1, Rectangle& aabb_box2, Resolution p_Resolution)
		{
			#undef max
			#undef min

			Vector2f deltaPosition;
			Vector2f boxMin;
			Vector2f penetration;

			boxMin.x = std::max(aabb_box2.getPosition().x - aabb_box2.getSize().x / 2, std::min(aabb_circle1.getPosition().x, aabb_box2.getPosition().x + aabb_box2.getSize().x / 2));
			boxMin.y = std::max(aabb_box2.getPosition().y - aabb_box2.getSize().y / 2, std::min(aabb_circle1.getPosition().y, aabb_box2.getPosition().y + aabb_box2.getSize().y / 2));

			deltaPosition.x = aabb_circle1.getPosition().x - boxMin.x;
			deltaPosition.y = aabb_circle1.getPosition().y - boxMin.y;

			if (deltaPosition.x * deltaPosition.x + deltaPosition.y * deltaPosition.y <=
				(aabb_circle1.getRadius()) * aabb_circle1.getRadius())
			{
				float dist = sqrt(deltaPosition.x * deltaPosition.x + deltaPosition.y * deltaPosition.y);

				float overlap = dist - aabb_circle1.getRadius();

				if (p_Resolution == Resolution::stop)
				{
					aabb_circle1.move
					(
						-overlap * (aabb_circle1.getPosition().x - boxMin.x) / dist,
						-overlap * (aabb_circle1.getPosition().y - boxMin.y) / dist
					);
				}
				else if (p_Resolution == Resolution::push)
				{
					aabb_box2.move
					(
						0.5 * overlap * (aabb_circle1.getPosition().x - boxMin.x) / dist,
						0.5 * overlap * (aabb_circle1.getPosition().y - boxMin.y) / dist
					);
					aabb_circle1.move
					(
						0.5 * -overlap * (aabb_circle1.getPosition().x - boxMin.x) / dist,
						0.5 * -overlap * (aabb_circle1.getPosition().y - boxMin.y) / dist
					);
				}
				return true;
			}
			return false;
		}
		bool CircleCollider::checkIntersection(Rectangle& aabb_box2, Circle& aabb_circle1, Resolution p_Resolution)
		{
			#undef max
			#undef min

			Vector2f deltaPosition;
			Vector2f boxMin;
			Vector2f penetration;

			boxMin.x = std::max(aabb_box2.getPosition().x - aabb_box2.getSize().x / 2, std::min(aabb_circle1.getPosition().x, aabb_box2.getPosition().x + aabb_box2.getSize().x / 2));
			boxMin.y = std::max(aabb_box2.getPosition().y - aabb_box2.getSize().y / 2, std::min(aabb_circle1.getPosition().y, aabb_box2.getPosition().y + aabb_box2.getSize().y / 2));

			deltaPosition.x = aabb_circle1.getPosition().x - boxMin.x;
			deltaPosition.y = aabb_circle1.getPosition().y - boxMin.y;

			if (deltaPosition.x * deltaPosition.x + deltaPosition.y * deltaPosition.y <=
				(aabb_circle1.getRadius()) * aabb_circle1.getRadius())
			{
				float dist = sqrt(deltaPosition.x * deltaPosition.x + deltaPosition.y * deltaPosition.y);

				float overlap = dist - aabb_circle1.getRadius();

				if (p_Resolution == Resolution::stop)
				{
					aabb_box2.move
					(
						overlap * (aabb_circle1.getPosition().x - boxMin.x) / dist,
						overlap * (aabb_circle1.getPosition().y - boxMin.y) / dist
					);
				}
				else if (p_Resolution == Resolution::push)
				{
					aabb_box2.move
					(
						0.5 * overlap * (aabb_circle1.getPosition().x - boxMin.x) / dist,
						0.5 * overlap * (aabb_circle1.getPosition().y - boxMin.y) / dist
					);
					aabb_circle1.move
					(
						0.5 * -overlap * (aabb_circle1.getPosition().x - boxMin.x) / dist,
						0.5 * -overlap * (aabb_circle1.getPosition().y - boxMin.y) / dist
					);
				}
				return true;
			}
			return false;
		}
		bool CircleCollider::checkIntersection(Circle& aabb_circle1, Rectangle& aabb_box2)
		{
			#undef max
			#undef min

			Vector2f deltaPosition;
			Vector2f boxMin;
			Vector2f penetration;

			boxMin.x = std::max(aabb_box2.getPosition().x - aabb_box2.getSize().x / 2, std::min(aabb_circle1.getPosition().x, aabb_box2.getPosition().x + aabb_box2.getSize().x / 2));
			boxMin.y = std::max(aabb_box2.getPosition().y - aabb_box2.getSize().y / 2, std::min(aabb_circle1.getPosition().y, aabb_box2.getPosition().y + aabb_box2.getSize().y / 2));

			deltaPosition.x = aabb_circle1.getPosition().x - boxMin.x;
			deltaPosition.y = aabb_circle1.getPosition().y - boxMin.y;

			if (deltaPosition.x * deltaPosition.x + deltaPosition.y * deltaPosition.y <
				(aabb_circle1.getRadius()) * aabb_circle1.getRadius())
				return true;
			return false;
		}
		bool CircleCollider::checkIntersection(Circle& aabb_circle1, Circle& aabb_circle2, Resolution p_Resolution)
		{
			// a^2 + b^2 = c^2

			Vector2f deltaPosition;
			Vector2f boxMin;
			Vector2f penetration;

			deltaPosition = Math::abs(Vector2f(aabb_circle1.getPosition().x - aabb_circle2.getPosition().x, aabb_circle1.getPosition().y - aabb_circle2.getPosition().y));

			if (deltaPosition.x * deltaPosition.x + deltaPosition.y * deltaPosition.y <=
				(aabb_circle1.getRadius() + aabb_circle2.getRadius()) * (aabb_circle1.getRadius() + aabb_circle2.getRadius()))
			{
				float dist = sqrt(deltaPosition.x * deltaPosition.x + deltaPosition.y * deltaPosition.y);

				float overlap = dist - aabb_circle1.getRadius() - aabb_circle2.getRadius();

				if (p_Resolution == Resolution::stop)
				{
					aabb_circle1.move
					(
						-overlap * (aabb_circle1.getPosition().x - aabb_circle2.getPosition().x) / dist,
						-overlap * (aabb_circle1.getPosition().y - aabb_circle2.getPosition().y) / dist
					);
				}
				else if (p_Resolution == Resolution::push)
				{
					aabb_circle1.move
					(
						0.5 * -overlap * (aabb_circle1.getPosition().x - aabb_circle2.getPosition().x) / dist,
						0.5 * -overlap * (aabb_circle1.getPosition().y - aabb_circle2.getPosition().y) / dist
					);
					aabb_circle2.move
					(
						0.5 * overlap * (aabb_circle1.getPosition().x - aabb_circle2.getPosition().x) / dist,
						0.5 * overlap * (aabb_circle1.getPosition().y - aabb_circle2.getPosition().y) / dist
					);
				}
				return true;
			}
			return false;
		}
		bool CircleCollider::checkIntersection(Circle& aabb_circle1, Circle& aabb_circle2)
		{
			// a^2 + b^2 = c^2

			Vector2f deltaPosition;
			Vector2f boxMin;
			Vector2f penetration;

			deltaPosition = Math::abs(Vector2f(aabb_circle1.getPosition().x - aabb_circle2.getPosition().x, aabb_circle1.getPosition().y - aabb_circle2.getPosition().y));

			if (deltaPosition.x * deltaPosition.x + deltaPosition.y * deltaPosition.y <
				(aabb_circle1.getRadius() + aabb_circle2.getRadius()) * (aabb_circle1.getRadius() + aabb_circle2.getRadius()))
				return true;
			return false;
		}

		bool CircleCollider::checkIntersection(Vector2f position, Circle& aabb_circle2)
		{
			Vector2f deltaPosition;

			deltaPosition = Vector2f(position.x - aabb_circle2.getPosition().x, position.y - aabb_circle2.getPosition().y);

			if (deltaPosition.x * deltaPosition.x + deltaPosition.y * deltaPosition.y < aabb_circle2.getRadius() * aabb_circle2.getRadius())
				return true;
			return false;
		}

		bool CircleCollider::isTouching(Rectangle& aabb_box1, Rectangle& aabb_box2)
		{
			return 0;
		}
		bool CircleCollider::isTouching(Vector2f position, Rectangle& aabb_box2)
		{
			return 0;
		}
	}
}
