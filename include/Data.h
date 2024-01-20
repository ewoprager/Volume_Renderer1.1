#ifndef Data_hpp
#define Data_hpp

#include "Header.h"
#include "Render.h"
#include "Mask.h"

#define THRESHOLD_CT_DATA

class IData {
public:
	virtual void InitGLComponents() = 0;
	virtual void MakeTexture(bool autoWindow=false) const = 0;
	
protected:
	GLuint textureHandle;
	bool glInitialised = false;
};

class CTData : public IData {
public:
	void InitGLComponents() override;
	void MakeTexture(bool autoWindow=false) const override; // requires prior initialisation of GL components
	
	GLuint TextureHandle() const { return textureHandle; }
	uint16_t Width() const { return width; }
	uint16_t Height() const { return height; }
	uint16_t Depth() const { return depth; }
	const vec<3> &Spacing() const { return spacing; }
	float DirVecLength() const { return dirVecLength; }
	
protected:
	uint16_t width, height, depth;
	vec<3> spacing;
	float dirVecLength;
	
	int16_t *data = nullptr;
	
	const float backgroundColour[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};

class CTDicomData : public CTData {
public:
	void Load(const char *filename, bool &leftPresent_out, bool &rightPresent_out, char *datasetName_out);
	
	const mat<4, 4> &C1Inverse() const { return matrixC1inverse; }
	const mat<4, 4> &C4() const { return matrixC4; }
	
private:
	mat<4, 4> matrixC1inverse;
	mat<4, 4> matrixC4;
};

class XRData : public IData {
public:
	void Load(int _width, int _height, const vec<2> &_spacing, uint16_t *_data);
	void InitGLComponents() override;
	void MakeTexture(bool autoWindow=false) const override; // requires prior initialisation of GL components
	
	GLuint TextureHandle() const { return textureHandle; }
	uint16_t Width() const { return width; }
	uint16_t Height() const { return height; }
	const vec<2> &Spacing() const { return spacing; }
	const vec<3> &SourcePosition() const { return sourcePosition; }
	
	Array2D<uint16_t> ToArrayWithRect(const SDL_Rect &rect) const;
	
	void FlipY();
	
protected:
	uint16_t width, height;
	vec<2> spacing;
	vec<3> sourcePosition;
	
	uint16_t *data = nullptr;
	
	const float backgroundColour[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

class XRDicomData : public XRData {
public:
	void Load(const char *filename);
	
	const mat<4, 4> &C2() const { return matrixC2; }
	const mat<4, 4> &P() const { return matrixP; }
	const mat<4, 4> &C3() const { return matrixC3; }
	const mat<4, 4> &B2() const { return matrixB2; }
	
	void UpdateSourceOffset(const vec<2> &newSourceOffset);
	
private:
	mat<4, 4> matrixC2;
	mat<4, 4> matrixP;
	mat<4, 4> matrixC3;
	mat<4, 4> matrixB2;
};

class OtherData {
public:
	void Load(bool _leftPresent,
			  bool _rightPresent,
			  const char *fn_points3d,
			  const char *fn_points2d,
			  float _enlargement,
			  const char *fn_ocLeft,
			  const char *fn_ocRight);
	~OtherData();
	
	const Render::DrawArray &Points3d() const { return *points3d; }
	const Render::DrawArray &P3DTransformed() const { return *p3dTransformed; }
	const Render::DrawArray *P3DEnlargedLeft() const { return p3dEnlargedLeft; }
	const Render::DrawArray *P3DEnlargedRight() const { return p3dEnlargedRight; }
	const Render::DrawArray &Points2d() const { return *points2d; }
	const Render::DrawElementArray *LeftOC() const { return leftOticCapsule; }
	const Render::DrawElementArray *RightOC() const { return rightOticCapsule; }
	
	void Transform3dPoints(const vec<3> &pan, const mat<4, 4> &rotationMatrix/*, const vec<2> &sourceOffset*/) const;
	void CalcEnlargedPointPositions() const;
	bool OutsideEnlarged() const;
	
	void WriteElectrodeRays(const vec<3> &pan, const mat<4, 4> &rotationMatrix, const char *filename) const;
	
private:
	// point data:
	Render::DrawArray *points3d;
	Render::DrawArray *p3dTransformed;
	uint32_t n3d;
	const int subSampleEnlarged = 1;
	int nEnlarged;
	float enlargement;
	
	bool leftPresent, rightPresent;
	
	Render::DrawArray *p3dEnlargedLeft=nullptr;
	Render::DrawArray *p3dEnlargedRight=nullptr;
	
	Render::DrawArray *points2d;
	uint32_t n2d;
	Render::DrawElementArray *leftOticCapsule=nullptr;
	Render::DrawElementArray *rightOticCapsule=nullptr;
};

Params ReadInitialAlignment(const char *filename);
GLfloat *ReadPoints(const char *filename, uint32_t &d_out, uint32_t &n_out);

#endif /* Data_hpp */
