#include "Matrix.h"

#include "Vector.h"
#include "Math.h"

/////////////////////////////////////////////////////////////////
//Matrix3fD
/////////////////////////////////////////////////////////////////

Matrix3f::Matrix3f()
{
    identity();
}

Matrix3f::Matrix3f
(
    float m0, float m1, float m2,     // 1st column
    float m3, float m4, float m5,     // 2nd column
    float m6, float m7, float m8      // 3rd column
)
{
    m[0] = m0;
    m[1] = m1;
    m[2] = m2;
    m[3] = m3;
    m[4] = m4;
    m[5] = m5;
    m[6] = m6;
    m[7] = m7;
    m[8] = m8;
}

Matrix3f& Matrix3f::transpose()
{
    std::swap(m[1],  m[3]);
    std::swap(m[2],  m[6]);
    std::swap(m[5],  m[7]);

    return *this;
}

float Matrix3f::getDeterminant()
{
    return 
           m[0] * (m[4] * m[8] - m[5] * m[7])
         - m[3] * (m[1] * m[8] - m[2] * m[7])
         + m[6] * (m[1] * m[5] - m[2] * m[4]);
}

 void Matrix3f::identity()
{
    m[0] = m[4] = m[8] = 1.0f;
    m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = 0.0f;
}

std::ostream& operator<< (std::ostream& output, Matrix3f& mat3);

 float Matrix3f::operator[](int index) const
{
    return m[index];
}

 float& Matrix3f::operator[](int index)
{
    return m[index];
}

std::ostream& operator<< (std::ostream& output, Matrix3f& mat3)
{
    output << std::fixed << std::setprecision(5) <<
    "          ======Matrix==3x3======\n" <<
    "[ " << mat3[0] << "    " << mat3[1] << "    " << mat3[2] << "]" << std::endl <<
    "[ " << mat3[3] << "    " << mat3[4] << "    " << mat3[5] << "]" << std::endl <<
    "[ " << mat3[6] << "    " << mat3[7] << "    " << mat3[8] << "]" << std::endl;

    return output;
}

/////////////////////////////////////////////////////////////////
//Matrix4D
/////////////////////////////////////////////////////////////////

Matrix4f& Matrix4f::transpose()
{
    std::swap(m[1],  m[4]);     //    [0  1   2  3]       [0  4  8  12]
    std::swap(m[2],  m[8]);     //    [4  5   6  7]  __\  [1  5  9  13]
    std::swap(m[3],  m[12]);    //    [8  9  10 11]    /  [2  6  10 14]
    std::swap(m[6],  m[9]);     //    [12 13 14 15]       [3  7  11 15]
    std::swap(m[7],  m[13]);
    std::swap(m[11], m[14]);

    return *this;
}

float Matrix4f::getDeterminant() const
{
    return m[0] * getCofactor(m[5],m[6],m[7], m[9],m[10],m[11], m[13],m[14],m[15]) -
           m[1] * getCofactor(m[4],m[6],m[7], m[8],m[10],m[11], m[12],m[14],m[15]) +
           m[2] * getCofactor(m[4],m[5],m[7], m[8],m[9], m[11], m[12],m[13],m[15]) -
           m[3] * getCofactor(m[4],m[5],m[6], m[8],m[9], m[10], m[12],m[13],m[14]);
}

Matrix4f& Matrix4f::rotate(float angle, const Vector3f& axis)
{
    return rotate(angle, axis.x, axis.y, axis.z);
}

Matrix4f& Matrix4f::rotate(float angle, float x, float y, float z)
{
    float c = cosf(angle * Math::DEG2RAD);    // cosine
    float s = sinf(angle * Math::DEG2RAD);    // sine
    float c1 = 1.0f - c;                // 1 - c

    float
    m0 = m[0],  m4 = m[4],  m8 = m[8],  m12 = m[12],
    m1 = m[1],  m5 = m[5],  m9 = m[9],  m13 = m[13],
    m2 = m[2],  m6 = m[6], m10 = m[10], m14 = m[14];

    // build rotation matrix
    float r0 = x * x * c1 + c;
    float r1 = x * y * c1 + z * s;
    float r2 = x * z * c1 - y * s;
    float r4 = x * y * c1 - z * s;
    float r5 = y * y * c1 + c;
    float r6 = y * z * c1 + x * s;
    float r8 = x * z * c1 + y * s;
    float r9 = y * z * c1 - x * s;
    float r10= z * z * c1 + c;

    // multiply rotation matrix
    m[0] = r0 * m0 + r4 * m1 + r8 * m2;
    m[1] = r1 * m0 + r5 * m1 + r9 * m2;
    m[2] = r2 * m0 + r6 * m1 + r10* m2;
    m[4] = r0 * m4 + r4 * m5 + r8 * m6;
    m[5] = r1 * m4 + r5 * m5 + r9 * m6;
    m[6] = r2 * m4 + r6 * m5 + r10* m6;
    m[8] = r0 * m8 + r4 * m9 + r8 * m10;
    m[9] = r1 * m8 + r5 * m9 + r9 * m10;
    m[10]= r2 * m8 + r6 * m9 + r10* m10;
    m[12]= r0 * m12+ r4 * m13+ r8 * m14;
    m[13]= r1 * m12+ r5 * m13+ r9 * m14;
    m[14]= r2 * m12+ r6 * m13+ r10* m14;

    return *this;
}

