#ifndef Header_h
#define Header_h

#include <ESDL/ESDL_OpenGL.h>

#include "mattresses.h"

//#define SKIP_OPTIMISE
#define MAX_DATASET_NAME_LENGTH 10

// classes
class VData;
class CTData;
class CTDicomData;
class XRData;
class XRDicomData;
class OtherData;

class Mask;

struct Params;

enum class DrawMode;
class View;

namespace Render {
	struct DrawParameter;
	struct DrawArray;
	struct DrawElementArray;
}

#ifndef SKIP_OPTIMISE
namespace Optimise {
	struct OpVals;
	struct PSParams;
	struct Particle;
}
#endif

// registration parameters
struct Params {
	vec<3> pan;
	vec<3> rotation;
	vec<2> sourceOffset;
	
	friend Params operator+(const Params &a, const Params &b){ return {a.pan + b.pan, a.rotation + b.rotation, a.sourceOffset + b.sourceOffset}; }
	friend Params operator-(const Params &a, const Params &b){ return {a.pan - b.pan, a.rotation - b.rotation, a.sourceOffset - b.sourceOffset}; }
	friend Params operator*(const Params &a, const Params &b){ return {a.pan*b.pan, a.rotation*b.rotation, a.sourceOffset*b.sourceOffset}; }
	friend Params operator*(const Params &p, float f){ return {p.pan*f, p.rotation*f, p.sourceOffset*f}; }
	friend Params operator*(float f, const Params &p){ return {f*p.pan, f*p.rotation, f*p.sourceOffset}; }
	friend Params operator/(const Params &p, float f){ return {p.pan/f, p.rotation/f, p.sourceOffset/f}; }
	
	Params &operator+=(const Params &o){ *this = *this + o; return *this; }
	Params &operator-=(const Params &o){ *this = *this - o; return *this; }
	Params &operator*=(const Params &o){ *this = *this*o; return *this; }
	Params &operator*=(float f){ *this = *this*f; return *this; }
	Params &operator/=(float f){ *this = *this/f; return *this; }
	
	Params &operator()(float (*func)(void)){
//		pan(func);
//		rotation(func);
//		sourceOffset(func);
		return *this;
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const Params& v){
		return stream << v.pan << ", " << v.rotation << ", " << v.sourceOffset;
	}
	
	void Print() const;
};

// instance getters
const CTDicomData &CT();
const XRDicomData &XRay();
const OtherData &Data();
const View &ViewManager();
const Mask &XRayMask();

// instance modifiers
void UpdateSourceOffset(const vec<2> &newSourceOffset);
void SetViewParams(const Params &newParams);

#endif /* Header_h */
