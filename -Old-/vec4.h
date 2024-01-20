#ifndef vec<4>_hpp
#define vec<4>_hpp

#include <iostream>
#include <cmath>

#include "vec<2>.h"
#include "vec<3>.h"

class vec<4>;

vec<4> V4_Normalised(const vec<4>& v);
vec<4> operator|(const vec<3> &v3, const float &w);
vec<4> operator|(const float &_x, const vec<3> &v3);
vec<4> operator|(const vec<2> &v2a, const vec<2> &v2b);

class vec<4> {
public:
	friend vec<4> operator+(const vec<4>& v1, const vec<4>& v2) {
		return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w};
	}
	friend vec<4> operator-(const vec<4>& v1, const vec<4>& v2) {
		return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w};
	}
	friend vec<4> operator*(const vec<4>& v1, const vec<4>& v2){
		return {v1.x*v2.x, v1.y*v2.y, v1.z*v2.z, v1.w*v2.w};
	}
	friend vec<4> operator*(const vec<4>& v, const float& a){
		return {a*v.x, a*v.y, a*v.z, a*v.w};
	}
	friend vec<4> operator*(const float& a, const vec<4>& v){
		return {a*v.x, a*v.y, a*v.z, a*v.w};
	}
	friend vec<4> operator/(const vec<4>& v, const float& a){
		return {v.x/a, v.y/a, v.z/a, v.w/a};
	}
	vec<4> operator-(){
		return {-x, -y, -z, -w};
	}
	vec<4>& operator+=(const vec<4>& v){
		*this = *this + v;
		return *this;
	}
	vec<4>& operator-=(const vec<4>& v){
		*this = *this - v;
		return *this;
	}
	vec<4>& operator*=(const float& a){
		*this = *this * a;
		return *this;
	}
	vec<4>& operator/=(const float& a){
		*this = *this / a;
		return *this;
	}
	vec<4> &operator()(float (*const &func)(void)){
		x = func(); y = func(); z = func(); w = func();
		return *this;
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const vec<4>& v){
		stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
		return stream;
	}
	float& operator[](const int& index){
		if(index == 0) return x;
		if(index == 1) return y;
		if(index == 2) return z;
		if(index == 3) return w;
		std::cout << "ERROR: vec<4> index out of range." << std::endl;
		return x;
	}
	
	friend float V4_Dot(const vec<4> &v1, const vec<4> &v2){
		return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w;
	}
	
	vec<4>& Normalise(){
		*this /= sqrt(SqMag());
		return *this;
	}
	friend vec<4> V4_Normalised(const vec<4>& v){
		return v / sqrt(v.SqMag());
	}
	
	float SqMag() const {
		return x*x + y*y + z*z + w*w;
	}
	
	vec<2> xy() const { return {x, y}; }
	vec<2> xz() const { return {x, z}; }
	vec<2> xw() const { return {x, w}; }
	vec<2> yz() const { return {y, z}; }
	vec<2> yw() const { return {y, w}; }
	vec<2> zw() const { return {z, w}; }
	
	vec<3> xyz() const { return {x, y, z}; }
	vec<3> xyw() const { return {x, y, w}; }
	vec<3> xzw() const { return {x, z, w}; }
	vec<3> yzw() const { return {y, z, w}; }
	
	friend vec<4> operator|(const vec<3> &v3, const float &_w){ return {v3.x, v3.y, v3.z, _w}; }
	friend vec<4> operator|(const float &_x, const vec<3> &v3){ return {_x, v3.x, v3.y, v3.z}; }
	friend vec<4> operator|(const vec<2> &v2a, const vec<2> &v2b){ return {v2a.x, v2a.y, v2b.x, v2b.y}; }
	
	float x, y, z, w;
};

#endif /* vec<4>_hpp */
