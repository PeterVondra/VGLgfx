#include "SATCollider.h"
#include <cmath>
#include <vector>

#undef min
#undef max

namespace vgl
{
	namespace Physics2D
	{
		SATCollider::SATCollider()
		{

		}
		SATCollider::~SATCollider()
		{

		}
		
		bool SATCollider::checkIntersection(Rectangle& box1, Rectangle& box2, Resolution p_Resolution)
		{
			Vector2f linebox1[4];
			linebox1[0] = box1.getPosition() + Math::rotate(box1.getSize() / 2, box1.getRotation());
			linebox1[1] = box1.getPosition() + Math::rotate(Vector2f(-box1.getSize().x / 2, box1.getSize().y / 2), box1.getRotation());
			linebox1[2] = box1.getPosition() + Math::rotate((box1.getSize() / 2) * -1, box1.getRotation());
			linebox1[3] = box1.getPosition() + Math::rotate(Vector2f(box1.getSize().x / 2, -box1.getSize().y / 2), box1.getRotation());

			Vector2f linebox2[4];
			linebox2[0] = box2.getPosition() + Math::rotate(box2.getSize() / 2, box2.getRotation());
			linebox2[2] = box2.getPosition() + Math::rotate(Vector2f(-box2.getSize().x / 2, box2.getSize().y / 2), box2.getRotation());
			linebox2[1] = box2.getPosition() + Math::rotate((box2.getSize() / 2) * -1, box2.getRotation());
			linebox2[3] = box2.getPosition() + Math::rotate(Vector2f(box2.getSize().x / 2, -box2.getSize().y / 2), box2.getRotation());

			float overlap = INFINITY;

			Vector2f currentAxis(0);

			for (int i = 0; i < 4; i++)
			{
				int b = (i + 1) % 4;

				Vector2f axisP = Math::normalize({ -(linebox1[b].y - linebox1[i].y), (linebox1[b].x - linebox1[i].x) });

				float min_r1 = INFINITY, max_r1 = -INFINITY;

				for (int j = 0; j < 4; j++)
				{
					float q = Math::dot(linebox1[j], axisP);
					min_r1 = std::min(min_r1, q);
					max_r1 = std::max(max_r1, q);
				}

				float min_r2 = INFINITY, max_r2 = -INFINITY;

				for (int j = 0; j < 4; j++)
				{
					float q = Math::dot(linebox2[j], axisP);
					min_r2 = std::min(min_r2, q);
					max_r2 = std::max(max_r2, q);
				}

				if (std::min(max_r1, max_r2) - std::max(min_r1, min_r2) < overlap)
				{
					overlap = std::min(max_r1, max_r2) - std::max(min_r1, min_r2);
					currentAxis = axisP;
				}

				if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
					return false;
			}

			linebox2[0] = box2.getPosition() + Math::rotate(box2.getSize() / 2, box2.getRotation());
			linebox2[1] = box2.getPosition() + Math::rotate(Vector2f(-box2.getSize().x / 2, box2.getSize().y / 2), box2.getRotation());
			linebox2[2] = box2.getPosition() + Math::rotate((box2.getSize() / 2) * -1, box2.getRotation());
			linebox2[3] = box2.getPosition() + Math::rotate(Vector2f(box2.getSize().x / 2, -box2.getSize().y / 2), box2.getRotation());

			for (int i = 0; i < 4; i++)
			{
				int b = (i + 1) % 4;
				Vector2f axisP = Math::normalize({ -(linebox2[b].y - linebox2[i].y), (linebox2[b].x - linebox2[i].x) });

				float min_r1 = INFINITY, max_r1 = -INFINITY;

				for (int j = 0; j < 4; j++)
				{
					float q = Math::dot(linebox2[j], axisP);
					min_r1 = std::min(min_r1, q);
					max_r1 = std::max(max_r1, q);
				}

				float min_r2 = INFINITY, max_r2 = -INFINITY;

				for (int j = 0; j < 4; j++)
				{
					float q = Math::dot(linebox1[j], axisP);
					min_r2 = std::min(min_r2, q);
					max_r2 = std::max(max_r2, q);
				}

				if (std::min(max_r1, max_r2) - std::max(min_r1, min_r2) < overlap)
				{
					overlap = std::min(max_r1, max_r2) - std::max(min_r1, min_r2);
					currentAxis = axisP;
				}

				if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
					return false;
			}

			if (p_Resolution == Resolution::none)
				return true;

			Vector2f d = Math::normalize(box2.getPosition() - box1.getPosition());
			if (Math::dot(currentAxis, d*-1) > 0)
			{
				box1.move(currentAxis * -overlap);
				return true;
			}

			box1.move(currentAxis * overlap);
			return true;
		}
		bool SATCollider::checkIntersection(RigidBody& box1, RigidBody& box2, Resolution p_Resolution)
		{
			std::vector<Vector2f> linebox1(4);
			linebox1[0] = box1.getPosition() + Math::rotate(box1.getSize() / 2, box1.getRotation());
			linebox1[1] = box1.getPosition() + Math::rotate(Vector2f(-box1.getSize().x / 2, box1.getSize().y / 2), box1.getRotation());
			linebox1[2] = box1.getPosition() + Math::rotate((box1.getSize() / 2) * -1, box1.getRotation());
			linebox1[3] = box1.getPosition() + Math::rotate(Vector2f(box1.getSize().x / 2, -box1.getSize().y / 2), box1.getRotation());

			std::vector<Vector2f> linebox2(4);
			linebox2[0] = box2.getPosition() + Math::rotate(box2.getSize() / 2, box2.getRotation());
			linebox2[1] = box2.getPosition() + Math::rotate(Vector2f(-box2.getSize().x / 2, box2.getSize().y / 2), box2.getRotation());
			linebox2[2] = box2.getPosition() + Math::rotate((box2.getSize() / 2) * -1, box2.getRotation());
			linebox2[3] = box2.getPosition() + Math::rotate(Vector2f(box2.getSize().x / 2, -box2.getSize().y / 2), box2.getRotation());			

			float overlap = INFINITY;

			Vector2f currentAxis(0);

			for (int i = 0; i < 4; i++)
			{
				int b = (i + 1) % 4;

				Vector2f axisP = Math::normalize({-(linebox1[b].y - linebox1[i].y), (linebox1[b].x - linebox1[i].x)});

				float min_r1 = INFINITY, max_r1 = -INFINITY;
				int min_r1_index = 0;
				int min_r2_index = 0;
				int max_r1_index = 0;
				int max_r2_index = 0;

				for (int j = 0; j < 4; j++)
				{
					float q = Math::dot(linebox1[j], axisP);
					float min = min_r1;
					float max = min_r1;
					min_r1 = std::min(min_r1, q);
					if(min_r1 < min)
						min_r1_index = j;
					max_r1 = std::max(max_r1, q);
					if(max_r1 < max)
						max_r1_index = j;
				}

				float min_r2 = INFINITY, max_r2 = -INFINITY;

				for (int j = 0; j < 4; j++)
				{
					float q = Math::dot(linebox2[j], axisP);
					min_r2 = std::min(min_r2, q);
					max_r2 = std::max(max_r2, q);
				}

				if (std::min(max_r1, max_r2) - std::max(min_r1, min_r2) < overlap)
				{
					overlap = std::min(max_r1, max_r2) - std::max(min_r1, min_r2);
					currentAxis = axisP;
				}

				if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
					return false;
			}

			for (int i = 0; i < 4; i++)
			{
				int b = (i + 1) % 4;
				Vector2f axisP = Math::normalize({ -(linebox2[b].y - linebox2[i].y), (linebox2[b].x - linebox2[i].x) });

				float min_r1 = INFINITY, max_r1 = -INFINITY;

				for (int j = 0; j < 4; j++)
				{
					float q = Math::dot(linebox2[j], axisP);
					min_r1 = std::min(min_r1, q);
					max_r1 = std::max(max_r1, q);
				}

				float min_r2 = INFINITY, max_r2 = -INFINITY;

				for (int j = 0; j < 4; j++)
				{
					float q = Math::dot(linebox1[j], axisP);

					min_r2 = std::min(min_r2, q);
					max_r2 = std::max(max_r2, q);
				}

				if (std::min(max_r1, max_r2) - std::max(min_r1, min_r2) < overlap)
				{
					overlap = std::min(max_r1, max_r2) - std::max(min_r1, min_r2);
					currentAxis = axisP;
				}

				if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
					return false;

			}

			if (p_Resolution == Resolution::none)
				return true;
			
			Vector2f d = box2.getPosition() - box1.getPosition();
			if (Math::dot(currentAxis, d) > 0)
				currentAxis *= -1;

			std::vector<Vector2f> contacts;

			const float percent = 1.0f;
			const float slop = 0.1f;
			Vector2f correction = currentAxis * percent * std::max(overlap - slop, 0.0f) / (box1.m_InvMass + box2.m_InvMass);	

			if(box1.m_Mass != 0)
				box1.move(correction * box1.m_InvMass);
			if(box2.m_Mass != 0)
				box2.move(correction * -box2.m_InvMass);

			linebox1[0] = box1.getPosition() + Math::rotate(box1.getSize() / 2, box1.getRotation());
			linebox1[1] = box1.getPosition() + Math::rotate(Vector2f(-box1.getSize().x / 2, box1.getSize().y / 2), box1.getRotation());
			linebox1[2] = box1.getPosition() + Math::rotate((box1.getSize() / 2) * -1, box1.getRotation());
			linebox1[3] = box1.getPosition() + Math::rotate(Vector2f(box1.getSize().x / 2, -box1.getSize().y / 2), box1.getRotation());


			linebox2[0] = box2.getPosition() + Math::rotate(box2.getSize() / 2, box2.getRotation());
			linebox2[1] = box2.getPosition() + Math::rotate(Vector2f(-box2.getSize().x / 2, box2.getSize().y / 2), box2.getRotation());
			linebox2[2] = box2.getPosition() + Math::rotate((box2.getSize() / 2) * -1, box2.getRotation());
			linebox2[3] = box2.getPosition() + Math::rotate(Vector2f(box2.getSize().x / 2, -box2.getSize().y / 2), box2.getRotation());			


			Edge e1 = getBestEdge(linebox1, getSupportPoint(linebox1, currentAxis * -1), currentAxis * -1);
			Edge e2 = getBestEdge(linebox2, getSupportPoint(linebox2, currentAxis), currentAxis);

			bool flip = false;
			std::pair<Edge, Edge> riedge = getRIEdge(e1, e2, currentAxis, flip);
			contacts = getContactPoints(riedge.first, riedge.second, currentAxis, flip);	

			for(auto& contact : contacts)
			{	
				//std::cout << contact << std::endl;
				//Vector2f contact(0);
				//std::cout << contacts.size() << ", " << contact << std::endl;
				//std::cout << "-------------------------" << std::endl;
				Vector2f r1 = contact - box1.getPosition(); 
				Vector2f r2 = contact - box2.getPosition(); 

				Vector2f rv = box2.m_Velocity + Math::cross(box2.m_AngularVelocity, r2) - box1.m_Velocity - Math::cross(box1.m_AngularVelocity, r1);
				float contactV = Math::dot(rv, currentAxis);

				if(contactV < 0)
					return false;

				float e = std::min(box1.m_Material.restitution, box2.m_Material.restitution);

				float mass_sum = box1.m_Mass + box2.m_Mass;
				
				float rACrossN = Math::cross(r1, currentAxis);
				float rBCrossN = Math::cross(r2, currentAxis);
				float j = -(1 + e) * contactV / (box1.m_InvMass + box2.m_InvMass + std::sqrt(Math::abs(rACrossN)) * box1.m_InvInertia + std::sqrt(Math::abs(rBCrossN)) * box2.m_InvInertia);	
				j /= contacts.size();

				Vector2f impulse = currentAxis * j;
	
				if(box1.m_Mass != 0)
				{
					box1.m_Velocity -= impulse * (box1.m_InvMass);
					box1.m_AngularVelocity -= Math::cross(r1, impulse) * box1.m_InvInertia;
				}
				if(box2.m_Mass != 0)
				{
					box2.m_Velocity += impulse * (box2.m_InvMass);
					box2.m_AngularVelocity += Math::cross(r2, impulse) * box2.m_InvInertia;
				}
			}

			if (box2.m_Mass == 0)
				box2.m_Velocity = 0;
			if (box1.m_Mass == 0)
				box1.m_Velocity = 0;
			
			return true;
		}
		Vector2f SATCollider::getSupportPoint(std::vector<Vector2f>& p_Vertices, Vector2f p_Direction)
		{
			float bestProjection = -INFINITY;
			Vector2f bestVertex;

			for(int i = 0; i < p_Vertices.size(); i++)
			{
				Vector2f v = p_Vertices[i];
				float projection = Math::dot(v, p_Direction);

				if(projection > bestProjection)
				{	
					bestVertex = v;
					bestProjection = projection;
				}
			}
			
			std::vector<Vector2f> rest;
			for(int i = 0; i < p_Vertices.size(); i++)
			{
				Vector2f v = p_Vertices[i];
				float projection = Math::dot(v, p_Direction);

				if(projection == bestProjection)
				{	
					bestVertex = v;
					rest.push_back(v);
					bestProjection = projection;
				}
			}

			for(auto& v : rest)
				if(v.x*v.x + v.y*v.y > bestVertex.x*bestVertex.x + bestVertex.y*bestVertex.y)
					bestVertex = v;

			return bestVertex;
		}