Matrix4f& Matrix4f::rotateX(float angle)
{
    float c = cosf(angle * Math::DEG2RAD);
    float s = sinf(angle * Math::DEG2RAD);
    float m1 = m[1],  m2 = m[2],
          m5 = m[5],  m6 = m[6],
          m9 = m[9],  m10= m[10],
          m13= m[13], m14= m[14];

    m[1] = m1 * c + m2 *-s;
    m[2] = m1 * s + m2 * c;
    m[5] = m5 * c + m6 *-s;
    m[6] = m5 * s + m6 * c;
    m[9] = m9 * c + m10*-s;
    m[10]= m9 * s + m10* c;
    m[13]= m13* c + m14*-s;
    m[14]= m13* s + m14* c;

    return *this;
}

Matrix4f& Matrix4f::rotateY(float angle)
{
    float c = cosf(angle * Math::DEG2RAD);
    float s = sinf(angle * Math::DEG2RAD);
    float m0 = m[0],  m2 = m[2],
          m4 = m[4],  m6 = m[6],
          m8 = m[8],  m10= m[10],
          m12= m[12], m14= m[14];

    m[0] = m0 * c + m2 * s;
    m[2] = m0 *-s + m2 * c;
    m[4] = m4 * c + m6 * s;
    m[6] = m4 *-s + m6 * c;
    m[8] = m8 * c + m10* s;
    m[10]= m8 *-s + m10* c;
    m[12]= m12* c + m14* s;
    m[14]= m12*-s + m14* c;

    return *this;
}

Matrix4f& Matrix4f::rotateZ(float angle)
{
    float c = cosf(angle * Math::DEG2RAD);
    float s = sinf(angle * Math::DEG2RAD);
    float m0 = m[0],  m1 = m[1],
          m4 = m[4],  m5 = m[5],
          m8 = m[8],  m9 = m[9],
          m12= m[12], m13= m[13];

    m[0] = m0 * c + m1 *-s;
    m[1] = m0 * s + m1 * c;
    m[4] = m4 * c + m5 *-s;
    m[5] = m4 * s + m5 * c;
    m[8] = m8 * c + m9 *-s;
    m[9] = m8 * s + m9 * c;
    m[12]= m12* c + m13*-s;
    m[13]= m12* s + m13* c;

    return *this;
}

Matrix4f Matrix4f::perspectiveRH(float FOV, float ASPECT, float zNear, float zFar)
{
    float yScale = tan((Math::DEG2RAD * FOV) / 2);

    Matrix4f Result = getIdentity();

    Result[0] = 1 / (ASPECT * yScale);
    Result[1] = 0.0f;
    Result[2] = 0.0f;
    Result[3] = 0.0f;

    Result[4] = 0.0f;
    Result[5] = 1 / yScale;
    Result[6] = 0.0f;
    Result[7] = 0.0f;

	Result[8] = 0.0f;
	Result[9] = 0.0f;
	Result[10] = -(zFar + zNear) / (zFar - zNear);
	Result[11] = -1.0f;

	Result[12] = 0.0f;
	Result[13] = 0.0f;
	Result[14] = -(2 * zFar * zNear) / (zFar - zNear);
	Result[15] = 0.0f;

    Result.transpose();

    return Result;
}
Matrix4f Matrix4f::perspectiveRH_ZO(float FOV, float ASPECT, float zNear, float zFar)
{
	float yScale = tan((Math::DEG2RAD * FOV) / 2);

	Matrix4f Result = getIdentity();

	Result[0] = 1 / (ASPECT * yScale);
	Result[1] = 0.0f;
	Result[2] = 0.0f;
	Result[3] = 0.0f;

	Result[4] = 0.0f;
	Result[5] = 1 / yScale;
	Result[6] = 0.0f;
	Result[7] = 0.0f;

	Result[8] = 0.0f;
	Result[9] = 0.0f;
	Result[10] = zFar / (zNear - zFar);
	Result[11] = -1.0f;

	Result[12] = 0.0f;
	Result[13] = 0.0f;
	Result[14] = -(zFar * zNear) / (zFar - zNear);
	Result[15] = 0.0f;

	Result.transpose();

	return Result;
}

Matrix4f Matrix4f::perspectiveLH(float FOV, float ASPECT, float zNear, float zFar)
{
	float yScale = tan((Math::DEG2RAD * FOV) / 2);

	Matrix4f Result = getIdentity();

	Result[0] = 1 / (ASPECT * yScale);
	Result[1] = 0.0f;
	Result[2] = 0.0f;
	Result[3] = 0.0f;

	Result[4] = 0.0f;
	Result[5] = 1 / yScale;
	Result[6] = 0.0f;
	Result[7] = 0.0f;

	Result[8] = 0.0f;
	Result[9] = 0.0f;
	Result[10] = -(zFar + zNear) / (zFar - zNear);
	Result[11] = 1.0f;
	
	Result[12] = 0.0f;
	Result[13] = 0.0f;
	Result[14] = -(2 * zFar * zNear) / (zFar - zNear);
	Result[15] = 0.0f;

	Result.transpose();

	return Result;
}
Matrix4f Matrix4f::perspectiveLH_ZO(float FOV, float ASPECT, float zNear, float zFar)
{
	float yScale = tan((Math::DEG2RAD * FOV) / 2);

	Matrix4f Result = getIdentity();

	Result[0] = 1 / (ASPECT * yScale);
	Result[1] = 0.0f;
	Result[2] = 0.0f;
	Result[3] = 0.0f;

	Result[4] = 0.0f;
	Result[5] = 1 / yScale;
	Result[6] = 0.0f;
	Result[7] = 0.0f;

	Result[8] = 0.0f;
	Result[9] = 0.0f;
	Result[10] = zFar / (zFar - zNear);
	Result[11] = 1.0f;

	Result[12] = 0.0f;
	Result[13] = 0.0f;
	Result[14] = -(zFar * zNear) / (zFar - zNear);
	Result[15] = 0.0f;

	Result.transpose();

	return Result;
}

