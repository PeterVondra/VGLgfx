#pragma once

#include <iostream>
#include <vector>
#include <limits.h>
#include <cmath>
#include <random>

#include "Vector.h"

class Math
{   
    public:
		Math();

        static float dot(Vector3f a, Vector3f b);
        static float dot(Vector2f a, Vector2f b);
        static float det(float a, float b, float c, float d);
        static Vector3f cross(Vector3f a, Vector3f b);
        static float cross(const Vector2f a, const Vector2f b);
        static Vector2f cross(const Vector2f a, const float b);
        static Vector2f cross(const float a, const Vector2f b);
        static Vector2f rotate(const Vector2f p_Vector, const float p_Degrees);
        static Vector3f rotate(const Vector3f p_Vector, Vector3f p_Direction, const float p_Degrees);

        static double sqrt(double num);
        static float lerp(float a, float b, float f);
        static float fade(const float t);
        static float grad(const float p);
	static float magnitude(Vector2f& v);
        static float magnitude(Vector3f& v);
        static float magnitude(Vector4f& v);

        static Vector2f normalize(Vector2f vec2);
        static Vector3f normalize(Vector3f vec3);
        static Vector4f normalize(Vector4f vec4);

		static int abs(const int value);
		static double abs(const double value);
		static float abs(const float value);
		static Vector2f abs(Vector2f vector);
		static Vector3f abs(Vector3f vector);
		static Vector4f abs(Vector4f vector);

        static int    getRandomNumberI(const int MIN, const int MAX); // Integer value
        static int    getRandomNumberIND(const int MIN, const int MAX); // Integer Normal Distribution
        static double getRandomNumber(const float MIN, const float MAX);
        static double getRandomNumberND(const float MIN, const float MAX); // Normal Distribution
        static float perlinNoise1D(const int  ARR_SIZE, float SEED, float BIAS, const int OCTAVES);
        static float perlinNoise2D(const Vector2i SIZE, float SEED, float BIAS, const int OCTAVES);
        static float perlinNoise3D(const Vector3i SIZE, float SEED, float BIAS, const int OCTAVES);

        static constexpr float PI = 3.141593f;
        static constexpr float DEG2RAD = 3.141593f / 180.0f;
        static constexpr float RAD2DEG = 180.0f / 3.141593f;
        static constexpr float EPSILON = 0.00001f;

    protected:
    private:

        static std::random_device random;
        static std::mt19937 gen;
};
