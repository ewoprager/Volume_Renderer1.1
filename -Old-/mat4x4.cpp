#include "mat4x4.h"

float Sign(const float& x){
	return (float)((x > 0.0f) - (x < 0.0f));
}

// I took this function from somewhere on the internet a long time ago and have adapted it since
void M4x4_Frustum(const float& left, const float& right, const float& bottom, const float& top, const float& zNear, const float& zFar, float out[4][4]){
	const float zDelta = zFar - zNear;
	const float dir = right - left;
	const float height = top - bottom;
	const float zNear2 = 2.0f * zNear;
	
	out[0][0] = zNear2 / dir;
	out[0][1] = 0;
	out[0][2] = 0;
	out[0][3] = 0;
	out[1][0] = 0;
	out[1][1] = zNear2 / height;
	out[1][2] = 0;
	out[1][3] = 0;
	out[2][0] = (right + left) / dir;
	out[2][1] = (top + bottom) / height;
	out[2][2] = -(zFar + zNear) / zDelta;
	out[2][3] = - 1.0f;
	out[3][0] = 0;
	out[3][1] = 0;
	out[3][2] = - zFar * zNear2 / zDelta;
	out[3][3] = 0;
}

void M4x4_Perspective(const float& angleOfView, const float& aspect, const float& near, const float& far, float out[4][4]) {
	const float f = tan(0.5f * (M_PI - angleOfView));
	const float rangeInv = 1.0f / (near - far);

	out[0][0] = f / aspect;	out[0][1] = 0.0f;	out[0][2] = 0.0f;							out[0][3] = 0.0f;
	out[1][0] = 0.0f;		out[1][1] = f;		out[1][2] = 0.0f;							out[1][3] = 0.0f;
	out[2][0] = 0.0f;		out[2][1] = 0.0f;	out[2][2] = (near + far) * rangeInv;		out[2][3] = -1.0f;
	out[3][0] = 0.0f;		out[3][1] = 0.0f;	out[3][2] = near * far * rangeInv * 2.0f;	out[3][3] = 0.0f;
}
void M4x4_Print(const float m[4][4]){
	std::cout << "\n";
	std::cout << m[0][0] << " " << m[1][0] << " " << m[2][0] << " " << m[3][0] << "\n";
	std::cout << m[0][1] << " " << m[1][1] << " " << m[2][1] << " " << m[3][1] << "\n";
	std::cout << m[0][2] << " " << m[1][2] << " " << m[2][2] << " " << m[3][2] << "\n";
	std::cout << m[0][3] << " " << m[1][3] << " " << m[2][3] << " " << m[3][3] << "\n";
	std::cout << "\n";
}
void V4_Print(const vec<4>& vec){
	std::cout << "\n";
	std::cout << vec.x << "\n";
	std::cout << vec.y << "\n";
	std::cout << vec.z << "\n";
	std::cout << vec.w << "\n";
	std::cout << "\n";
}
void M4x4_Multiply(const float m1[4][4], const float m2[4][4], float out[4][4]){
	for(int r=0; r<4; r++){
		for(int c=0; c<4; c++){
			float sum = 0.0f;
			for(int i=0; i<4; i++) sum += m1[i][r]*m2[c][i];
			out[c][r] = sum;
		}
	}
}
vec<4> M4x4_Multiply(const float m[4][4], vec<4> vec){
	vec<4> out;
	for(int r=0; r<4; r++){
		float sum = 0.0f;
		for(int i=0; i<4; i++) sum += m[i][r] * vec[i];
		out[r] = sum;
	}
	return out;
}
void M4x4_PreMultiply(float matrix[4][4], const float by[4][4]){
	float temp[4][4];
	memcpy(temp, matrix, 16 * sizeof(float));
	M4x4_Multiply(by, temp, matrix);
}
void M4x4_PostMultiply(float matrix[4][4], const float by[4][4]){
	float temp[4][4];
	memcpy(temp, matrix, 16 * sizeof(float));
	M4x4_Multiply(temp, by, matrix);
}
vec<4> &M4x4_PreMultiply(vec<4> &vec, const float by[4][4]){
	return vec = M4x4_Multiply(by, vec);
}

void M4x4_AxisRotation(const float& theta, const vec<3>& axis, float out[4][4]){ // rotates about the given axis, clockwise looking in the direction of the axis vector
	const float c = cos(theta);
	const float s = sin(theta);
	const float cm = 1.0f - c;
	out[0][0] = c + axis.x*axis.x*cm;			out[1][0] = axis.x*axis.y*cm - axis.z*s;	out[2][0] = axis.x*axis.z*cm + axis.y*s;	out[3][0] = 0.0f;
	out[0][1] = axis.y*axis.x*cm + axis.z*s;	out[1][1] = c + axis.y*axis.y*cm;			out[2][1] = axis.y*axis.z*cm - axis.x*s;	out[3][1] = 0.0f;
	out[0][2] = axis.z*axis.x*cm - axis.y*s;	out[1][2] = axis.z*axis.y*cm + axis.x*s;	out[2][2] = c + axis.z*axis.z*cm;			out[3][2] = 0.0f;
	out[0][3] = 0.0f;							out[1][3] = 0.0f;							out[2][3] = 0.0f;							out[3][3] = 1.0f;
}

