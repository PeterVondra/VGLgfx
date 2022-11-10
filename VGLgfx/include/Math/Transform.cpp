#include "Transform.h"

/////////////////////////////////////////////////////////////////////
//Transform2D
/////////////////////////////////////////////////////////////////////

Vector2f Transform2D::getPosition()
{
	return position;
}
Vector2f Transform2D::getScalar()
{
	return scalar;
}
float Transform2D::getRotation()
{
	return rotation;
}

void Transform2D::rotate(const float ROTATION_DEG)
{
	rotation += ROTATION_DEG;
	
	Vector2f pos = position;
	setPosition(0, 0);

	model.rotateZ(ROTATION_DEG);
	setPosition(pos - m_Origin);
}

void Transform2D::move(const Vector2f VEC)
{
	position += VEC;
	model.translate(VEC.x, VEC.y, 0.0f);
}
void Transform2D::move(const float X, const float Y)
{
	position += Vector2f(X, Y);
	model.translate(X, Y, 0.0f);
}

void Transform2D::scale(const Vector2f SCALAR)
{
	scalar *= SCALAR;
	model.scale(SCALAR.x, SCALAR.y, 1.0f);
}
void Transform2D::scale(const float SCALAR_X, const float SCALAR_Y)
{
	scalar *= Vector2f(SCALAR_X, SCALAR_Y);
	model.scale(SCALAR_X, SCALAR_Y, 1.0f);
}

void Transform2D::setRotation(const float ROTATION_DEG)
{
	rotation = ROTATION_DEG;
	Vector2f pos = position;
	setPosition(0.0f, 0.0f);
	model.translate(m_Origin.x, m_Origin.y, 0.0f);
	model.rotateZ(ROTATION_DEG);
	setPosition(pos);
}

void Transform2D::setScale(const Vector2f SCALAR)
{
	scalar = SCALAR;
	model.identity();
	model.scale(scalar.x, scalar.y, 1);
	model.rotate(rotation, 0, 1, 0);
	model.translate(position.x, position.y, 0);
}
void Transform2D::setScale(const float SCALAR_X, const float SCALAR_Y)
{
	scalar = { SCALAR_X, SCALAR_Y };
	model.identity();
	model.scale(scalar.x, scalar.y, 1);
	model.rotate(rotation, 0, 1, 0);
	model.translate(position.x, position.y, 0);
}

void Transform2D::setPosition(const Vector2f POSITION)
{
	model.identity();
	position = POSITION;
	model.scale(scalar.x, scalar.y, 1.0f);
	model.rotate(rotation, 0, 0, 1);
	model.translate(position.x, position.y, 0.0f);
}
void Transform2D::setPosition(const float POS_X, const float POS_Y)
{
	model.identity();
	position = Vector2f(POS_X, POS_Y);
	model.scale(scalar.x, scalar.y, 1.0f);
	model.rotate(rotation, 0, 0, 1);
	model.translate(position.x, position.y, 0.0f);
}

void Transform2D::reset()
{
	model.identity();
}


/////////////////////////////////////////////////////////////////////
//Transform3D
/////////////////////////////////////////////////////////////////////

Vector3f& Transform3D::getPosition()
{
	return position;
}
Vector3f& Transform3D::getRotation()
{
	return rotation;
}
Vector3f& Transform3D::getScalar()
{
	return scalar;
}

void Transform3D::rotate(const float ROT_DEG, const Vector3f ROTATION_DEG)
{
	rotation += ROTATION_DEG * ROT_DEG;
	model.rotate(ROT_DEG, ROTATION_DEG);
}
void Transform3D::rotate(const float ROT_DEG, const float ROT_X, const float ROT_Y, const float ROT_Z)
{
	rotation += Vector3f(ROT_X, ROT_Y, ROT_Z) * ROT_DEG;
	model.rotate(ROT_DEG, Vector3f(ROT_X, ROT_Y, ROT_Z));
}

void Transform3D::move(const Vector3f VEC)
{
	position += VEC;
	model.translate(VEC);
}
void Transform3D::move(const float X, const float Y, const float Z)
{
	position += Vector3f(X, Y, Z);
	model.translate(Vector3f(X, Y, Z));
}

void Transform3D::scale(const Vector3f SCALAR)
{
	model.translate(position * -1);
	scalar *= SCALAR;
	model.scale(SCALAR);
	model.translate(position);
}
void Transform3D::scale(const float SCALAR_X, const float SCALAR_Y, const float SCALAR_Z)
{
	model.translate(position * -1);
	scalar *= Vector3f(SCALAR_X, SCALAR_Y, SCALAR_Z);
	model.scale(Vector3f(SCALAR_X, SCALAR_Y, SCALAR_Z));
	model.translate(position);
}

void Transform3D::setRotation(const float ROT_DEG, const Vector3f ROTATION_DEG)
{
	rotation = ROTATION_DEG * ROT_DEG;
	model.rotate(ROT_DEG, rotation);
}
void Transform3D::setRotation(const float ROT_DEG, const float ROT_X, const float ROT_Y, const float ROT_Z)
{
	rotation = Vector3f(ROT_X, ROT_Y, ROT_Z) * ROT_DEG;
	model.rotate(ROT_DEG, rotation);
}

void Transform3D::setScale(const Vector3f SCALAR)
{
	scalar = SCALAR;
	model.identity();
	model.scale(scalar);
	model.rotate(rotation.x, 1, 0, 0);
	model.rotate(rotation.y, 0, 1, 0);
	model.rotate(rotation.z, 0, 0, 1);
	model.translate(position);
}
void Transform3D::setScale(const float SCALAR_X, const float SCALAR_Y, const float SCALAR_Z)
{
	scalar = Vector3f(SCALAR_X, SCALAR_Y, SCALAR_Z);
	model.identity();
	model.scale(scalar);
	model.rotate(rotation.x, 1, 0, 0);
	model.rotate(rotation.y, 0, 1, 0);
	model.rotate(rotation.z, 0, 0, 1);
	model.translate(position);
}

void Transform3D::setPosition(const Vector3f POSITION)
{
	model.identity();
	position = POSITION;
	model.scale(scalar);
	model.rotate(rotation.x, 1, 0, 0);
	model.rotate(rotation.y, 0, 1, 0);
	model.rotate(rotation.z, 0, 0, 1);
	model.translate(position);
}
void Transform3D::setPosition(const float POS_X, const float POS_Y, const float POS_Z)
{
	position = Vector3f(POS_X, POS_Y, POS_Z);
	model.identity();
	model.scale(scalar);
	model.rotate(rotation.x, 1, 0, 0);
	model.rotate(rotation.y, 0, 1, 0);
	model.rotate(rotation.z, 0, 0, 1);
	model.translate(position);
}

void Transform3D::reset()
{
	model.identity();
}
