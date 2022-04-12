#include <iomanip>
#include "Vector.h"


/////////////////////////////////////////////////////////////////////////////////
//Vector2D
/////////////////////////////////////////////////////////////////////////////////
Vector2f::Vector2f()
{
    //constructor
}

void Vector2f::set(float x, float y)
{
    this->x = x;
    this->y = y;
}

Vector2f Vector2f::operator= (const Vector2f vec)
{
	x = vec.x;
	y = vec.y;
    return *this;
}

Vector2f Vector2f::operator+ (const Vector2f vec)const
{
    return Vector2f(x + vec.x, y + vec.y);
}

Vector2f Vector2f::operator- (const Vector2f vec)const
{
    return Vector2f(x - vec.x, y - vec.y);
}

Vector2f Vector2f::operator+=(const Vector2f vec)
{
	*this = *this + vec;
	return *this;
}

Vector2f Vector2f::operator-=(const Vector2f vec)
{
	*this = *this - vec;
	return *this;
}

Vector2f Vector2f::operator/ (const Vector2f vec)const
{
    return Vector2f(x / vec.x, y / vec.y);
}

Vector2f Vector2f::operator/ (const float& num)const
{
    return Vector2f(x / num, y / num);
}

Vector2f Vector2f::operator* (const Vector2f vec)const
{
    return Vector2f(x * vec.x, y * vec.y);
}

Vector2f Vector2f::operator*=(const Vector2f vec)
{
	*this = *this * vec;
	return *this;
}

Vector2f Vector2f::operator* (const float num)const
{
    return Vector2f(x * num, y * num);
}

Vector2f Vector2f::operator*=(const float num)
{
	*this = *this * num;
	return *this;
}

bool Vector2f::operator!= (const Vector2f vec)
{
    if(x != vec.x || y != vec.y)
        return true;
    else
        return false;
}

std::ostream& operator<< (std::ostream& output, const Vector2f& vec)
{
    return output << std::fixed << std::setprecision(5) << "[" << vec.x << ", " << vec.y << "]";
}

//Vector2 int32_t
Vector2i::Vector2i()
{
    //constructor
}

void Vector2i::set(int32_t x, int32_t y)
{
    this->x = x;
    this->y = y;
}

Vector2i Vector2i::operator= (const Vector2i vec)
{
    return Vector2i(x = vec.x, y = vec.y);
}

Vector2i Vector2i::operator+ (const Vector2i vec)const
{
    return Vector2i(x + vec.x, y + vec.y);
}

Vector2i Vector2i::operator- (const Vector2i vec)const
{
    return Vector2i(x - vec.x, y - vec.y);
}

Vector2i Vector2i::operator/ (const Vector2i vec)const
{
    return Vector2i(x / vec.x, y / vec.y);
}

Vector2i Vector2i::operator/ (const int32_t& num)const
{
    return Vector2i(x / num, y / num);
}

Vector2i Vector2i::operator* (const Vector2i vec)const
{
    return Vector2i(x * vec.x, y * vec.y);
}

Vector2i Vector2i::operator* (const int32_t& num)const
{
    return Vector2i(x * num, y * num);
}

bool Vector2i::operator== (const Vector2i& vec)
{
    if(x == vec.x && y == vec.y)
        return true;
    else
        return false;
}
bool Vector2i::operator!= (const Vector2i& vec)
{
    if(x != vec.x || y != vec.y)
        return true;
    else
        return false;
}

std::ostream& operator<< (std::ostream& output, const Vector2i& vec)
{
    return output << "[" << vec.x << ", " << vec.y << "]";
}



/////////////////////////////////////////////////////////////////////////////////
//Vector3f
/////////////////////////////////////////////////////////////////////////////////
Vector3f::Vector3f()
{
    //constructor
}

void Vector3f::set(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

Vector3f& Vector3f::operator= (const Vector3f& vec)
{
    x = vec.x;
    y = vec.y;
    z = vec.z;
    return *this;
}

Vector3f Vector3f::operator+ (const Vector3f vec)const
{
    return Vector3f(x + vec.x, y + vec.y, z + vec.z);
}

Vector3f Vector3f::operator- (const Vector3f vec)const
{
    return Vector3f(x - vec.x, y - vec.y, z - vec.z);
}

Vector3f Vector3f::operator/ (const Vector3f vec)const
{
    return Vector3f(x / vec.x, y / vec.y, z / vec.z);
}

Vector3f Vector3f::operator/ (const float& num)const
{
    return Vector3f(x / num, y / num, z / num);
}

Vector3f Vector3f::operator* (const Vector3f vec)const
{
    return Vector3f(x * vec.x, y * vec.y, z * vec.z);
}

Vector3f Vector3f::operator* (const float& num)const
{
    return Vector3f(x * num, y * num, z * num);
}
void Vector3f::operator*= (const Vector3f vec)
{
    *this = *this * vec;
}
bool Vector3f::operator!= (const Vector3f vec)
{
    return x != vec.x || y != vec.y || z != vec.z;
}
bool Vector3f::operator== (const Vector3f vec)
{
    return x == vec.x && y == vec.y && z == vec.z;
}

std::ostream& operator<< (std::ostream& output, const Vector3f& vec)
{
    return output << std::fixed << std::setprecision(5) << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
}

Vector3f Vector3f::operator+= (const Vector3f& vec)
{
    x += vec.x;
    y += vec.y;
    z += vec.z;
    return *this;
}

Vector3f Vector3f::operator-= (const Vector3f& vec)
{
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    return *this;
}

float& Vector3f::operator[] (const uint32_t index)
{ 
    switch(index)
    {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        std::cout << "index is too big!!!" << std::endl;
    }
}

/////////////////////////////////////////////////////////////////////////////////
//Vector4f
/////////////////////////////////////////////////////////////////////////////////
void Vector4f::set(float x, float y, float z, float w)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

Vector4f Vector4f::operator= (const Vector4f& vec)
{
    return Vector4f(x = vec.x, y = vec.y, z = vec.z, w = vec.w);
}

Vector4f Vector4f::operator+ (const Vector4f& vec)const
{
    return Vector4f(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
}

Vector4f Vector4f::operator- (const Vector4f& vec)const
{
    return Vector4f(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
}

Vector4f Vector4f::operator/ (const Vector4f& vec)const
{
    return Vector4f(x / vec.x, y / vec.y, z / vec.z, w / vec.w);
}

Vector4f Vector4f::operator/ (const float& num)const
{
    return Vector4f(x / num, y / num, z / num, w / num);
}

Vector4f Vector4f::operator* (const Vector4f& vec)const
{
    return Vector4f(x * vec.x, y * vec.y, z * vec.z, w * vec.w);
}

Vector4f Vector4f::operator* (const float& num)const
{
    return Vector4f(x * num, y * num, z * num, w * num);
}

bool Vector4f::operator!= (const Vector4f& vec)
{
    if(x != vec.x || y != vec.y || z != vec.z || w != vec.w)
        return true;
    else
        return false;
}

std::ostream& operator<< (std::ostream& output, const Vector4f& vec)
{
    return output << std::fixed << std::setprecision(5) << "[" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << "]";
}
