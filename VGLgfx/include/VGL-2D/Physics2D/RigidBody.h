#pragma once

#include "../Rectangle.h"

namespace vgl
{
	namespace Physics2D
	{
		struct Material
		{
			Material() : density(0), restitution(0) {};
			Material(float p_Density, float p_Restituiton) : density(p_Density), restitution(p_Restituiton) {};
			float density;
			float restitution;
		};

		//Rock       Density : 0.6  Restitution : 0.1
		//Wood       Density : 0.3  Restitution : 0.2
		//Metal      Density : 1.2  Restitution : 0.05
		//BouncyBall Density : 0.3  Restitution : 0.8
		//SuperBall  Density : 0.3  Restitution : 0.95
		//Pillow     Density : 0.1  Restitution : 0.2
		//Static     Density : 0.0  Restitution : 0.4
		class Materials
		{
			public:
				static Material Rock;
				static Material Wood;
				static Material Metal;
				static Material Bouncy;
				static Material Super;
				static Material Pillow;
				static Material Static;
		};

		class RigidBody : public Rectangle
		{
			public:
				RigidBody();
				RigidBody(Vector2f p_Size, Vector2f p_Position, Vector3f p_Color, Material p_Material);
				~RigidBody();

				void setDeltaTime(const float p_DeltaTime);
				void setGravityConst(const float p_GravityConst);
				void setPosition(Vector2f p_Position) override;
				void setMass(const float p_Mass) { m_Mass = p_Mass; m_InvMass = 1/p_Mass; }				

				Material getMaterial();

				void push(Vector2f p_Force);
				void decelerate();

				void update();

				Vector2f m_Velocity;

			protected:
			private:
				friend class SATCollider;

				float RK4(Vector2f v0, float x, float h);

				float m_Mass;
				float m_InvMass;
				float m_Inertia;
				float m_InvInertia;
				float m_Orientation;
				float m_Torque;
				float m_AngularVelocity;
				float m_DeltaTime;
				float m_GravityConst;
				float m_StaticFriction;
				float m_DynamicFriction;

				Vector2f m_Force;
				Vector2f m_Acceleration;
				Vector2f m_Deceleration;

				Material m_Material;
		};
	}
}
