#pragma once

#include <iostream>
#include <utility>
#include <vector>

#include "../Rectangle.h"
#include "../VertexShape.h"
#include "AABBCollider.h"
#include "RigidBody.h"
#include "../Math/Math.h"
#include "../Math/Vector.h"

namespace vgl
{
	namespace Physics2D
	{
		struct Edge
		{
			Vector2f v0;
			Vector2f v;
			Vector2f max;
		};

		class SATCollider
		{
			public:
				SATCollider();
				~SATCollider();

				static bool checkIntersection(Rectangle& box1, Rectangle& box2, Resolution p_Resolution);
				static bool checkIntersection(RigidBody& box1, RigidBody& box2, Resolution p_Resolution);
				//static bool checkIntersection(RigidBody& aabb_box1, RigidBody& aabb_box2, Resolution p_Resolution);
				static bool checkIntersection(Vector2f position, Rectangle& aabb_box2);

				//bool checkIntersection(VertexShape& polygon1, VertexShape& polygon2);
				//bool checkIntersection(Vector2f position, VertexShape& polygon2);

				bool isTouching(Rectangle& aabb_box1, Rectangle& aabb_box2);
				bool isTouching(Vector2f position, Rectangle& aabb_box2);

				//bool isTouching(VertexShape& polygon1, VertexShape& polygon2);
				//bool isTouching(Vector2f position, VertexShape& polygon2);d

			protected:
			private:
				
				static Vector2f getSupportPoint(std::vector<Vector2f>& p_Vertices, Vector2f p_Direction);
				static Edge getBestEdge(std::vector<Vector2f>& p_Vertices, Vector2f p_Vertex, Vector2f p_Normal);
				static Edge getMostPerpendicularEdge(std::pair<Edge, Edge> p_ConnectedEdges, Vector2f p_Normal);
				static std::pair<Edge, Edge> getRIEdge(Edge p_Edge1, Edge p_Edge2, Vector2f p_Normal, bool& p_Flipped);
				static std::vector<Vector2f> clip(Vector2f v1, Vector2f v2, Vector2f p_Normal, double o);
				static std::vector<Vector2f> getContactPoints(Edge p_rEdge, Edge p_iEdge, Vector2f p_Normal, bool p_Flipped);

				Vector2f deltaPosition;
				Vector2f penetration;

				Rectangle* m_polygon1_ptr;

				Rectangle* m_polygon2_ptr;
		};
	}
}