void M4x4_Translation(const vec<3>& translation, float out[4][4]){
	out[0][0] = 1.0f;	out[1][0] = 0.0f;	out[2][0] = 0.0f;	out[3][0] = translation.x;
	out[0][1] = 0.0f;	out[1][1] = 1.0f;	out[2][1] = 0.0f;	out[3][1] = translation.y;
	out[0][2] = 0.0f;	out[1][2] = 0.0f;	out[2][2] = 1.0f;	out[3][2] = translation.z;
	out[0][3] = 0.0f;	out[1][3] = 0.0f;	out[2][3] = 0.0f;	out[3][3] = 1.0f;
}

void M4x4_Transpose(const float matrix[4][4], float out[4][4]){
	for(int r=0; r<4; r++) for(int c=0; c<4; c++) out[c][r] = matrix[r][c];
}

// I took this function from somewhere on the internet a long time ago and have adapted it since
void M4x4_Inverse(const float matrix[4][4], float out[4][4]){
	const float m00 = matrix[0][0];
	const float m01 = matrix[0][1];
	const float m02 = matrix[0][2];
	const float m03 = matrix[0][3];
	const float m10 = matrix[1][0];
	const float m11 = matrix[1][1];
	const float m12 = matrix[1][2];
	const float m13 = matrix[1][3];
	const float m20 = matrix[2][0];
	const float m21 = matrix[2][1];
	const float m22 = matrix[2][2];
	const float m23 = matrix[2][3];
	const float m30 = matrix[3][0];
	const float m31 = matrix[3][1];
	const float m32 = matrix[3][2];
	const float m33 = matrix[3][3];
	const float tmp_0  = m22 * m33;
	const float tmp_1  = m32 * m23;
	const float tmp_2  = m12 * m33;
	const float tmp_3  = m32 * m13;
	const float tmp_4  = m12 * m23;
	const float tmp_5  = m22 * m13;
	const float tmp_6  = m02 * m33;
	const float tmp_7  = m32 * m03;
	const float tmp_8  = m02 * m23;
	const float tmp_9  = m22 * m03;
	const float tmp_10 = m02 * m13;
	const float tmp_11 = m12 * m03;
	const float tmp_12 = m20 * m31;
	const float tmp_13 = m30 * m21;
	const float tmp_14 = m10 * m31;
	const float tmp_15 = m30 * m11;
	const float tmp_16 = m10 * m21;
	const float tmp_17 = m20 * m11;
	const float tmp_18 = m00 * m31;
	const float tmp_19 = m30 * m01;
	const float tmp_20 = m00 * m21;
	const float tmp_21 = m20 * m01;
	const float tmp_22 = m00 * m11;
	const float tmp_23 = m10 * m01;

	const float t0 = (tmp_0 * m11 + tmp_3 * m21 + tmp_4 * m31) - (tmp_1 * m11 + tmp_2 * m21 + tmp_5 * m31);
	const float t1 = (tmp_1 * m01 + tmp_6 * m21 + tmp_9 * m31) - (tmp_0 * m01 + tmp_7 * m21 + tmp_8 * m31);
	const float t2 = (tmp_2 * m01 + tmp_7 * m11 + tmp_10 * m31) - (tmp_3 * m01 + tmp_6 * m11 + tmp_11 * m31);
	const float t3 = (tmp_5 * m01 + tmp_8 * m11 + tmp_11 * m21) - (tmp_4 * m01 + tmp_9 * m11 + tmp_10 * m21);

	const float d = 1.0 / (m00 * t0 + m10 * t1 + m20 * t2 + m30 * t3);

	out[0][0] = d * t0;
	out[0][1] = d * t1;
	out[0][2] = d * t2;
	out[0][3] = d * t3;
	
	out[1][0] = d * ((tmp_1 * m10 + tmp_2 * m20 + tmp_5 * m30) - (tmp_0 * m10 + tmp_3 * m20 + tmp_4 * m30));
	out[1][1] = d * ((tmp_0 * m00 + tmp_7 * m20 + tmp_8 * m30) - (tmp_1 * m00 + tmp_6 * m20 + tmp_9 * m30));
	out[1][2] = d * ((tmp_3 * m00 + tmp_6 * m10 + tmp_11 * m30) - (tmp_2 * m00 + tmp_7 * m10 + tmp_10 * m30));
	out[1][3] = d * ((tmp_4 * m00 + tmp_9 * m10 + tmp_10 * m20) - (tmp_5 * m00 + tmp_8 * m10 + tmp_11 * m20));
	
	out[2][0] = d * ((tmp_12 * m13 + tmp_15 * m23 + tmp_16 * m33) - (tmp_13 * m13 + tmp_14 * m23 + tmp_17 * m33));
	out[2][1] = d * ((tmp_13 * m03 + tmp_18 * m23 + tmp_21 * m33) - (tmp_12 * m03 + tmp_19 * m23 + tmp_20 * m33));
	out[2][2] = d * ((tmp_14 * m03 + tmp_19 * m13 + tmp_22 * m33) - (tmp_15 * m03 + tmp_18 * m13 + tmp_23 * m33));
	out[2][3] = d * ((tmp_17 * m03 + tmp_20 * m13 + tmp_23 * m23) - (tmp_16 * m03 + tmp_21 * m13 + tmp_22 * m23));
	
	out[3][0] = d * ((tmp_14 * m22 + tmp_17 * m32 + tmp_13 * m12) - (tmp_16 * m32 + tmp_12 * m12 + tmp_15 * m22));
	out[3][1] = d * ((tmp_20 * m32 + tmp_12 * m02 + tmp_19 * m22) - (tmp_18 * m22 + tmp_21 * m32 + tmp_13 * m02));
	out[3][2] = d * ((tmp_18 * m12 + tmp_23 * m32 + tmp_15 * m02) - (tmp_22 * m32 + tmp_14 * m02 + tmp_19 * m12));
	out[3][3] = d * ((tmp_22 * m22 + tmp_16 * m02 + tmp_21 * m12) - (tmp_20 * m12 + tmp_23 * m22 + tmp_17 * m02));
}