Matrix4f Matrix4f::orthoRH
(
	const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar
)
{
	Matrix4f ortho;
	ortho[0] = 2 / (right - left);
	ortho[1] = 0;
	ortho[2] = 0;
	ortho[3] = 0;

	ortho[4] = 0;
	ortho[5] = 2 / (top - bottom);
	ortho[6] = 0;
	ortho[7] = 0;

	ortho[8] = 0;
	ortho[9] = 0;
	ortho[10] = -2 / (zFar - zNear);
	ortho[11] = 0;

	ortho[12] = -(right + left) / (right - left);
	ortho[13] = -(top + bottom) / (top - bottom);
	ortho[14] = -(zFar + zNear) / (zFar - zNear);
	ortho[15] = 1;

	return ortho;
}
Matrix4f Matrix4f::orthoRH_ZO
(
	const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar
)
{
	Matrix4f ortho;
	ortho[0] = 2 / (right - left);
	ortho[1] = 0;
	ortho[2] = 0;
	ortho[3] = 0;

	ortho[4] = 0;
	ortho[5] = 2 / (top - bottom);
	ortho[6] = 0;
	ortho[7] = 0;

	ortho[8] = 0;
	ortho[9] = 0;
	ortho[10] = -1 / (zFar - zNear);
	ortho[11] = 0;

	ortho[12] = -(right + left) / (right - left);
	ortho[13] = -(top + bottom) / (top - bottom);
	ortho[14] = -zNear / (zFar - zNear);
	ortho[15] = 1;

	return ortho;
}
Matrix4f Matrix4f::orthoLH
(
	const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar
)
{
	Matrix4f ortho;
	ortho[0] = 2 / (right - left);
	ortho[1] = 0;
	ortho[2] = 0;
	ortho[3] = 0;

	ortho[4] = 0;
	ortho[5] = 2 / (top - bottom);
	ortho[6] = 0;
	ortho[7] = 0;

	ortho[8] = 0;
	ortho[9] = 0;
	ortho[10] = 2 / (zFar - zNear);
	ortho[11] = 0;

	ortho[12] = -(right + left) / (right - left);
	ortho[13] = -(top + bottom) / (top - bottom);
	ortho[14] = -(zFar + zNear) / (zFar - zNear);
	ortho[15] = 1;

	return ortho;
}
Matrix4f Matrix4f::orthoLH_ZO
(
	const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar
)
{
	Matrix4f ortho;
	ortho[0] = 2 / (right - left);
	ortho[1] = 0;
	ortho[2] = 0;
	ortho[3] = 0;

	ortho[4] = 0;
	ortho[5] = 2 / (top - bottom);
	ortho[6] = 0;
	ortho[7] = 0;

	ortho[8] = 0;
	ortho[9] = 0;
	ortho[10] = 1 / (zFar - zNear);
	ortho[11] = 0;

	ortho[12] = -(right + left) / (right - left);
	ortho[13] = -(top + bottom) / (top - bottom);
	ortho[14] = -zNear / (zFar - zNear);
	ortho[15] = 1;

	return ortho;
}

Matrix4f Matrix4f::lookAtRH(Vector3f const &eye, Vector3f const &center, Vector3f const &up)
{
    Vector3f const f(normalize(center - eye));
    Vector3f const s(normalize(cross(f, up)));
    Vector3f const u(cross(s, f));

    mat4x4 Result;

    Result[0] = s.x;           Result[4] = u.x;           Result[8]  = -f.x;          Result[12] = 0;
    Result[1] = s.y;           Result[5] = u.y;           Result[9]  = -f.y;          Result[13] = 0;
    Result[2] = s.z;           Result[6] = u.z;           Result[10] = -f.z;          Result[14] = 0;
    Result[3] = -dot(s, eye);  Result[7] = -dot(u, eye);  Result[11] = dot(f, eye);   Result[15] = 1;

    Result.transpose();

    return Result;
}
Matrix4f Matrix4f::lookAtLH(Vector3f const &eye, Vector3f const &center, Vector3f const &up)
{
	Vector3f const f(normalize(center - eye));
	Vector3f const s(normalize(cross(up, f)));
	Vector3f const u(cross(f, s));

	mat4x4 Result;

	Result[0] = s.x;           Result[4] = u.x;           Result[8] =  f.x;            Result[12] = 0;
	Result[1] = s.y;           Result[5] = u.y;           Result[9] =  f.y;            Result[13] = 0;
	Result[2] = s.z;           Result[6] = u.z;           Result[10] = f.z;            Result[14] = 0;
	Result[3] = -dot(s, eye);  Result[7] = -dot(u, eye);  Result[11] = -dot(f, eye);   Result[15] = 1;

	Result.transpose();

	return Result;
}