		Edge SATCollider::getBestEdge(std::vector<Vector2f>& p_Vertices, Vector2f p_Vertex, Vector2f p_Normal)
		{
			int prevIndex = 0;
			int nextIndex = 0;
			for(int i = 0; i < p_Vertices.size(); i++)
			{
				if(p_Vertices[i] == p_Vertex)
				{
					prevIndex = i - 1;
					nextIndex = i + 1;

					if(prevIndex < 0)
						prevIndex = p_Vertices.size() - 1;
					if(nextIndex > p_Vertices.size() - 1)
						nextIndex = 0;	
				}
			}

			std::pair<Edge, Edge> edges;
			edges.first.v = p_Vertex;
			edges.first.v0 = p_Vertices[prevIndex];
			edges.first.max = p_Vertex;
			edges.second.v0 = p_Vertex;
			edges.second.v = p_Vertices[nextIndex];
			edges.second.max = p_Vertex;

			Vector2f r = p_Vertex - p_Vertices[prevIndex];	
			Vector2f l = p_Vertex - p_Vertices[nextIndex];
			
			l = Math::normalize(l);
			r = Math::normalize(r);

			Edge bestEdge;

			if(Math::dot(r, p_Normal) <= Math::dot(l, p_Normal)){
				bestEdge = edges.first;
				return bestEdge;
			}
				bestEdge = edges.second;

			return bestEdge;
				
		}
		