void M4x4_Identity(float out[4][4]){
	out[0][0] = 1.0f; out[1][0] = 0.0f; out[2][0] = 0.0f; out[3][0] = 0.0f;
	out[0][1] = 0.0f; out[1][1] = 1.0f; out[2][1] = 0.0f; out[3][1] = 0.0f;
	out[0][2] = 0.0f; out[1][2] = 0.0f; out[2][2] = 1.0f; out[3][2] = 0.0f;
	out[0][3] = 0.0f; out[1][3] = 0.0f; out[2][3] = 0.0f; out[3][3] = 1.0f;
}

void M4x4_LookAt(const vec<3>& pos, const vec<3>& target, const vec<3>& up, float out[4][4]){
	const vec<3> zAxis = V3_Normalised(pos - target);
	const vec<3> xAxis = V3_Normalised(V3_Cross(up, zAxis));
	const vec<3> yAxis = V3_Normalised(V3_Cross(zAxis, xAxis));
	out[0][0] = xAxis.x;	out[0][1] = xAxis.y;	out[0][2] = xAxis.z;	out[0][3] = 0.0f;
	out[1][0] = yAxis.x;	out[1][1] = yAxis.y;	out[1][2] = yAxis.z;	out[1][3] = 0.0f;
	out[2][0] = zAxis.x;	out[2][1] = zAxis.y;	out[2][2] = zAxis.z;	out[2][3] = 0.0f;
	out[3][0] = pos.x;		out[3][1] = pos.y;		out[3][2] = pos.z;		out[3][3] = 1.0f;
}

// rotations are right-hand sense:
void M4x4_xRotation(const float& angle, float out[4][4]){
	const float c = cosf(angle);
	const float s = sinf(angle);
	out[0][0] = 1.0f;	out[1][0] = 0.0f;	out[2][0] = 0.0f;	out[3][0] = 0.0f;
	out[0][1] = 0.0f;	out[1][1] = c;		out[2][1] = -s;		out[3][1] = 0.0f;
	out[0][2] = 0.0f;	out[1][2] = s;		out[2][2] = c;		out[3][2] = 0.0f;
	out[0][3] = 0.0f;	out[1][3] = 0.0f;	out[2][3] = 0.0f;	out[3][3] = 1.0f;
}
void M4x4_yRotation(const float& angle, float out[4][4]){
	const float c = cosf(angle);
	const float s = sinf(angle);
	out[0][0] = c;		out[1][0] = 0.0f;	out[2][0] = s;		out[3][0] = 0.0f;
	out[0][1] = 0.0f;	out[1][1] = 1.0f;	out[2][1] = 0.0f;	out[3][1] = 0.0f;
	out[0][2] = -s;		out[1][2] = 0.0f;	out[2][2] = c;		out[3][2] = 0.0f;
	out[0][3] = 0.0f;	out[1][3] = 0.0f;	out[2][3] = 0.0f;	out[3][3] = 1.0f;
}
void M4x4_zRotation(const float& angle, float out[4][4]){
	const float c = cosf(angle);
	const float s = sinf(angle);
	out[0][0] = c;		out[1][0] = -s;		out[2][0] = 0.0f;	out[3][0] = 0.0f;
	out[0][1] = s;		out[1][1] = c;		out[2][1] = 0.0f;	out[3][1] = 0.0f;
	out[0][2] = 0.0f;	out[1][2] = 0.0f;	out[2][2] = 1.0f;	out[3][2] = 0.0f;
	out[0][3] = 0.0f;	out[1][3] = 0.0f;	out[2][3] = 0.0f;	out[3][3] = 1.0f;
}