Matrix4f Matrix4f::getInverse()
{
    Matrix4f adjugate;
    adjugate = *this;

    //             
    //  [0, 4, 8,  12]
    //  [1, 5, 9,  13]
    //  [2, 6, 10, 14]
    //  [3, 7, 11, 15]
    //
    adjugate[0]  =  Matrix3f (m[5], m[6], m[7], m[9], m[10], m[11], m[13], m[14], m[15]).getDeterminant();
    adjugate[1]  = -Matrix3f (m[4], m[6], m[7], m[8], m[10], m[11], m[12], m[14], m[15]).getDeterminant();
    adjugate[2]  =  Matrix3f (m[4], m[5], m[7], m[8], m[9],  m[11], m[12], m[13], m[15]).getDeterminant();
    adjugate[3]  = -Matrix3f (m[4], m[5], m[6], m[8], m[9],  m[10], m[12], m[13], m[14]).getDeterminant();

    adjugate[4]  = -Matrix3f (m[1], m[2], m[3], m[9], m[10], m[11], m[13], m[14], m[15]).getDeterminant();
    adjugate[5]  =  Matrix3f (m[0], m[2], m[3], m[8], m[10], m[11], m[12], m[14], m[15]).getDeterminant();
    adjugate[6]  = -Matrix3f (m[0], m[1], m[3], m[8], m[9],  m[11], m[12], m[13], m[15]).getDeterminant();
    adjugate[7]  =  Matrix3f (m[0], m[1], m[2], m[8], m[9],  m[10], m[12], m[13], m[14]).getDeterminant();

    adjugate[8]  =  Matrix3f (m[1], m[2], m[3], m[5], m[6],  m[7],  m[13], m[14], m[15]).getDeterminant();
    adjugate[9]  = -Matrix3f (m[0], m[2], m[3], m[4], m[6],  m[7],  m[12], m[14], m[15]).getDeterminant();
    adjugate[10] =  Matrix3f (m[0], m[1], m[3], m[4], m[5],  m[7],  m[12], m[13], m[15]).getDeterminant();
    adjugate[11] = -Matrix3f (m[0], m[1], m[2], m[4], m[5],  m[6],  m[12], m[13], m[14]).getDeterminant();

    adjugate[12] = -Matrix3f (m[1], m[2], m[3], m[5], m[6],  m[7],  m[9],  m[10], m[11]).getDeterminant();
    adjugate[13] =  Matrix3f (m[0], m[2], m[3], m[4], m[6],  m[7],  m[8],  m[10], m[11]).getDeterminant();
    adjugate[14] = -Matrix3f (m[0], m[1], m[3], m[4], m[5],  m[7],  m[8],  m[9],  m[11]).getDeterminant();
    adjugate[15] =  Matrix3f (m[0], m[1], m[2], m[4], m[5],  m[6],  m[8],  m[9],  m[10]).getDeterminant();

    adjugate.transpose();

    float det = getDeterminant();

	if (det)
		return Matrix4f
		(
			adjugate[0] / det, adjugate[4] / det, adjugate[8] / det, adjugate[12] / det,
			adjugate[1] / det, adjugate[5] / det, adjugate[9] / det, adjugate[13] / det,
			adjugate[2] / det, adjugate[6] / det, adjugate[10] / det, adjugate[14] / det,
			adjugate[3] / det, adjugate[7] / det, adjugate[11] / det, adjugate[15] / det
		);
	else
		return Matrix4f::getIdentity();
}

std::ostream& operator<< (std::ostream& output, Matrix4f& mat4);

Math::Math()
{
    //void constructor
}

Matrix4f::Matrix4f()
{
    identity();
}

Matrix4f::Matrix4f(const float stc[16])
{
	m[0] = stc[0];
	m[1] = stc[1];
	m[2] = stc[2];
	m[3] = stc[3];
	m[4] = stc[4];
	m[5] = stc[5];
	m[6] = stc[6];
	m[7] = stc[7];
	m[8] = stc[8];
	m[9] = stc[9];
	m[10] = stc[10];
	m[11] = stc[11];
	m[12] = stc[12];
	m[13] = stc[13];
	m[14] = stc[14];
	m[15] = stc[15];
}

float* Matrix4f::get()
{
    return m;
}

void Matrix4f::set
(
    float m00, float m01, float m02, float m03,
    float m04, float m05, float m06, float m07,
    float m08, float m09, float m10, float m11,
    float m12, float m13, float m14, float m15)
{
    m[0] = m00;  m[1] = m01;  m[2] = m02;  m[3] = m03;
    m[4] = m04;  m[5] = m05;  m[6] = m06;  m[7] = m07;
    m[8] = m08;  m[9] = m09;  m[10]= m10;  m[11]= m11;
    m[12]= m12;  m[13]= m13;  m[14]= m14;  m[15]= m15;
}

void Matrix4f::setRow(int index, const float row[4])
{
    m[index] = row[0];  m[index + 4] = row[1];  m[index + 8] = row[2];  m[index + 12] = row[3];
}

