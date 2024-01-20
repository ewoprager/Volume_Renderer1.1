#ifndef vec<3>_hpp
#define vec<3>_hpp

#include <iostream>
#include <cmath>

#include "vec<2>.h"

class vec<3>;

vec<3> V3_Normalised(const vec<3>& v);
vec<3> V3_Cross(const vec<3>& v1, const vec<3>& v2);
vec<3> operator|(const vec<2> &v2, const float &_z);
vec<3> operator|(const float &_x, const vec<2> &v2);

class vec<3> {
public:
	friend vec<3> operator+(const vec<3>& v1, const vec<3>& v2) {
		return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
	}
	friend vec<3> operator-(const vec<3>& v1, const vec<3>& v2) {
		return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
	}
	friend vec<3> operator*(const vec<3>& v1, const vec<3>& v2){
		return {v1.x*v2.x, v1.y*v2.y, v1.z*v2.z};
	}
	friend vec<3> operator*(const vec<3>& v, const float& a){
		return {a*v.x, a*v.y, a*v.z};
	}
	friend vec<3> operator*(const float& a, const vec<3>& v){
		return {a*v.x, a*v.y, a*v.z};
	}
	friend vec<3> operator/(const vec<3>& v, const float& a){
		const float aInv = 1.0f/a;
		return {v.x*aInv, v.y*aInv, v.z*aInv};
	}
	friend vec<3> operator/(const float& a, const vec<3>& v){
		return {a/v.x, a/v.y, a/v.z};
	}
	vec<3> operator-(){
		return {-x, -y, -z};
	}
	vec<3>& operator+=(const vec<3>& v){
		*this = *this + v;
		return *this;
	}
	vec<3>& operator-=(const vec<3>& v){
		*this = *this - v;
		return *this;
	}
	vec<3>& operator*=(const float& a){
		*this = *this * a;
		return *this;
	}
	vec<3>& operator/=(const float& a){
		*this = *this / a;
		return *this;
	}
	vec<3> &operator()(float (*const &func)(void)){
		x = func(); y = func(); z = func();
		return *this;
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const vec<3>& v){
		stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return stream;
	}
	
	friend float V3_Dot(const vec<3> &v1, const vec<3> &v2){
		return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
	}
	
	vec<3>& Normalise(){
		*this /= sqrt(SqMag());
		return *this;
	}
	
	friend vec<3> V3_Normalised(const vec<3>& v){
		return v / sqrt(v.SqMag());
	}
	
	friend vec<3> V3_Cross(const vec<3>& v1, const vec<3>& v2){
		return {v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x};
	}
	
	float SqMag() const {
		return x*x + y*y + z*z;
	}
	
	vec<2> xy() const { return {x, y}; }
	vec<2> yx() const { return {y, x}; }
	vec<2> xz() const { return {x, z}; }
	
	friend vec<3> operator|(const vec<2> &v2, const float &_z){ return {v2.x, v2.y, _z}; }
	friend vec<3> operator|(const float &_x, const vec<2> &v2){ return {_x, v2.x, v2.y}; }
	
	float x, y, z;
};

#endif /* vec<3>_hpp */
