#include "RigidBody.h"

namespace vgl
{
	namespace Physics2D
	{
		//Rock       Density : 0.6  Restitution : 0.1
		//Wood       Density : 0.3  Restitution : 0.2
		//Metal      Density : 1.2  Restitution : 0.05
		//BouncyBall Density : 0.3  Restitution : 0.8
		//SuperBall  Density : 0.3  Restitution : 0.95
		//Pillow     Density : 0.1  Restitution : 0.2
		//Static     Density : 0.0  Restitution : 0.4
		Material Materials::Rock	= Material(0.6, 0.1);
		Material Materials::Wood	= Material(0.3, 0.2);
		Material Materials::Metal	= Material(1.2, 0.05);
		Material Materials::Bouncy	= Material(0.3, 0.8);
		Material Materials::Super	= Material(0.3, 0.95);
		Material Materials::Pillow	= Material(0.1, 0.2);
		Material Materials::Static	= Material(0.0, 0.4);

		RigidBody::RigidBody()
		{

		}
		RigidBody::RigidBody(Vector2f p_Size, Vector2f p_Position, Vector3f p_Color, Material p_Material)
			: Rectangle::Rectangle(p_Size, p_Position, p_Color), m_Force(0, 0), m_GravityConst(0), m_Velocity({0, 0}), m_AngularVelocity(0), m_DeltaTime(0), m_Acceleration(0), m_Deceleration(0)
		{
			m_Mass = p_Material.density * (p_Size.x * p_Size.y);
			m_InvInertia = 0;


			if (m_Mass != 0)
			{
				if (p_Material.restitution != 0)
				{
					m_StaticFriction = p_Material.restitution;
					m_DynamicFriction = p_Material.restitution;
				}
				else
				{
					m_StaticFriction = m_Mass;
					m_DynamicFriction = 0.1;
				}
				
				m_InvInertia = 1 / (m_Mass * p_Size.x * p_Size.y);
				m_InvMass = 1 / m_Mass;
			}
			else
			{
				m_StaticFriction = 1;
				m_DynamicFriction = 0.1;
				m_InvMass = 0;
			}

			std::cout << m_Mass << std::endl;
			std::cout << m_Velocity << std::endl;
			std::cout << "Static Friction: " << m_StaticFriction << " Dynamic Friction: " << m_DynamicFriction << std::endl;
	
			m_Material = p_Material;

		}
		RigidBody::~RigidBody()
		{

		}

		void RigidBody::setDeltaTime(const float p_DeltaTime)
		{
			m_DeltaTime = p_DeltaTime;
		}

		void RigidBody::setGravityConst(const float p_GravityConst)
		{
			m_GravityConst = p_GravityConst;
		}

		void RigidBody::push(Vector2f p_Force)
		{
			m_Force = p_Force;
		}
		void RigidBody::decelerate()
		{
		}
		void RigidBody::setPosition(Vector2f p_Position)
		{
			Rectangle::setPosition(p_Position);
		}

		void RigidBody::update()
		{
			if (m_DeltaTime < 1)
			{
				if (!m_Mass <= 0)
				{
					Vector2f gravity(0, -m_GravityConst);

					m_Acceleration = gravity;// + m_Force / m_InvMass;
					m_Velocity += m_Acceleration * m_DeltaTime;

					m_AngularVelocity += m_Torque * m_InvInertia * m_DeltaTime;
					m_Orientation += m_AngularVelocity * m_DeltaTime;

					setPosition(getPosition() + m_Velocity * m_DeltaTime);
					
					rotate(m_AngularVelocity * m_DeltaTime * Math::RAD2DEG);

					m_Force = { 0, 1 };
				}
				else
					m_Velocity = 0;
			}
		}
		
		float RigidBody::RK4(Vector2f v0, float x, float h)
		{
			int n =  (int)((x - v0.x)/h);
			float k1, k2, k3, k4;
			
			float y = v0.y;
			for(int i = 1; i <= n; i++)
			{
				k1 = h*((v0.x - y)/2);
				k2 = h*((v0.x + h/2) - (y + k1/2))/2;
				k3 = h*((v0.x + h/2) - (y + k2/2))/2;
				k4 = h*((v0.x + h) - (y + k3))/2;
				
				y = y + (1/6)*(k1 + 2*k2 + 2*k3 + k4);

				v0.x += h;
			}	
			return y;
		}

		Material RigidBody::getMaterial()
		{
			return m_Material;
		}
	}
}
