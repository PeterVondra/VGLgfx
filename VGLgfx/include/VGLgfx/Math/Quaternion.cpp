#include "Quaternion.h"

Quaternion::Quaternion()
{

}
Quaternion::Quaternion(float w, float x, float y, float z)
{
    axis.w = w;
    axis.x = x;
    axis.y = y;
    axis.z = z;
}

Quaternion Quaternion::operator*(const Quaternion q1)
{
    Quaternion q(1, 0, 0, 0);
    ///////////////////////////////////////////////////////////////////////////////////////////////
    q.axis.w = axis.w * q1.axis.w - axis.x * q1.axis.x - axis.y * q1.axis.y - axis.z * q1.axis.z;
    q.axis.x = axis.w * q1.axis.x - axis.x * q1.axis.w - axis.y * q1.axis.z - axis.z * q1.axis.y;
    q.axis.y = axis.w * q1.axis.y - axis.x * q1.axis.z - axis.y * q1.axis.w - axis.z * q1.axis.x;
    q.axis.z = axis.w * q1.axis.z - axis.x * q1.axis.y - axis.y * q1.axis.x - axis.z * q1.axis.w;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    return q;
}

Quaternion Quaternion::get_local_rotation(Vector4f vec4)
{
    Quaternion local;
    //////////////////////////////////////
    local.axis.w =          cos(vec4.w/2);
    local.axis.x = vec4.x * sin(vec4.w/2);
    local.axis.y = vec4.y * sin(vec4.w/2);
    local.axis.z = vec4.z * sin(vec4.w/2);
    //////////////////////////////////////
    return local;
}

Quaternion Quaternion::get_local_rotation(float w, float x, float y, float z)
{
    Quaternion local(1, 0, 0, 0);
    ////////////////////////////
    local.axis.w =     cos(w/2);
    local.axis.x = x * sin(w/2);
    local.axis.y = y * sin(w/2);
    local.axis.z = z * sin(w/2);
    ////////////////////////////
    return local;
}

Matrix4f Quaternion::get_quaternion_matrix()
{
    mat4x4 mat4;
    //column 1
    ///////////////////////////////////////////////////////////////////////
    mat4[0] = axis.w * axis.w + axis.x * axis.x - axis.y * axis.y - axis.z * axis.z;
    mat4[1] = 2 * axis.x * axis.y + 2 * axis.w * axis.z;
    mat4[2] = 2 * axis.x * axis.z - 2 * axis.w * axis.y;
    mat4[3] = 0;
    ///////////////////////////////////////////////////////////////////////

    //column 2
    ////////////////////////////////////////////////////////////////////////////////
    mat4[4] = 2 * axis.x * axis.y - 2 * axis.w * axis.z;
    mat4[5] = axis.w * axis.w - axis.x * axis.x + axis.y * axis.y - axis.z * axis.z;
    mat4[6] = 2 * axis.y * axis.z - 2 * axis.w * axis.x;
    mat4[7] = 0;
    ////////////////////////////////////////////////////////////////////////////////

    //column 3
    ////////////////////////////////////////////////////////////////////////
    mat4[8]  = 2 * axis.x * axis.z + 2 * axis.w * axis.y;
    mat4[9]  = 2 * axis.y * axis.z + 2 * axis.w * axis.x;
    mat4[10] = axis.w * axis.w - axis.x * axis.x - axis.y * axis.y + axis.z * axis.z;
    mat4[11] = 0;
    ////////////////////////////////////////////////////////////////////////

    //column 4
    /////////////
    mat4[12] = 0;
    mat4[13] = 0;
    mat4[14] = 0;
    mat4[15] = 1;
    /////////////

    mat4.transpose();

    return mat4;
}