		std::pair<Edge, Edge> SATCollider::getRIEdge(Edge p_Edge1, Edge p_Edge2, Vector2f p_Normal, bool& p_Flipped)
		{
			std::pair<Edge, Edge> ri;
			
			p_Flipped = false;
			if(std::abs(Math::dot(p_Edge1.v - p_Edge1.v0, p_Normal)) <= std::abs(Math::dot(p_Edge2.v - p_Edge2.v0, p_Normal)))
			{
				ri.first = p_Edge1;
				ri.second = p_Edge2;

				return ri;
			}	
			ri.first = p_Edge2;
			ri.second = p_Edge1;

			p_Flipped = true;

			return ri;
		}

		std::vector<Vector2f> SATCollider::clip(Vector2f v1, Vector2f v2, Vector2f p_Normal, double o)
		{
			std::vector<Vector2f> clipped;
			double d1 = Math::dot(p_Normal, v1) - o;
			double d2 = Math::dot(p_Normal, v2) - o;

			if(d1 >= 0.0f)
				clipped.push_back(v1);	
			if(d2 >= 0.0f)
				clipped.push_back(v2);

			if(d1 * d2 < 0.0f)
			{
				Vector2f e = v2 - v1;
				double u = d1 / (d1 - d2);
				e *= u;
				e += v1;

				clipped.push_back(e);
			}

			return clipped;
		}		
		
