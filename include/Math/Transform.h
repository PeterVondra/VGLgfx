#pragma once

#include<iostream>

#include "Math.h"
#include "Matrix.h"
#include "Vector.h"

namespace vgl{ class Scene; }

class Transform2D
{
	public:

		Transform2D() : position({ 0, 0 }), scalar(1), rotation(0), m_Origin({ 0, 0 }) {};
		Transform2D(const Transform2D& p_T2D) : position(0), rotation(0), scalar(0) { *this = p_T2D; };
		
		Vector2f getPosition();
		Vector2f getScalar();
		float getRotation();

		//Degrees
		void rotate(const float p_Degrees);

		void move(const Vector2f p_Vector);
		void move(const float p_Horizontal, const float p_Vertical);

		void scale(const Vector2f p_Scalar);
		void scale(const float p_ScalarX, const float p_ScalarY);

		void setRotation(const float p_Degrees);

		//Scale from 1
		void setScale(const Vector2f p_Scalar);
		void setScale(const float p_ScalarX, const float p_ScalarY);

		virtual void setPosition(const Vector2f p_Position);
		virtual void setPosition(const float p_PositionX, const float p_PositionY);

		//Set all configurations to default
		void reset();

		Matrix4f model = Matrix4f::getIdentity();

	protected:
    friend class vgl::Scene;

		Vector2f position;
		Vector2f scalar;
		float rotation;

		Vector2f m_Origin;
};

class Transform3D
{
	public:
		Transform3D() : position(0), rotation(0), scalar(1) {};

		virtual Vector3f& getPosition();
		Vector3f& getRotation();
		Vector3f& getScalar();

		void rotate(const float ROT_DEG, const Vector3f ROTATION_DEG);
		void rotate(const float ROT_DEG, const float ROT_X, const float ROT_Y, const float ROT_Z);

		virtual void move(const Vector3f VEC);
		virtual void move(const float X, const float Y, const float Z);

		void scale(const Vector3f SCALAR);
		void scale(const float SCALAR_X, const float SCALAR_Y, const float SCALAR_Z);

		void setRotation(const float ROT_DEG, const Vector3f ROTATION_DEG);
		void setRotation(const float ROT_DEG, const float ROT_X, const float ROT_Y, const float ROT_Z);

		//Scale from 0
		void setScale(const Vector3f SCALAR);
		void setScale(const float SCALAR_X, const float SCALAR_Y, const float SCALAR_Z);

		void setPosition(const Vector3f POSITION);
		void setPosition(const float POS_X, const float POS_Y, const float POS_Z);

		//Set all configurations to default
		void reset();

		Matrix4f model;
	protected:
    friend class vgl::Scene;

		Vector3f position = 0;
		Vector3f rotation = 0;
		Vector3f scalar = 1;

};
