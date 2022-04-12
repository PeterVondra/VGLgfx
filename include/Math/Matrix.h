#pragma once

#include <iostream>
#include <cmath>
#include <iomanip>
#include <algorithm>

#include "Vector.h"
#include "Math.h"

/////////////////////////////////////////////////////////////////
//Matrix2fD
/////////////////////////////////////////////////////////////////
typedef class Matrix2f : public Math
{
    public:
        // constructors
        Matrix2f();  // init with identity
        Matrix2f(const float src[4]);
        Matrix2f(float m0, float m1, float m2, float m3);

        void set(const float src[4]);
        void set
        (
            float m0, float m1,
            float m2, float m3
        );

        Matrix2f&    identity();
        Matrix2f&    transpose();
        Matrix2f&    invert();

        float getDeterminant();

        // operators
        Matrix2f     operator+(const Matrix2f& rhs) const;
        Matrix2f     operator-(const Matrix2f& rhs) const;
        Matrix2f&    operator+=(const Matrix2f& rhs);
        Matrix2f&    operator-=(const Matrix2f& rhs);
        Matrix2f     operator*(const Matrix2f& rhs) const;
        Matrix2f&    operator*=(const Matrix2f& rhs);
        bool   operator==(const Matrix2f& rhs) const;
        bool   operator!=(const Matrix2f& rhs) const;
        float     operator[](int index) const;
        float&    operator[](int index);

    protected:

    private:
        float m[4];

}mat2x2;

inline float Matrix2f::getDeterminant()
{
    return m[0] * m[3] - m[1] * m[2];
}

/////////////////////////////////////////////////////////////////
//Matrix3fD
/////////////////////////////////////////////////////////////////
typedef class Matrix3f : public Math
{
    public:
        // constructors
        Matrix3f();
        Matrix3f(const float src[9]);
        Matrix3f(
            float m0, float m1, float m2,     // 1st column
            float m3, float m4, float m5,     // 2nd column
            float m6, float m7, float m8      // 3rd column
        );

        void set(const float src[9]);
        void set
        (
            float m0, float m1, float m2,   // 1st column
            float m3, float m4, float m5,   // 2nd column
            float m6, float m7, float m8    // 3rd column
        );

        void    identity();
        Matrix3f&    transpose();
        Matrix3f&    invert();

        float getDeterminant();

        // operators
        Matrix3f     operator+(const Matrix3f& rhs) const;
        Matrix3f     operator-(const Matrix3f& rhs) const;
        Matrix3f&    operator+=(const Matrix3f& rhs);
        Matrix3f&    operator-=(const Matrix3f& rhs);
        Matrix3f     operator*(const Matrix3f& rhs) const;
        Matrix3f&    operator*=(const Matrix3f& rhs);
        bool        operator==(const Matrix3f& rhs) const;
        bool        operator!=(const Matrix3f& rhs) const;
        float       operator[](int index) const;
        float&      operator[](int index);

    protected:

    private:
        float m[9];

}mat3x3;

std::ostream& operator<< (std::ostream& output, Matrix3f& mat3);