		std::vector<Vector2f> SATCollider::getContactPoints(Edge p_rEdge, Edge p_iEdge, Vector2f p_Normal, bool p_Flipped)
		{
			Vector2f refv = Math::normalize(p_rEdge.v - p_rEdge.v0);
			
			std::vector<Vector2f> contactPoints;
			double o1 = Math::dot(refv, p_rEdge.v0);
			std::vector<Vector2f> clippedp = clip(p_iEdge.v0, p_iEdge.v, refv, o1);
			
			if(clippedp.size() < 2)
				return contactPoints;
			
			double o2 = Math::dot(refv, p_rEdge.v);
			clippedp = clip(clippedp[0], clippedp[1], refv * -1, -o2);
			
			if(clippedp.size() < 2)
				return clippedp;			

			Vector2f refNorm = Math::cross(refv, -1.0f);

			if(p_Flipped)
				refNorm = Math::cross(refv, -1.0f);

			double max = Math::dot(refNorm, p_rEdge.max);
			
			if(Math::dot(refNorm, clippedp[0]) < max)
			{
				clippedp[0] = clippedp[1];				
				clippedp.resize(clippedp.size() - 1);
			}
			else if(Math::dot(refNorm, clippedp[1]) < max)
				clippedp.resize(clippedp.size() - 1);

			contactPoints.insert(contactPoints.end(), clippedp.begin(), clippedp.end());
			
			return contactPoints;	
		}
		/*bool SATCollider::checkIntersection(RigidBody& box1, RigidBody& box2, Resolution p_Resolution)
		{
			Vector2f linebox1[4];
			linebox1[0] = box1.getPosition() + Math::rotate(box1.getSize() / 2, box1.getRotation());
			linebox1[1] = box1.getPosition() + Math::rotate((box1.getSize() / 2) * -1, box1.getRotation());
			linebox1[2] = box1.getPosition() + Math::rotate(Vector2f(-box1.getSize().x / 2, box1.getSize().y / 2), box1.getRotation());
			linebox1[3] = box1.getPosition() + Math::rotate(Vector2f(box1.getSize().x / 2, -box1.getSize().y / 2), box1.getRotation());

			Vector2f linebox2[4];
			linebox2[0] = box2.getPosition() + Math::rotate(box2.getSize() / 2, box2.getRotation());
			linebox2[1] = box2.getPosition() + Math::rotate(Vector2f(-box2.getSize().x / 2, box2.getSize().y / 2), box2.getRotation());
			linebox2[2] = box2.getPosition() + Math::rotate((box2.getSize() / 2) * -1, box2.getRotation());
			linebox2[3] = box2.getPosition() + Math::rotate(Vector2f(box2.getSize().x / 2, -box2.getSize().y / 2), box2.getRotation());

			bool intersected = false;

			for (int i = 0; i < 4; i++)
			{
				Vector2f line_r1s = linebox1[0];
				Vector2f line_r1e = linebox1[i];

				Vector2f displacement(0);

				for (int j = 0; j < 4; j++)
				{
					Vector2f line_r2s = linebox2[j];
					Vector2f line_r2e = linebox2[(j + 1) % 4];

					float h = (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r1e.y) - (line_r1s.x - line_r1e.x) * (line_r2e.y - line_r2s.y);
					float t1 = ((line_r2s.y - line_r2e.y) * (line_r1s.x - line_r2s.x) + (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r2s.y)) / h;
					float t2 = ((line_r1s.y - line_r1e.y) * (line_r1s.x - line_r2s.x) + (line_r1e.x - line_r1s.x) * (line_r1s.y - line_r2s.y)) / h;

					if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f)
					{
						displacement.x += (1.0f - t1) * (line_r1e.x - line_r1s.x);
						displacement.y += (1.0f - t1) * (line_r1e.y - line_r1s.y);

						intersected = true;
					}
				}
				if (intersected)
					box1.m_Velocity.y = 0;

				box1.move(displacement * -1);//((int)p_Resolution == 0 ? -1 : 1));
			}

			return intersected;
		}*/
		bool SATCollider::checkIntersection(Vector2f position, Rectangle& aabb_box2)
		{
			Vector2f halfSize2 = aabb_box2.getSize() / 2;

			Vector2f temp_deltaPosition = aabb_box2.getPosition() - position;
			Vector2f temp_penetration = Math::abs(temp_deltaPosition) - halfSize2;

			if (temp_penetration.x < 0.0f && temp_penetration.y < 0.0f)
				return true;
			return false;
		}

