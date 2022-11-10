#include "Math.h"

std::random_device Math::random;
std::mt19937 Math::gen = std::mt19937(random());

float Math::dot(Vector3f a, Vector3f b)
{
    return (a.x*b.x + a.y*b.y + a.z*b.z);
}
float Math::dot(Vector2f a, Vector2f b)
{
	return (a.x * b.x + a.y * b.y);
}

float Math::det(float a, float b, float c, float d)
{
    return a * d - b * c;
}

Vector3f Math::cross(Vector3f a, Vector3f b)
{
    return Vector3f(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

float Math::cross(const Vector2f a, const Vector2f b)
{
	return a.x * b.y - a.y * b.x;
}
Vector2f Math::cross(const Vector2f a, const float b)
{
	return Vector2f(b * a.y, -b * a.x);
}
Vector2f Math::cross(const float a, const Vector2f b)
{
	return Vector2f(-a * b.y, a * b.x);
}

Vector2f Math::rotate(const Vector2f p_Vector, const float p_Theta)
{
	if (p_Theta != 0)
	{
		Vector2f result;

		result.x = p_Vector.x * cos(p_Theta * DEG2RAD) - p_Vector.y * sin(p_Theta * DEG2RAD);
		result.y = p_Vector.x * sin(p_Theta * DEG2RAD) + p_Vector.y * cos(p_Theta * DEG2RAD);

		return result;
	}
	return p_Vector;
}

Vector3f Math::rotate(const Vector3f p_Vector, Vector3f p_Direction, const float p_Theta)
{
    float cos_theta = cos(p_Theta);
    float sin_theta = sin(p_Theta);

    return (p_Vector * cos_theta) + (cross(p_Direction, p_Vector) * sin_theta) + (p_Direction * dot(p_Direction, p_Vector)) * (1 - cos_theta);
}

double Math::sqrt(double num)
{
    double x0 = std::log(num);
    double x1 = (x0 - (((x0 * x0))/(2* x0)));
    x0 = x1;

    do{
        x1 = (x0 - (((x0 * x0) - num)/(2* x0)));
        x0 = x1;
    }
    while ((num / x1)+0.0001 < x0);

    return x1;
}

float Math::lerp(float a, float b, float f)
{
    return a + f * (b - a);
}
float Math::fade(const float t)
{
    return t*t*t*(t*(t-15) + 10);
}
float Math::grad(const float p)
{
	return 2.0f;
}

float Math::magnitude(Vector2f& v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}
float Math::magnitude(Vector3f& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
float Math::magnitude(Vector4f& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

Vector2f Math::normalize(Vector2f vec2) {
	const float EPSILON = 0.00001f;
	float xxyyzz = vec2.x * vec2.x + vec2.y * vec2.y;
	if (xxyyzz < EPSILON)
		return vec2;

	float invLength = sqrtf(xxyyzz);
	vec2.x = vec2.x / invLength;
	vec2.y = vec2.y / invLength;
	return vec2;
}

Vector3f Math::normalize(Vector3f vec3) {
    const float EPSILON = 0.00001f;
    float xxyyzz = vec3.x*vec3.x + vec3.y*vec3.y + vec3.z*vec3.z;
    if(xxyyzz < EPSILON)
        return vec3;

    float invLength = sqrtf(xxyyzz);
    vec3.x = vec3.x / invLength;
    vec3.y = vec3.y / invLength;
    vec3.z = vec3.z / invLength;
    return vec3;
}

Vector4f Math::normalize(Vector4f vec4) {
    const float EPSILON = 0.00001f;
    float xxyyzz = vec4.x*vec4.x + vec4.y*vec4.y + vec4.z*vec4.z + vec4.w * vec4.w;
    if(xxyyzz < EPSILON)
        return vec4;

    float invLength = sqrtf(xxyyzz);
    vec4.x = vec4.x / invLength;
    vec4.y = vec4.y / invLength;
    vec4.z = vec4.z / invLength;
    return vec4;
}

int Math::abs(const int value) {
	return value > 0 ? value : -value;
}
double Math::abs(const double value) {
	return value > 0.0 ? value : -value;
}
float Math::abs(const float value) {
	return value > 0.0f ? value : -value;
}
Vector2f Math::abs(Vector2f vector)
{
	return Vector2f(abs(vector.x), abs(vector.y));
}
Vector3f Math::abs(Vector3f vector)
{
	return Vector3f(abs(vector.x), abs(vector.y), abs(vector.z));
}
Vector4f Math::abs(Vector4f vector)
{
	return Vector4f(abs(vector.x), abs(vector.y), abs(vector.z), abs(vector.w));
}

int Math::getRandomNumberI(const int MIN, const int MAX) // Integer value
{   
    std::uniform_real_distribution<> dist(MIN, MAX);
    return (int)dist(gen);
}
int Math::getRandomNumberIND(const int MIN, const int MAX)// Integer Normal Distribution
{   
    std::normal_distribution<> dist(MIN, MAX);
    return (int)dist(gen);
}
double Math::getRandomNumber(const float MIN, const float MAX)
{   
    std::uniform_real_distribution<> dist(MIN, MAX);
    return dist(gen);
}
double Math::getRandomNumberND(const float MIN, const float MAX)// Normal Distribution
{   
    std::normal_distribution<> dist(MIN, MAX);
    return dist(gen);
}
float Math::perlinNoise1D(const int ARR_SIZE, float SEED, float BIAS, const int OCTAVES)
{
	return SEED;
}
float Math::perlinNoise2D(const Vector2i SIZE, float SEED, float BIAS, const int OCTAVES)
{
	return SEED;
}
float Math::perlinNoise3D(const Vector3i SIZE, float SEED, float BIAS, const int OCTAVES)
{
	return SEED;
}

