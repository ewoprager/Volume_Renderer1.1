#ifndef mat4x4_hpp
#define mat4x4_hpp

#include <iostream>
#include <math.h>
#include "vec<3>.h"
#include "vec<4>.h"

float Sign(const float& x);
void M4x4_Identity(float out[4][4]);
void M4x4_Frustum(const float& left, const float& right, const float& bottom, const float& top, const float& zNear, const float& zFar, float out[4][4]);
void M4x4_Perspective(const float& angleOfView, const float& aspect, const float& near, const float& far, float out[4][4]);
void M4x4_Print(const float m[4][4]);
void V4_Print(const vec<4>& vec);
void M4x4_Multiply(const float m1[4][4], const float m2[4][4], float out[4][4]);
vec<4> M4x4_Multiply(const float m[4][4], vec<4> vec);
void M4x4_PreMultiply(float matrix[4][4], const float by[4][4]);
void M4x4_PostMultiply(float matrix[4][4], const float by[4][4]);
vec<4> &M4x4_PreMultiply(vec<4>& vec, const float by[4][4]);
void M4x4_Translation(const vec<3>& translation, float out[4][4]);
void M4x4_Transpose(const float matrix[4][4], float out[4][4]);
void M4x4_Inverse(const float matrix[4][4], float out[4][4]);
void M4x4_LookAt(const vec<3>& pos, const vec<3>& target, const vec<3>& up, float out[4][4]);
void M4x4_xRotation(const float& angle, float out[4][4]);
void M4x4_yRotation(const float& angle, float out[4][4]);
void M4x4_zRotation(const float& angle, float out[4][4]);
void M4x4_AxisRotation(const float& theta, const vec<3>& axis, float out[4][4]); // rotates about the given axis, clockwise looking in the direction of the axis vector
void M4x4_Scaling(const vec<3>& scaling, float out[4][4]);
void M4x4_ScaleExcludingTranslation(float matrix[4][4], const vec<3>& scaling);
void M4x4_ModifyProjectionNearClippingPlane(float projectionMatrix[4][4], const vec<4>& clipPlane);
vec<3> ComponentRelativeAngle(const vec<3> v, const float& roll, const float& outAngle);
void M4x4_Extract3x3(const float in[4][4], float out[3][3]);
void M4x4_Extend3x3(const float in[3][3], float out[4][4]);

#endif /* mat4x4_hpp */