		/*var Derivative = function(position, velocity){
    this.position = position ? position : new Vec(0, 0);
    this.velocity = velocity ? velocity : new Vec(0, 0);
    
    this.iadd = function(other){
        this.position.iadd(other.position);
        this.velocity.iadd(other.velocity);
        return this;
    }
    this.add = function(other){
        return new Derivative(
            this.position.add(other.position),
            this.velocity.add(other.velocity)
        )
    }

    this.mul = function(scalar){
        return new Derivative(
            this.position.mul(scalar),
            this.velocity.mul(scalar)
        )
    }
    this.imul = function(scalar){
        this.position.imul(scalar);
        this.velocity.imul(scalar);
        return this;
    }
}
var compute = function(center, initial, delta, derivative){
    var state = derivative.mul(delta).add(initial);

    return new Derivative(
        state.velocity,
        acceleration(center, state.position)
    );
}
var d0 = new Derivative();

new OneBody('rk4', {
    body: {
        position: new Vec(70, 60),
        velocity: new Vec(0, 1)
    },
    step: function(center, body){
        delta = 1;
        var d1 = compute(center, body, delta*0.0, d0);
        var d2 = compute(center, body, delta*0.5, d1);
        var d3 = compute(center, body, delta*0.5, d2);
        var d4 = compute(center, body, delta*1.0, d3);
        
        d2.iadd(d3).imul(2);
        d4.iadd(d1).iadd(d2).imul(1/6);
        
        body.position.iadd(d4.position.mul(delta));
        body.velocity.iadd(d4.velocity.mul(delta));
    }
});*/