/////////////////////////////////////////////////////////////////
//Matrix4fD
/////////////////////////////////////////////////////////////////
typedef class Matrix4f : public Math
{
    public:
        Matrix4f();

        Matrix4f(const float stc[16]);

        Matrix4f
        (
            float m00, float m01, float m02, float m03, // 1st column
            float m04, float m05, float m06, float m07, // 2nd column
            float m08, float m09, float m10, float m11, // 3rd column
            float m12, float m13, float m14, float m15  // 4th column
        );

        void set(const float src[16]);
        void set(
            float m00, float m01, float m02, float m03, // 1st column
            float m04, float m05, float m06, float m07, // 2nd column
            float m08, float m09, float m10, float m11, // 3rd column
            float m12, float m13, float m14, float m15  // 4th column
        );

        float* get();

		static Matrix4f lookAtRH(Vector3f const &eye, Vector3f const &center, Vector3f const &up);
		static Matrix4f lookAtLH(Vector3f const &eye, Vector3f const &center, Vector3f const &up);

        static Matrix4f perspectiveRH   (float FOV, float ASPECT, float zNear, float zFar);
        static Matrix4f perspectiveRH_ZO(float FOV, float ASPECT, float zNear, float zFar);
        static Matrix4f perspectiveLH   (float FOV, float ASPECT, float zNear, float zFar);
        static Matrix4f perspectiveLH_ZO(float FOV, float ASPECT, float zNear, float zFar);
        static Matrix4f orthoRH
        (
         const float left, const float right,
         const float bottom, const float top,
         const float zNear, const float zFar
        );
        static Matrix4f orthoRH_ZO
        (
         const float left, const float right,
         const float bottom, const float top,
         const float zNear, const float zFar
        );
		static Matrix4f orthoLH
		(
			const float left, const float right,
			const float bottom, const float top,
			const float zNear, const float zFar
		);
		static Matrix4f orthoLH_ZO
		(
			const float left, const float right,
			const float bottom, const float top,
			const float zNear, const float zFar
		);

        static Matrix4f getIdentity();
        Matrix4f& identity();
        Matrix4f& transpose();
        Matrix4f  getInverse();
        Matrix4f& translate(const Vector3f& v);
        Matrix4f& translate(const float x, const float y, const float z);
        Matrix4f& scale(const float s);
        Matrix4f& scale(const Vector3f scalar);
        Matrix4f& scale(const float x, const float y, const float z);

        void        setRow(int index, const float row[4]);
        void        setRow(int index, const Vector4f& v);
        void        setRow(int index, const Vector3f& v);
        void        setColumn(int index, const float col[4]);
        void        setColumn(int index, const Vector4f& v);
        void        setColumn(int index, const Vector3f& v);

        float getDeterminant() const;

        Matrix4f& rotateX(float angle);
        Matrix4f& rotateY(float angle);
        Matrix4f& rotateZ(float angle);
        Matrix4f& rotate(float angle, const Vector3f& axis);
        Matrix4f& rotate(float angle, float x, float y, float z);

        // operators
        Matrix4f       operator+  (const    Matrix4f& rhs);
        Matrix4f       operator-  (const    Matrix4f& rhs);
        Matrix4f       operator*  (const    Matrix4f& rhs);
        Vector4f       operator*  (const    Vector4f& rhs);
        Vector3f       operator*  (const    Vector3f rhs);
        Matrix4f&      operator+= (const    Matrix4f& rhs);
        Matrix4f&      operator-= (const    Matrix4f& rhs);
        Matrix4f&      operator*= (const    Matrix4f& rhs);
        bool          operator== (const    Matrix4f&   n) const;
        bool          operator!= (const    Matrix4f&   n) const;
        float         operator[] (int      index       ) const;
        float&        operator[] (int      index       );

    protected:
    private:
        float getCofactor
        (
            float m0, float m1, float m2,
            float m3, float m4, float m5,
            float m6, float m7, float m8
        ) const;

        float m[16];
}mat4x4;

std::ostream& operator<< (std::ostream& output, Matrix4f& mat4);

/////////////////////////////////////////////////////////////////
//MatrixND
/////////////////////////////////////////////////////////////////
class MatrixN
{
public:
    MatrixN() { };
    //template<typename first, typename ... t_rest>
    //MatrixN(const int rows, const int columns, first value, t_rest... rest)
    //: m_rows(rows), m_columns(columns)
    //{
    //    push_all(m, value, rest...);
    //}
    MatrixN(const int rows, const int columns, const double value)
        : m_rows(rows), m_columns(columns)
    {
        for (int i = 0; i < rows * columns; i++)
        {
            m.push_back(value);
        }
    }
    // X is min and Y is max
    MatrixN(const int rows, const int columns, Vector2f random_value_extent)
        : m_rows(rows), m_columns(columns)
    {
        for (int i = 0; i < rows * columns; i++)
        {
            m.push_back(Math::getRandomNumberND(random_value_extent.x, random_value_extent.y));
        }
    }

    std::vector<double>& getRawData() { return m; };

    double& getRow(unsigned int row, unsigned int rowIndex);
    MatrixN getRow(unsigned int row);
    double& getColumn(unsigned int column, unsigned int columnIndex);
    MatrixN getColumn(unsigned int column);

    void setRow(unsigned int row, unsigned int rowIndex, const double value);
    void setColumn(unsigned int column, unsigned int columnIndex, const double value);

    MatrixN dot(MatrixN& a);
    MatrixN hadamard(MatrixN& a);
    MatrixN abs();
    double tcnorm();
    MatrixN getSign();

    double getRowSize() { return m_rows; }
    double getColumnSize() { return m_columns; }

    Vector3f operator*(Vector3f& rhs);
    MatrixN operator*(MatrixN& rhs);
    MatrixN operator*(double rhs);
    MatrixN operator+(double rhs);
    MatrixN operator+(MatrixN& rhs);
    MatrixN operator-(MatrixN& rhs);
    double& operator[](unsigned int index) { return m[index]; }

    void transpose();
    MatrixN transposed();

    void print(const uint16_t p_Precision);
    void print(const uint16_t p_Precision, const double p_HiddenNumber);
    void print(const uint64_t p_Rows, const uint64_t p_Columns, const uint16_t p_Precision);
    void print(const uint64_t p_Rows, const uint64_t p_Columns, const uint16_t p_Precision, const double p_HiddenNumber);

    unsigned int m_rows, m_columns;
protected:
private:
    std::vector<double> m;

    //void push_all(std::vector<double>& vec){}
    //template<typename... Rest>
    //void push_all(std::vector<double>& vec, double val, Rest ... rest)
    //{
    //    vec.push_back(val);
    //    push_all(vec, rest...);
    //}
};

std::ostream& operator<< (std::ostream& output, MatrixN& matN);