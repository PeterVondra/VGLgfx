#pragma once

#include <iostream>
#include <limits.h>
#include <cmath>
#include <vector>

/////////////////////////////////////////////////////////////////////////////////
//Vector2D
/////////////////////////////////////////////////////////////////////////////////
class Vector2f
{
    public:
        union {
            struct {
                float x;
                float y;
            };
            struct {
                float r;
                float g;
            };
            struct {
                float s;
                float t;
            };
        };

        Vector2f();

        Vector2f(float x, float y) : x(x), y(y) {};
        Vector2f(float xy) : x(xy), y(xy) {};

        void set(float x, float y);
        float* get() { return &x; };

        Vector2f operator= (const Vector2f vec);
        Vector2f operator+ (const Vector2f vec)const;
        Vector2f operator- (const Vector2f vec)const;
        Vector2f operator+= (const Vector2f vec);
        Vector2f operator-= (const Vector2f vec);
        Vector2f operator/ (const Vector2f vec)const;
        Vector2f operator/ (const float& num)const;
        Vector2f operator* (const Vector2f vec)const;
        Vector2f operator*= (const Vector2f vec);
        Vector2f operator* (const float num)const;
        Vector2f operator*= (const float num);
        bool operator!= (const Vector2f vec);
		bool operator==(const Vector2f vec) { return vec.x == x && vec.y == y;  }

    protected:
    private:
};

std::ostream& operator<< (std::ostream& output, const Vector2f& vec);

class Vector2i
{
    public:
        union {
            struct {
                int32_t x;
                int32_t y;
            };
            struct {
                int32_t r;
                int32_t g;
            };
            struct {
                int32_t s;
                int32_t t;
            };
        };

        Vector2i();

        Vector2i(int32_t x, int32_t y) : x(x), y(y) {};
        Vector2i(int32_t xy) : x(xy), y(xy) {};

        void set(int32_t x, int32_t y);
        int32_t* get() { return &x; };
		Vector2f getVec2f() { return Vector2f(x, y); }

        Vector2i operator= (const Vector2i vec);
        Vector2i operator+ (const Vector2i vec)const;
        Vector2i operator- (const Vector2i vec)const;
        Vector2i operator/ (const Vector2i vec)const;
        Vector2i operator/ (const int32_t& num)const;
        Vector2i operator* (const Vector2i vec)const;
        Vector2i operator* (const int32_t& num)const;
        bool operator== (const Vector2i& vec);
        bool operator!= (const Vector2i& vec);


    protected:
    private:
};

std::ostream& operator<< (std::ostream& output, const Vector2i& vec);

/////////////////////////////////////////////////////////////////////////////////
//Vector3D
/////////////////////////////////////////////////////////////////////////////////
class Vector3f
{
    public:
        union {
            struct {
                float x;
                float y;
                float z;
            };
            struct {
                float r;
                float g;
                float b;
            };
            struct {
                Vector2f xy;
                float z;
            };
            struct {
                Vector2f rg;
                float b;
            };
        };

        Vector3f();
        Vector3f(float xyz) : x(xyz), y(xyz), z(xyz) {};

        Vector3f(float x, float y, float z) : x(x), y(y), z(z) {};
        Vector3f normal() { return normalize(*this); }
        Vector3f    normalize(Vector3f vec3);
        bool        equal(const Vector3f& vec, float e) const;

        float length() { return std::sqrt(x * x + y * y + z * z); }
        void set(float x, float y, float z);
        float* get() { return &x; };

		Vector2f getVector2f() { return Vector2f(x, y); }

        Vector3f& operator= (const Vector3f& vec);
        Vector3f operator+ (const Vector3f vec)const;
        Vector3f operator- (const Vector3f vec)const;
        Vector3f operator/ (const Vector3f vec)const;
        Vector3f operator/ (const float& num)const;
        Vector3f operator* (const Vector3f vec)const;
        Vector3f operator* (const float& num)const;
        void operator*= (const Vector3f vec);
        Vector3f operator+= (const Vector3f& vec);
        Vector3f operator-= (const Vector3f& vec);
        float& operator []  (const uint32_t index);
        bool operator!= (const Vector3f vec);
        bool operator== (const Vector3f vec);

