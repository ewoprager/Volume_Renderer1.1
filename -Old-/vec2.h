#ifndef vec<2>_hpp
#define vec<2>_hpp

#include <iostream>
#include <cmath>

class vec<2>;

vec<2> V2_Normalised(const vec<2>& v);
vec<2> V2_Crossed(const vec<2>& v);
vec<2> V2_DirectionUnit(const double& angle);
vec<2> V2_RandomUnit();

class vec<2> {
public:
	friend vec<2> operator+(const vec<2>& v1, const vec<2>& v2) {
		return {v1.x + v2.x, v1.y + v2.y};
	}
	friend vec<2> operator-(const vec<2>& v1, const vec<2>& v2) {
		return {v1.x - v2.x, v1.y - v2.y};
	}
	friend vec<2> operator*(const vec<2>& v1, const vec<2>& v2){
		return {v1.x*v2.x, v1.y*v2.y};
	}
	friend vec<2> operator*(const vec<2>& v, const float& a){
		return {a*v.x, a*v.y};
	}
	friend vec<2> operator*(const float& a, const vec<2>& v){
		return {a*v.x, a*v.y};
	}
	friend vec<2> operator/(const vec<2>& v1, const vec<2>& v2){
		return {v1.x/v2.x, v1.y/v2.y};
	}
	friend vec<2> operator/(const vec<2>& v, const float& a){
		const float aInv = 1.0f/a;
		return {v.x*aInv, v.y*aInv};
	}
	friend vec<2> operator/(const float& a, const vec<2>& v){
		return {a/v.x, a/v.y};
	}
	vec<2> operator-(){
		return {-x, -y};
	}
	vec<2>& operator+=(const vec<2>& v){
		*this = *this + v;
		return *this;
	}
	vec<2>& operator-=(const vec<2>& v){
		*this = *this - v;
		return *this;
	}
	vec<2>& operator*=(const float& a){
		*this = *this * a;
		return *this;
	}
	vec<2>& operator*=(const vec<2>& v){
		*this = *this * v;
		return *this;
	}
	vec<2>& operator/=(const float& a){
		*this = *this / a;
		return *this;
	}
	vec<2> &operator()(float (*const &func)(void)){
		x = func(); y = func();
		return *this;
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const vec<2>& v){
		stream << "(" << v.x << ", " << v.y << ")";
		return stream;
	}
	
	vec<2>& Normalise(){
		*this /= sqrt(SqMag());
		return *this;
	}
	friend vec<2> V2_Normalised(const vec<2>& v){
		return v / sqrt(v.SqMag());
	}
	
	friend float V2_Dot(const vec<2> &v1, const vec<2> &v2){
		return v1.x*v2.x + v1.y*v2.y;
	}
	
	vec<2>& Cross(){
		const float tmp = y;
		y = -x;
		x = tmp;
		return *this;
	}
	friend vec<2> V2_Crossed(const vec<2>& v){
		return {v.y, -v.x};
	}
	float SqMag() const {
		return x*x + y*y;
	}
	
	friend vec<2> V2_RandomUnit(){
		const float outX = -1.0f + (float)(rand() % 10001) * 0.0002f;
		return {outX, sqrt(1.0f - outX*outX)};
	}
	friend vec<2> V2_DirectionUnit(const double& angle){
		return {(float)cos(angle), -(float)sin(angle)};
	}
	double Angle() const {
		return atan2(-(double)y, (double)x);
	}
	vec<2> ComponentRelativeAngle(const double& angle) const {
		vec<2> dHat = V2_DirectionUnit(Angle() + angle);
		return dHat * V2_Dot(*this, dHat);
	}
	
	float x, y;
};

#endif /* vec<2>_hpp */
