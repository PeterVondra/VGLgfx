#pragma once

#include "Math.h"
#include "Matrix.h"

class Quaternion
{
    public:
        Quaternion();
        Quaternion(float w, float x, float y, float z);

        Quaternion operator *(const Quaternion q);
        Quaternion get_local_rotation(Vector4f vec4);
        Quaternion get_local_rotation(float w, float x, float y, float z);
        Matrix4f get_quaternion_matrix();

    protected:
    private:
        Vector4f axis;
};