		//bool checkIntersection(VertexShape& polygon1, VertexShape& polygon2)
		//{
		//	m_polygon1_ptr = &polygon1;
		//	m_polygon1_ptr = &polygon1;
		//
		//
		//}
		//bool checkIntersection(Vector2f position, VertexShape& polygon2)
		//{
		//
		//}

		//bool isTouching(VertexShape& polygon1, VertexShape& polygon2)
		//{
		//
		//}
		//bool isTouching(Vector2f position, VertexShape& polygon2)
		//{
		//
		//}

		bool SATCollider::isTouching(Rectangle& aabb_box1, Rectangle& aabb_box2)
		{
			m_polygon1_ptr = &aabb_box1;
			m_polygon2_ptr = &aabb_box2;

			Vector2f halfSize1 = m_polygon1_ptr->getSize() / 2;
			Vector2f halfSize2 = m_polygon2_ptr->getSize() / 2;

			deltaPosition = m_polygon2_ptr->getPosition() - m_polygon1_ptr->getPosition();
			penetration = Math::abs(deltaPosition) - (halfSize1 + halfSize2);

			if (penetration.x == 0.0f || penetration.y == 0.0f)
				return true;
			return false;
		}
		bool SATCollider::isTouching(Vector2f position, Rectangle& aabb_box2)
		{
			Vector2f halfSize2 = aabb_box2.getSize() / 2;

			Vector2f temp_deltaPosition = aabb_box2.getPosition() - position;
			Vector2f temp_penetration = Math::abs(temp_deltaPosition) - halfSize2;

			if (temp_penetration.x == 0.0f || temp_penetration.y == 0.0f)
				return true;
			return false;
		}
		//void SATCollider::onCollision(RESOLUTION::SAT_RESOLUTION resolution)
		//{
		//	if ((m_polygon1_ptr != NULL && m_polygon2_ptr != NULL) && resolution == RESOLUTION::stop)
		//	{
		//		if (penetration.x > penetration.y)
		//		{
		//			if (deltaPosition.x > 0.0f)
		//				m_polygon1_ptr->move(penetration.x, 0.0f);
		//			else
		//				m_polygon1_ptr->move(-penetration.x, 0.0f);
		//		}
		//		else
		//		{
		//			if (deltaPosition.y > 0.0f)
		//				m_polygon1_ptr->move(0.0f, penetration.y);
		//			else
		//				m_polygon1_ptr->move(0.0f, -penetration.y);
		//		}
		//	}
		//}