void Matrix4f::setRow(int index, const Vector4f& v)
{
    m[index] = v.x;  m[index + 4] = v.y;  m[index + 8] = v.z;  m[index + 12] = v.w;
}

void Matrix4f::setRow(int index, const Vector3f& v)
{
    m[index] = v.x;  m[index + 4] = v.y;  m[index + 8] = v.z;
}

void Matrix4f::setColumn(int index, const float col[4])
{
    m[index*4] = col[0];
    m[index*4 + 1] = col[1];
    m[index*4 + 2] = col[2];
    m[index*4 + 3] = col[3];
}

void Matrix4f::setColumn(int index, const Vector4f& v)
{
    m[index*4] = v.x;
    m[index*4 + 1] = v.y;
    m[index*4 + 2] = v.z;
    m[index*4 + 3] = v.w;
}

void Matrix4f::setColumn(int index, const Vector3f& v)
{
    m[index*4] = v.x;
    m[index*4 + 1] = v.y;
    m[index*4 + 2] = v.z;
}

float Matrix4f::getCofactor
(
    float m0, float m1, float m2,
    float m3, float m4, float m5,
    float m6, float m7, float m8) const
{
    return m0 * (m4 * m8 - m5 * m7) -
           m1 * (m3 * m8 - m5 * m6) +
           m2 * (m3 * m7 - m4 * m6);
}

Matrix4f& Matrix4f::translate(const Vector3f& v)
{
    return translate(v.x, v.y, v.z);
}

Matrix4f& Matrix4f::translate(float x, float y, float z)
{
    m[0] += m[3] * x;
    m[1] += m[3] * y;
    m[2] += m[3] * z;

    m[4] += m[7] * x;
    m[5] += m[7] * y;
    m[6] += m[7] * z;

    m[8] += m[11]* x;
    m[9] += m[11]* y;
    m[10]+= m[11]* z;

    m[12]+= m[15]* x;
    m[13]+= m[15]* y;
    m[14]+= m[15]* z;

    return *this;
}

Matrix4f& Matrix4f::scale(const float s)
{
    return scale(s, s, s);
}

Matrix4f& Matrix4f::scale(const Vector3f scalar)
{
	m[0] *= scalar.x;
	m[1] *= scalar.y;
	m[2] *= scalar.z;

	m[4] *= scalar.x;
	m[5] *= scalar.y;
	m[6] *= scalar.z;
			
	m[8] *= scalar.x;
	m[9] *= scalar.y;
	m[10] *= scalar.z;

	m[12] *= scalar.x;
	m[13] *= scalar.y;
	m[14] *= scalar.z;

	return *this;
}

Matrix4f& Matrix4f::scale(const float x, const float y, const float z)
{
    m[0] *= x;
    m[1] *= y;
    m[2] *= z;

    m[4] *= x;
    m[5] *= y;
    m[6] *= z;

    m[8] *= x;
    m[9] *= y;
    m[10]*= z;  

    m[12] *= x;
    m[13] *= y;
    m[14] *= z;

    return *this;
}