void M4x4_Scaling(const vec<3>& scaling, float out[4][4]){
	out[0][0] = scaling.x;	out[0][1] = 0.0f;		out[0][2] = 0.0f;		out[0][3] = 0.0f;
	out[1][0] = 0.0f;		out[1][1] = scaling.y;	out[1][2] = 0.0f;		out[1][3] = 0.0f;
	out[2][0] = 0.0f;		out[2][1] = 0.0f;		out[2][2] = scaling.z;	out[2][3] = 0.0f;
	out[3][0] = 0.0f;		out[3][1] = 0.0f;		out[3][2] = 0.0f;		out[3][3] = 1.0f;
}
void M4x4_ScaleExcludingTranslation(float matrix[4][4], const vec<3>& scaling){
	float xSave = matrix[3][0];
	float ySave = matrix[3][1];
	float zSave = matrix[3][2];
	float m[4][4];
	M4x4_Scaling(scaling, m);
	M4x4_PreMultiply(matrix, m);
	matrix[3][0] = xSave;
	matrix[3][1] = ySave;
	matrix[3][2] = zSave;
}
void M4x4_ModifyProjectionNearClippingPlane(float projectionMatrix[4][4], const vec<4>& clipPlane){
	vec<4> q = {
		(Sign(clipPlane.x) + projectionMatrix[2][0]) / projectionMatrix[0][0],
		(Sign(clipPlane.y) + projectionMatrix[2][1]) / projectionMatrix[1][1],
		-1.0f,
		(1.0f + projectionMatrix[2][2]) / projectionMatrix[3][2]
	};
	vec<4> c = clipPlane * 2.0f / V4_Dot(clipPlane, q);
	projectionMatrix[0][2] = c.x;
	projectionMatrix[1][2] = c.y;
	projectionMatrix[2][2] = c.z + 1.0f;
	projectionMatrix[3][2] = c.w;
}

vec<3> ComponentRelativeAngle(const vec<3> v, const float& roll, const float& outAngle){
	vec<3> axis;
	if(v.x > v.y && v.x > v.z){ // start with y
		axis.y = 0.5f; // less than 1/sqrt(3) = 0.5774f
		const float a = v.z*v.z + v.x*v.x;
		const float b = axis.y*v.y*v.z;
		const float c = axis.y*axis.y*(v.x*v.x + v.y*v.y) - v.x*v.x;
		axis.z = (-b + sqrt(b*b - 4.0f*a*c))/(2.0f * a);
		axis.x = -(axis.y*v.y + axis.z*v.z)/v.x;
	} else { // start with x
		axis.x = 0.5f; // less than 1/sqrt(3) = 0.5774f
		const float a = v.z*v.z + v.y*v.y;
		const float b = axis.x*v.x*v.z;
		const float c = axis.x*axis.x*(v.x*v.x + v.y*v.y) - v.y*v.y;
		axis.z = (-b + sqrt(b*b - 4.0f*a*c))/(2.0f * a);
		axis.y = -(axis.x*v.x + axis.z*v.z)/v.y;
	}
	float outMatrix[4][4]; float rollMatrix[4][4]; float matrix[4][4];
	M4x4_AxisRotation(outAngle, axis, outMatrix);
	M4x4_AxisRotation(roll, v, rollMatrix);
	M4x4_Multiply(rollMatrix, outMatrix, matrix);
	return cos(outAngle)*M4x4_Multiply(matrix, {v.x, v.y, v.z, 1.0f}).xyz();
}

void M4x4_Extract3x3(const float in[4][4], float out[3][3]){
	memcpy(&out[0][0], &in[0][0], 3*sizeof(float));
	memcpy(&out[1][0], &in[1][0], 3*sizeof(float));
	memcpy(&out[2][0], &in[2][0], 3*sizeof(float));
}
void M4x4_Extend3x3(const float in[3][3], float out[4][4]){
	memcpy(&out[0][0], &in[0][0], 3*sizeof(float));
	memcpy(&out[1][0], &in[1][0], 3*sizeof(float));
	memcpy(&out[2][0], &in[2][0], 3*sizeof(float));
	out[3][0] = out[3][1] = out[3][2] = out[0][3] = out[1][3] = out[2][3] = 0.0f;
	out[3][3] = 1.0f;
}
