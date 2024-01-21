#ifndef Tools_hpp
#define Tools_hpp

#include <math.h>
#include <sys/time.h>
#include <cassert>

#include <ESDL/ESDL_OpenGL.h>

unsigned long UTime();

GLsizei GLTypeSize(const GLenum &glType);

GLenum TypePLYtoGL(const char *name);

void Flip2DByteArrayVertically(uint8_t *array, int width, int height);

float Uniform(float lb, float ub, float prec=10000.0f);

template <typename T, typename intType> T IntPower(const T &base, const intType &power){
	if(power == 0) return (T)1;
	if(power == 1) return base;
	if(power % 2) return base*IntPower<T, intType>(base*base, power/2);
	else return IntPower<T, intType>(base*base, power/2);
}

struct KahanSum {
public:
	KahanSum(const double &_value=0.0, const double &_error=0.0) : value(_value), error(_error) {}
	
	const double &Value() const { return value; }
	const double &Error() const { return error; }
	operator double() const { return value - error; }
	
	const KahanSum &operator=(const double &d){
		value = d;
		error = 0.0;
		return *this;
	}
	const KahanSum &operator+=(const double &d){
		const double y = d - error;
		const double t = value + y;
		error = (t - value) - y;
		value = t;
		return *this;
	}
	const KahanSum &operator-=(const double &d){
		const double y = - d - error;
		const double t = value + y;
		error = (t - value) - y;
		value = t;
		return *this;
	}
	friend KahanSum operator+(const double &d, const KahanSum &ks){
		const double y = d - ks.error;
		const double t = ks.value + y;
		return KahanSum(t, (t - ks.value) - y);
	}
	
private:
	double value, error;
};

template <typename T> struct Array2D {
public:
	Array2D(const T *_start, unsigned int _width, unsigned int _height) : start(_start), width(_width), height(_height), iJump(1), jJump(_width) {}
	Array2D(const T *_start, unsigned int _width, unsigned int _height, int _iJump, int _jJump) : start(_start), width(_width), height(_height), iJump(_iJump), jJump(_jJump) {}
	
	unsigned int Width() const { return width; }
	unsigned int Height() const { return height; }
	int IJump() const { return iJump; }
	int JJump() const { return jJump; }
	
	const T &operator()(unsigned int j, unsigned int i) const {
		return start[j*jJump + i*iJump];
	}
	
private:
	const T *const start;
	unsigned int width, height;
	int iJump, jJump;
};

#endif /* Tools_hpp */