Matrix4f& Matrix4f::identity()
{
    m[0] = m[5] = m[10] = m[15] = 1.0f;
    m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
    return *this;
}
Matrix4f Matrix4f::getIdentity()
{
    Matrix4f identityMatrix
    (
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return identityMatrix;
}

Matrix4f::Matrix4f
(
    float m00, float m01, float m02, float m03, // 1st column
    float m04, float m05, float m06, float m07, // 2nd column
    float m08, float m09, float m10, float m11, // 3rd column
    float m12, float m13, float m14, float m15  // 4th column
)
{
    m[0] = m00; m[1] = m01; m[2] = m02; m[3] = m03;      // 1st column
    m[4] = m04; m[5] = m05; m[6] = m06; m[7] = m07;      // 2nd column
    m[8] = m08; m[9] = m09; m[10] = m10; m[11] = m11;    // 3rd column
    m[12] = m12; m[13] = m13; m[14] = m14; m[15] = m15;  // 4th column
}

Matrix4f Matrix4f::operator+(const Matrix4f& rhs)
{
    m[0] += rhs.m[0];
    m[1] += rhs.m[1];
    m[2] += rhs.m[2];
    m[3] += rhs.m[3];
    m[4] += rhs.m[4];
    m[5] += rhs.m[5];
    m[6] += rhs.m[6];
    m[7] += rhs.m[7];
    m[8] += rhs.m[8];
    m[9] += rhs.m[9];
    m[10] += rhs.m[10];
    m[11] += rhs.m[11];
    m[12] += rhs.m[12];
    m[13] += rhs.m[13];
    m[14] += rhs.m[14];
    m[15] += rhs.m[15];

    return *this;
}

Matrix4f Matrix4f::operator-(const Matrix4f& rhs)
{
    m[0] *= rhs.m[0];
    m[1] *= rhs.m[1];
    m[2] *= rhs.m[2];
    m[3] *= rhs.m[3];
    m[4] *= rhs.m[4];
    m[5] *= rhs.m[5];
    m[6] *= rhs.m[6];
    m[7] *= rhs.m[7];
    m[8] *= rhs.m[8];
    m[9] *= rhs.m[9];
    m[10] *= rhs.m[10];
    m[11] *= rhs.m[11];
    m[12] *= rhs.m[12];
    m[13] *= rhs.m[13];
    m[14] *= rhs.m[14];
    m[15] *= rhs.m[15];

    return *this;
}

Matrix4f Matrix4f::operator*(const Matrix4f& rhs)
{
    return Matrix4f
    (
        m[0]*rhs[0] + m[1]*rhs[4] + m[2]*rhs[8] + m[3]*rhs[12],
        m[0]*rhs[1] + m[1]*rhs[5] + m[2]*rhs[9] + m[3]*rhs[13],
        m[0]*rhs[2] + m[1]*rhs[6] + m[2]*rhs[10] + m[3]*rhs[14],
        m[0]*rhs[3] + m[1]*rhs[7] + m[2]*rhs[11] + m[3]*rhs[15],

        m[4]*rhs[0] + m[5]*rhs[4] + m[6]*rhs[8] + m[7]*rhs[12],
        m[4]*rhs[1] + m[5]*rhs[5] + m[6]*rhs[9] + m[7]*rhs[13],
        m[4]*rhs[2] + m[5]*rhs[6] + m[6]*rhs[10] + m[7]*rhs[14],
        m[4]*rhs[3] + m[5]*rhs[7] + m[6]*rhs[11] + m[7]*rhs[15],

        m[8]*rhs[0] + m[9]*rhs[4] + m[10]*rhs[8] + m[11]*rhs[12],
        m[8]*rhs[1] + m[9]*rhs[5] + m[10]*rhs[9] + m[11]*rhs[13],
        m[8]*rhs[2] + m[9]*rhs[6] + m[10]*rhs[10] + m[11]*rhs[14],
        m[8]*rhs[3] + m[9]*rhs[7] + m[10]*rhs[11] + m[11]*rhs[15],

        m[12]*rhs[0] + m[13]*rhs[4] + m[14]*rhs[8] + m[15]*rhs[12],
        m[12]*rhs[1] + m[13]*rhs[5] + m[14]*rhs[9] + m[15]*rhs[13],
        m[12]*rhs[2] + m[13]*rhs[6] + m[14]*rhs[10] + m[15]*rhs[14],
        m[12]*rhs[3] + m[13]*rhs[7] + m[14]*rhs[11] + m[15]*rhs[15]
    );
}

Vector4f Matrix4f::operator*(const Vector4f& rhs)
{
    return Vector4f
    (
        m[0]*rhs.x + m[4]*rhs.y + m[8]*rhs.z  + m[12]*rhs.w,
        m[1]*rhs.x + m[5]*rhs.y + m[9]*rhs.z  + m[13]*rhs.w,
        m[2]*rhs.x + m[6]*rhs.y + m[10]*rhs.z + m[14]*rhs.w,
        m[3]*rhs.x + m[7]*rhs.y + m[11]*rhs.z + m[15]*rhs.w
    );
}

Vector3f Matrix4f::operator*(const Vector3f rhs)
{
    return Vector3f
    (
        m[0]*rhs.x + m[4]*rhs.y + m[8]*rhs.z + m[12],
        m[1]*rhs.x + m[5]*rhs.y + m[9]*rhs.z + m[13],
        m[2]*rhs.x + m[6]*rhs.y + m[10]*rhs.z+ m[14]);
}

Matrix4f& Matrix4f::operator+=(const Matrix4f& rhs)
{
    *this = *this + rhs;

    return *this;
}

Matrix4f& Matrix4f::operator-=(const Matrix4f& rhs)
{
    *this = *this - rhs;

    return *this;
}

Matrix4f& Matrix4f::operator*=(const Matrix4f& rhs)
{
    *this = *this * rhs;
    return *this;
}

bool Matrix4f::operator==(const Matrix4f& n) const
{
    return (m[0] == n[0])  && (m[1] == n[1])  && (m[2] == n[2])  && (m[3] == n[3])  &&
           (m[4] == n[4])  && (m[5] == n[5])  && (m[6] == n[6])  && (m[7] == n[7])  &&
           (m[8] == n[8])  && (m[9] == n[9])  && (m[10]== n[10]) && (m[11]== n[11]) &&
           (m[12]== n[12]) && (m[13]== n[13]) && (m[14]== n[14]) && (m[15]== n[15]);
}
bool Matrix4f::operator!=(const Matrix4f& n) const
{
    return (m[0] != n[0])  || (m[1] != n[1])  || (m[2] != n[2])  || (m[3] != n[3])  ||
           (m[4] != n[4])  || (m[5] != n[5])  || (m[6] != n[6])  || (m[7] != n[7])  ||
           (m[8] != n[8])  || (m[9] != n[9])  || (m[10]!= n[10]) || (m[11]!= n[11]) ||
           (m[12]!= n[12]) || (m[13]!= n[13]) || (m[14]!= n[14]) || (m[15]!= n[15]);
}

 float Matrix4f::operator[](int index) const
{
    return m[index];
}

float& Matrix4f::operator[](int index)
{
    return m[index];
}

std::ostream& operator<< (std::ostream& output, Matrix4f& mat4)
{
    output << std::fixed << std::setprecision(5) <<
    "          ======Matrix==4x4======\n" <<
    "[ " << mat4[0] <<  "    " << mat4[1] <<  "    " << mat4[2] <<  "    "  << mat4[3]  << " ]\n" <<
    "[ " << mat4[4] <<  "    " << mat4[5] <<  "    " << mat4[6] <<  "    "  << mat4[7]  << " ]\n" <<
    "[ " << mat4[8] <<  "    " << mat4[9] <<  "    " << mat4[10] << "    "  << mat4[11] << " ]\n" <<
    "[ " << mat4[12] << "    " << mat4[13] << "    " << mat4[14] << "    "  << mat4[15] << " ]";

    return output;
}

/////////////////////////////////////////////////////////////////
//MatrixND
/////////////////////////////////////////////////////////////////

double& MatrixN::getRow(unsigned int row, unsigned int rowIndex)
{
    if (rowIndex > m_columns)
        rowIndex = m_columns - 1;
    if (row > m_rows)
        row = m_rows - 1;
    return m[(row + 1) * m_columns - (m_columns - rowIndex)];
}
double& MatrixN::getColumn(unsigned int column, unsigned int columnIndex)
{
    return getRow(columnIndex, column);
}

MatrixN MatrixN::getRow(unsigned int row)
{
    MatrixN result(1, m_columns, 0.0);
    for (int c = 0; c < m_columns; c++)
        result.getColumn(c, 0) = getRow(row, c);

    return result;
}
MatrixN MatrixN::getColumn(unsigned int column)
{
    MatrixN result(m_rows, 1, 0.0);
    for (int r = 0; r < m_rows; r++)
        result.getRow(r, 0) = getColumn(column, r);

    return result;
}
void MatrixN::setRow(unsigned int row, unsigned int rowIndex, const double value)
{
    if (rowIndex > m_columns)
        rowIndex = m_columns - 1;
    if (row > m_rows)
        row = m_rows - 1;
    m[(row + 1) * m_columns - (m_columns - rowIndex)] = value;
}
void MatrixN::setColumn(unsigned int column, unsigned int columnIndex, const double value)
{
    if (columnIndex > m_rows)
        columnIndex = m_rows - 1;
    if (column > m_columns)
        column = m_columns - 1;
    transpose();
    setRow(column, columnIndex, value);
    transpose();
}

MatrixN MatrixN::hadamard(MatrixN& rhs)
{
    MatrixN result(m_rows, m_columns, (double)0);

    if (m_columns == rhs.m_columns && m_rows == rhs.m_rows)
        for (int i = 0; i < rhs.m.size(); i++)
            result.m[i] = m[i] * rhs.m[i];
    else
        std::cout << "Matrices not of same dimensions: " << m_rows << "x" << m_columns << " and " << rhs.m_rows << "x" << rhs.m_columns << std::endl;
    return result;
}
MatrixN MatrixN::abs()
{
    MatrixN rhs = *this;
    for (auto& v : rhs.m)
        v = std::abs(v);
    return rhs;

}double MatrixN::tcnorm()
{
    double rhs = 0;
    for (auto& v : m)
        rhs += std::abs(v);
    return rhs;
}
MatrixN MatrixN::getSign()
{
    MatrixN result = *this;

    for (int i = 0; i < result.m.size(); i++)
        result.m[i] = m[i] / std::abs(m[i]);
    return result;
}

MatrixN MatrixN::dot(MatrixN& a)
{
    if (m_columns != a.m_rows)
    {
        MatrixN result;

        // If both are vectors
        if (m_columns == a.m_columns && m_rows == 1 && a.m_rows == 1) {
            a.transpose();
            result = MatrixN(m_rows, a.m_columns, (double)0);
            for (int i = 0; i < m_rows; i++)
                for (int j = 0; j < a.m_columns; j++)
                    for (int h = 0; h < m_columns; h++)
                        result.getRow(i, j) += getRow(i, h) * a.getColumn(j, h);
            a.transpose();
        }
        else if (m_rows == a.m_rows && m_rows == 1 && a.m_rows == 1) {
            result = MatrixN(m_rows, a.m_columns, (double)0);
            a.transpose();
            for (int i = 0; i < m_rows; i++)
                for (int j = 0; j < a.m_columns; j++)
                    for (int h = 0; h < m_columns; h++)
                        result.getRow(i, j) += getRow(i, h) * a.getColumn(j, h);
            a.transpose();
        }
        return result;
    }

    MatrixN result(m_rows, a.m_columns, (double)0);

    for (int i = 0; i < m_rows; i++)
        for (int j = 0; j < a.m_columns; j++)
            for (int h = 0; h < m_columns; h++)
                result.getRow(i, j) += getRow(i, h) * a.getColumn(j, h);

    return result;
}



void MatrixN::transpose()
{
    MatrixN temp(m_rows, m_columns, (double)0);
    temp = *this;
    std::swap(m_columns, m_rows);
    for (int i = 0; i < temp.getRowSize(); i++)
    {
        for (int j = 0; j < temp.getColumnSize(); j++)
        {
            setRow(j, i, temp.getRow(i, j));
        }
    }
}
MatrixN MatrixN::transposed()
{
    MatrixN temp(m_rows, m_columns, (double)0);

    std::swap(m_columns, m_rows);
    MatrixN result = *this;
    std::swap(m_columns, m_rows);

    temp = *this;

    for (int i = 0; i < temp.getRowSize(); i++)
    {
        for (int j = 0; j < temp.getColumnSize(); j++)
        {
            result.setRow(j, i, temp.getRow(i, j));
        }
    }

    return result;
}

Vector3f MatrixN::operator*(Vector3f& rhs)
{
    if (m_columns == 3)
    {
        Vector3f temp;
        for (unsigned int j = 0; j < m_columns; j++)
        {
            for (int i = 0; i < 3; i++)
            {
                temp[i] = float(rhs[i] * getColumn(j, i));
            }
        }
        return temp;
    }
    else {
        std::cout << "operation: (*) on Matrix[" << m_rows << "x" << m_columns << "], Vector3f : Failed" << std::endl;
        return Vector3f();
    }
}

MatrixN MatrixN::operator*(double rhs)
{
    MatrixN result = *this;
    for (auto& r : result.m)
        r *= rhs;
    return result;
}
MatrixN MatrixN::operator+(double rhs)
{
    MatrixN result = *this;
    for (auto& r : result.m)
        r += rhs;
    return result;
}

MatrixN MatrixN::operator*(MatrixN& rhs)
{
    return dot(rhs);
}
MatrixN MatrixN::operator+(MatrixN& rhs)
{
    if (m_rows == rhs.m_rows && m_columns == rhs.m_columns)
    {
        for (int i = 0; i < m.size(); i++)
        {
            m[i] += rhs.m[i];
        }
    }
    else
        std::cout << "operation: (+) on Matrix[" << m_rows << "x" << m_columns << "], Matrix[" << rhs.m_rows << "x" << rhs.m_columns << "] : Failed" << std::endl;

    return *this;
}
MatrixN MatrixN::operator-(MatrixN& rhs)
{
    if (m_rows == rhs.m_rows && m_columns == rhs.m_columns)
    {
        for (int i = 0; i < m.size(); i++)
        {
            m[i] -= rhs.m[i];
        }
    }
    else
        std::cout << "operation: (-) on Matrix[" << m_rows << "x" << m_columns << "], Matrix[" << rhs.m_rows << "x" << rhs.m_columns << "] : Failed" << std::endl;

    return *this;
}

void MatrixN::print(const uint16_t p_Precision)
{
    std::cout << std::fixed << std::setprecision(0) << "";
    std::cout << "Matrix " << getRowSize() << "x" << getColumnSize();
    int j = 0;
    for (int i = 0; i < getRowSize() * getColumnSize(); i++)
    {
        if (i == j * getColumnSize())
        {
            j++;
            std::cout << "\n";
        }
        std::cout << std::fixed << std::setprecision(p_Precision) << m[i] << "   ";
    }
    std::cout << "\n";
}

void MatrixN::print(const uint64_t p_Rows, const uint64_t p_Columns, const uint16_t p_Precision)
{
    std::cout << std::fixed << std::setprecision(0) << "";
    std::cout << "Matrix " << p_Rows << "x" << p_Columns;
    int j = 0;
    for (int i = 0; i < p_Rows * p_Columns; i++)
    {
        if (i == j * p_Columns)
        {
            j++;
            std::cout << "\n";
        }
        std::cout << std::fixed << std::setprecision(p_Precision) << m[i] << "   ";
    }
    std::cout << "\n";
}

void MatrixN::print(const uint16_t p_Precision, const double p_HiddenNumber)
{
    std::cout << std::fixed << std::setprecision(0) << "";
    std::cout << "Matrix " << getRowSize() << "x" << getColumnSize();
    int j = 0;
    for (int i = 0; i < getRowSize() * getColumnSize(); i++)
    {
        if (i == j * getColumnSize())
        {
            j++;
            std::cout << "\n";
        }
        if (m[i] == p_HiddenNumber)
            std::cout << std::fixed << std::setprecision(p_Precision) << "    ";
        else
            std::cout << std::fixed << std::setprecision(p_Precision) << m[i] << "   ";
    }
    std::cout << "\n";
}

void MatrixN::print(const uint64_t p_Rows, const uint64_t p_Columns, const uint16_t p_Precision, const double p_HiddenNumber)
{
    std::cout << std::fixed << std::setprecision(0) << "";
    std::cout << "Matrix " << p_Rows << "x" << p_Columns;
    int j = 0;
    for (int i = 0; i < p_Rows * p_Columns; i++)
    {
        if (i == j * p_Columns)
        {
            j++;
            std::cout << "\n";
        }
        if (m[i] == p_HiddenNumber)
            std::cout << std::fixed << std::setprecision(p_Precision) << "    ";
        else
            std::cout << std::fixed << std::setprecision(p_Precision) << m[i] << "   ";
    }
    std::cout << "\n";
}

std::ostream& operator<< (std::ostream& output, MatrixN& matN)
{
    output << std::fixed << std::setprecision(0) << "";
    output << "Matrix " << matN.getRowSize() << "x" << matN.getColumnSize();
    int j = 0;
    for (int i = 0; i < matN.getRowSize() * matN.getColumnSize(); i++)
    {
        if (i == j * matN.getColumnSize())
        {
            j++;
            std::cout << "\n";
        }
        output << std::fixed << std::setprecision(5) << matN[i] << "   ";
    }
    output << "\n";
    return output;
}