		/*
		bool intersected = false;

			Vector2f displacement(0);

			for (int i = 0; i < 4; i++)
			{
				Vector2f line_r1s = box1.getPosition();
				Vector2f line_r1e = linebox1[i];

				for (int j = 0; j < 4; j++)
				{
					Vector2f line_r2s = linebox2[j];
					Vector2f line_r2e = linebox2[(j + 1) % 4];

					float h = (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r1e.y) - (line_r1s.x - line_r1e.x) * (line_r2e.y - line_r2s.y);
					float t1 = ((line_r2s.y - line_r2e.y) * (line_r1s.x - line_r2s.x) + (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r2s.y)) / h;
					float t2 = ((line_r1s.y - line_r1e.y) * (line_r1s.x - line_r2s.x) + (line_r1e.x - line_r1s.x) * (line_r1s.y - line_r2s.y)) / h;

					if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f)
					{
						Vector2f t = (line_r1e - line_r1s) * (1.0f - t1);
						displacement.x -= t.x;
						displacement.y -= t.y;

						//std::cout << t << std::endl;

						intersected = true;
						box2.setColor({ 0, 0, 0 });
					}
				}
			}

			linebox2[0] = box2.getPosition() + Math::rotate(box2.getSize() / 2, box2.getRotation());
			linebox2[2] = box2.getPosition() + Math::rotate(Vector2f(-box2.getSize().x / 2, box2.getSize().y / 2), box2.getRotation());
			linebox2[1] = box2.getPosition() + Math::rotate((box2.getSize() / 2) * -1, box2.getRotation());
			linebox2[3] = box2.getPosition() + Math::rotate(Vector2f(box2.getSize().x / 2, -box2.getSize().y / 2), box2.getRotation());

			linebox1[0] = box1.getPosition() + Math::rotate(box1.getSize() / 2, box1.getRotation());
			linebox1[2] = box1.getPosition() + Math::rotate((box1.getSize() / 2) * -1, box1.getRotation());
			linebox1[1] = box1.getPosition() + Math::rotate(Vector2f(-box1.getSize().x / 2, box1.getSize().y / 2), box1.getRotation());
			linebox1[3] = box1.getPosition() + Math::rotate(Vector2f(box1.getSize().x / 2, -box1.getSize().y / 2), box1.getRotation());

			for (int i = 0; i < 4; i++)
			{
				Vector2f line_r1s = box2.getPosition();
				Vector2f line_r1e = linebox2[i];

				for (int j = 0; j < 4; j++)
				{
					Vector2f line_r2s = linebox1[j];
					Vector2f line_r2e = linebox1[(j + 1) % 4];

					float h = (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r1e.y) - (line_r1s.x - line_r1e.x) * (line_r2e.y - line_r2s.y);
					float t1 = ((line_r2s.y - line_r2e.y) * (line_r1s.x - line_r2s.x) + (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r2s.y)) / h;
					float t2 = ((line_r1s.y - line_r1e.y) * (line_r1s.x - line_r2s.x) + (line_r1e.x - line_r1s.x) * (line_r1s.y - line_r2s.y)) / h;

					if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f)
					{
						Vector2f t = (line_r1e - line_r1s) * (1.0f - t1);
						displacement.x += t.x;
						displacement.y += t.y;

						intersected = true;
						box2.setColor({ 0, 0, 0 });
					}
				}
			}

			box1.move(displacement);//((int)p_Resolution == 0 ? -1 : 1));

			if(!intersected)
				box2.setColor({ 0.8, 0, 0.1 });
		*/
	}
}