    protected:
    private:
};
std::ostream& operator<< (std::ostream& output, const Vector3f& vec);

class Vector3i
{
    public:
        union {
            struct {
                int32_t x;
                int32_t y;
                int32_t z;
            };
            struct {
                int32_t r;
                int32_t g;
                int32_t b;
            };
            struct {
                Vector2i xy;
                int32_t z;
            };
            struct {
                Vector2i rg;
                int32_t b;
            };
        };

        Vector3i();
        Vector3i(int32_t xyz) : x(xyz), y(xyz), z(xyz) {};

        Vector3i(int32_t x, int32_t y, int32_t z) : x(x), y(y), z(z) {};
        Vector3i&    normal();
        Vector3i    normalize(Vector3i vec3);
        bool        equal(const Vector3i& vec, float e) const;

        void set(int32_t x, int32_t y, int32_t z);
        int32_t* get() { return &x; };

        Vector3i operator= (const Vector3i& vec);
        Vector3i operator+ (const Vector3i& vec)const;
        Vector3i operator- (const Vector3i& vec)const;
        Vector3i operator/ (const Vector3i& vec)const;
        Vector3i operator/ (const float& num)const;
        Vector3i operator* (const Vector3i& vec)const;
        Vector3i operator* (const float& num)const;
        void operator*= (const Vector3i& vec);
        Vector3i operator+= (const Vector3i& vec);
        Vector3i operator-= (const Vector3i& vec);
        float& operator []  (const uint32_t index);
        bool operator!= (const Vector3i& vec);

    protected:
    private:
};
std::ostream& operator<< (std::ostream& output, const Vector3i& vec);

/////////////////////////////////////////////////////////////////////////////////
//Vector4D
/////////////////////////////////////////////////////////////////////////////////
class Vector4f
{
    public:
        union {
            struct {
                float x;
                float y;
                float z;
                float w;
            };
            struct {
                Vector2f xy;
                Vector2f zw;
            };
            struct {
                Vector3f xyz;
                float w;
            };
        };

        Vector4f() : x(0), y(0), z(0), w(0) {};

        Vector4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};

        void set(float x, float y, float z, float w);
        float* get() { return &x; };

        Vector4f operator= (const Vector4f& vec);
        Vector4f operator+ (const Vector4f& vec)const;
        Vector4f operator- (const Vector4f& vec)const;
        Vector4f operator/ (const Vector4f& vec)const;
        Vector4f operator/ (const float& num)const;
        Vector4f operator* (const Vector4f& vec)const;
        Vector4f operator* (const float& num)const;
        bool operator!= (const Vector4f& vec);

    protected:
    private:
};

std::ostream& operator<< (std::ostream& output, const Vector4f& vec);

class Vector4i
{
    public:
        union{
            struct {
                int32_t x;
                int32_t y;
                int32_t z;
                int32_t w;
            };
            struct {
                Vector2i xy;
                Vector2i zw;
            };
            struct {
                Vector3i xyz;
                int32_t w;
            };
        };

        Vector4i() : x(0), y(0), z(0), w(0) {};

        Vector4i(int32_t x, int32_t y, int32_t z, int32_t w) : x(x), y(y), z(z), w(w) {};

        void set(int32_t x, int32_t y, int32_t z, int32_t w);
        int32_t* get() { return &x; };

        Vector4i operator= (const Vector4i& vec);
        Vector4i operator+ (const Vector4i& vec)const;
        Vector4i operator- (const Vector4i& vec)const;
        Vector4i operator/ (const Vector4i& vec)const;
        Vector4i operator/ (const float& num)const;
        Vector4i operator* (const Vector4i& vec)const;
        Vector4i operator* (const float& num)const;
        bool operator!= (const Vector4i& vec);

    protected:
    private:
};

std::ostream& operator<< (std::ostream& output, const Vector4i& vec);
