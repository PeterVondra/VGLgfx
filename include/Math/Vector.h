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
            struct { float x, y; };
            struct { float r, g; };
            struct { float s, t; };
        };

        Vector2f() {};

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
            struct { int32_t x, y; };
            struct { int32_t r, g; };
            struct { int32_t s, t; };
        };

        Vector2i() {};

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
            struct { float x, y, z; };
            struct { float r, g, b; };
        };

        Vector3f() {};
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
            struct { int32_t x, y, z; };
            struct { int32_t r, g, b; };
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
            struct { float x, y, z, w; };
            struct { float r, g, b, a; };
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
            struct { int32_t x, y, z, w; };
            struct { float r, g, b, a; };
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
