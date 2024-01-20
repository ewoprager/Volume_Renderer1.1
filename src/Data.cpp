#include <fstream>
#include <map>

#include "mattresses.h"
#include "Tools.h"
#include "PLY.h"

#include "Data.h"

static const int thresh = 3250;

// ----------
// CTData
// ----------
void CTData::InitGLComponents(){
	if(glInitialised){ std::cout << "Warning: GL components already initialised.\n"; return; }
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_3D, textureHandle);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, backgroundColour);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glInitialised = true;
}
void CTData::MakeTexture(bool autoWindow) const {
	glBindTexture(GL_TEXTURE_3D, textureHandle);
	if(!autoWindow){
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, width, height, depth, 0, GL_RED, GL_SHORT, data);
		return;
	}
	
	// taking photon MeV = 1e-1
	//static const float muWater = 1.707e-1f; // cm^2 / g
	//static const float muAir = 1.541e-1f; // cm^2 / g
	//static const float a = 1e-3f*(muWater - muAir);
	
	const int N = width*height*depth;
	uint16_t *texData = (uint16_t *)malloc(N * sizeof(uint16_t));
	
#ifdef THRESHOLD_CT_DATA
	for(int i=0; i<N; i++) if(data[i] < -1000) data[i] = -1000;
#endif
	
	// rescaling data
	double min = /*a**/data[0]/* + muWater*/;
	double max = /*a**/data[0]/* + muWater*/;
	for(int i=1; i<N; i++){
		const double d = /*a**/data[i]/* + muWater*/;
		if(d < min) min = d;
		else if(d > max) max = d;
	}
	const double fac = (double)UINT16_MAX/(max - min);
	for(int i=0; i<N; i++) texData[i] = (uint16_t)(fac*(double)(/*a**/data[i]/* + muWater*/ - min));
	
	//std::cout << fac*a << ", " << fac*(muWater - min)/(double)UINT16_MAX << "\n";
	
	/*
	const int L = 20;
	const int S = 30;
	assert(width == height);
	const float deltaS = 2.0f*M_PI/S;
	const int deltaL = depth / L;
	float r = 0.495f*(float)width;
	for(int l=0; l<L; l++) for(int s=0; s<S; s++){
		const int i = (int)roundf(0.5f*(float)width + r*cosf((float)s*deltaS));
		const int j = (int)roundf(0.5f*(float)width + r*sinf((float)s*deltaS));
		const int k = deltaL*l;
		std::cout << data[width*width*k + width*j + i] << ", ";
	}
	std::cout << "\n";
	r = 0.505f*(float)width;
	for(int l=0; l<L; l++) for(int s=0; s<S; s++){
		const int i = (int)roundf(0.5f*(float)width + r*cosf((float)s*deltaS));
		const int j = (int)roundf(0.5f*(float)width + r*sinf((float)s*deltaS));
		const int k = deltaL*l;
		std::cout << data[width*width*k + width*j + i] << ", ";
	}
	std::cout << "\n";
	*/
	
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, width, height, depth, 0, GL_RED, GL_UNSIGNED_SHORT, texData);
	
	free(texData);
}

// ----------
// CTDicomData
// ----------
void CTDicomData::Load(const char *filename, bool &leftPresent_out, bool &rightPresent_out, char *datasetName_out){
	FILE *fptr = fopen(filename, "r");
	if(!fptr){ fputs("File opening error", stderr); exit(3); }
	fseek(fptr, 0, SEEK_END);
	long length = ftell(fptr); // reading length of file
	fseek(fptr, 0, SEEK_SET); // going back to the beginning of the file
	
	uint8_t leftByte, rightByte;
	
	// reading `84 + MAX_DATASET_NAME_LENGTH` byte header
	fread(&width, 2, 1, fptr);
	fread(&height, 2, 1, fptr);
	fread(&depth, 2, 1, fptr);
	fread(&spacing, 12, 1, fptr);
	fread(&matrixC1inverse, 64, 1, fptr);
	fread(&leftByte, 1, 1, fptr);
	fread(&rightByte, 1, 1, fptr);
	fread(datasetName_out, MAX_DATASET_NAME_LENGTH, 1, fptr);
	length -= 84 + MAX_DATASET_NAME_LENGTH;
	const int N = width*height*depth;
	assert(length == sizeof(int16_t)*N);
	
	leftPresent_out = (bool)leftByte; rightPresent_out = (bool)rightByte;
	
	// reallocating memory for data and reading it in
	free(data);
	data = (int16_t *)malloc(length);
	size_t result = fread(data, 1, length, fptr);
	if(result != length){ fputs("Reading error", stderr); exit(3); }
	
	fclose(fptr);
	
	std::cout << "CT data: size=[" << width << " x " << height << " x " << depth << "], spacing=" << spacing << "\nC1 inverse = " << matrixC1inverse << "\n";
	
	const vec<3> dims = spacing * (vec<3>){(float)width, (float)height, (float)depth};
	
	// C4 transformation matrix:
//	float a[4][4];
//	M4x4_Translation({-0.5f, -0.5f, -0.5f}, matrixC4);
//	M4x4_Scaling(dims, a);
//	M4x4_PreMultiply(matrixC4, a);
	
	matrixC4 = mat<4, 4>::Scaling(dims) & mat<4, 4>::Translation({-0.5f, -0.5f, -0.5f});
	
	std::cout << "C4 = " << matrixC4 << "\n";
	
	// scale of volume render vector
	dirVecLength = 2.0f*sqrt(dims.SqMag());
}

// ----------
// XRData
// ----------
void XRData::Load(int _width, int _height, const vec<2> &_spacing, uint16_t *_data){
	width = _width; height = _height;
	spacing = _spacing;
	
	data = _data;
}
void XRData::InitGLComponents(){
	if(glInitialised){ std::cout << "Warning: GL components already initialised.\n"; return; }
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, backgroundColour);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glInitialised = true;
}
void XRData::MakeTexture(bool autoWindow) const {
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	
	if(autoWindow){
		const int N = width*height;
		uint16_t min = data[0];
		uint16_t max = data[0];
		for(int i=1; i<N; i++){
			if(data[i] < min) min = data[i];
			else if(data[i] > max) max = data[i];
		}
		
		int16_t *texData = (int16_t *)malloc(N * sizeof(uint16_t));
		
		const float fac = (float)UINT16_MAX/(float)(max - min);
		for(int i=0; i<N; i++) texData[i] = /*data[i] < thresh ? 0 : */(uint16_t)(fac*(float)(data[i] - min));
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, texData);
		
		free(texData);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, data);;
	}
	
	// blurring x-ray with gaussian
	const float s = (CT().Spacing().x + CT().Spacing().y + CT().Spacing().z) / (1.5f*(spacing.x + spacing.y));
	std::cout << "X-ray Gaussian blur applied, sigma = " << s << "\n";
	const int k = (int)floorf(3.0f*s);
	float v[k];
	for(int i=0; i<k; i++) v[i] = expf(-0.5f*(float)(i*i)/(s*s))/(s*sqrt(2.0f*M_PI));
	uint16_t buf[width*height];
	for(int j=0; j<height; j++) for(int i=0; i<width; i++){
		float sum = v[0]*(float)data[width*j + i];
		for(int c=1; c<k; c++) sum += v[c]*(float)(data[width*j + (i - c <= 0 ? 0 : i - c)] +
												   data[width*j + (i + c >= width ? width - 1 : i + c)]);
		buf[width*j + i] = (uint16_t)roundf(sum);
	}
	for(int j=0; j<height; j++) for(int i=0; i<width; i++){
		float sum = v[0]*(float)buf[width*j + i];
		for(int c=1; c<k; c++) sum += v[c]*(float)(buf[width*(j - c <= 0 ? 0 : j - c) + i] +
												   buf[width*(j + c >= height ? height - 1 : j + c) + i]);
		data[width*j + i] = (uint16_t)roundf(sum);
	}
}

Array2D<uint16_t> XRData::ToArrayWithRect(const SDL_Rect &rect) const {
	return Array2D<uint16_t>(data + width*rect.y + rect.x, rect.w, rect.h, 1, width);
}

void XRData::FlipY(){
	//uint16_t buf[width*height];
	//memcpy(buf, data, width*height*sizeof(uint16_t));
	//for(int j=0; j<height; j++) memcpy(&data[j*width], &buf[(height - j - 1)*width], width*sizeof(uint16_t));
	Flip2DByteArrayVertically((uint8_t *)data, width*sizeof(uint16_t), height);
}

// ----------
// XRDicomData
// ----------
void XRDicomData::Load(const char *filename){
	FILE *fptr = fopen(filename, "r");
	if(!fptr){
		fputs("File opening error: ", stderr); exit(3);
	}
	fseek(fptr, 0, SEEK_END);
	long length = ftell(fptr);
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	
	// !was 24 // 16 byte header
	fread(&width, 2, 1, fptr);
	fread(&height, 2, 1, fptr);
	fread(&spacing, 8, 1, fptr);
	//fread(&sourcePosition, 12, 1, fptr);
	fread(&sourcePosition.z, 4, 1, fptr);
	sourcePosition.x = 0.0f; sourcePosition.y = 0.0f;
	
	length -= 16;//24;
	const int N = width*height;
	assert(length == sizeof(uint16_t)*N);
	
	free(data);
	data = (uint16_t *)malloc(length);
	
	size_t result = fread(data, 1, length, fptr);
	if(result != length){ fputs("Reading error", stderr); exit(3); }
	fclose(fptr);
	
	std::cout << "X-Ray: size=[" << width << " x " << height << "], spacing=" << spacing << ", source at " << sourcePosition << "\n";
	
	// transformation matrices
	const vec<2> dims = spacing*(vec<2>){(float)width, (float)height};
	// C2
	{
		matrixC2 = {{
			{1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{-sourcePosition.x, -sourcePosition.y, -sourcePosition.z, 1.0f}
		}};
	}
	
	// P
	{
		const float d = -sourcePosition.z;
		const float n = 0.5f*d;
		const float f = 1.5f*d;
		matrixP = {{
			{d, 0.0f, 0.0f, 0.0f},
			{0.0f, d, 0.0f, 0.0f},
			{sourcePosition.x, sourcePosition.y, n/(n - f), 1.0f},
			{0.0f, 0.0f, -n*f/(n - f), 0.0f}
		}};
	}
	
	// C3
	{
		matrixC3 = {{
			{2.0f/dims.x, 0.0f, 0.0f, 0.0f},
			{0.0f, 2.0f/dims.y, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 1.0f}
		}};
	}
	
	// B2 (uses C2)
	{
		mat<4, 4> F2 = matrixC2.Inverted();
		mat<4, 4> C5 = {{
			{1.0f/dims.x, 0.0f, 0.0f, 0.0f},
			{0.0f, -1.0f/dims.y, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.5f, 0.5f, 0.0f, 1.0f}
		}};
		F2 = C5 & F2;
		matrixB2 = F2.Inverted();
	}
}
void XRDicomData::UpdateSourceOffset(const vec<2> &newSourceOffset){
	matrixC2[3][0] = -newSourceOffset.x;
	matrixC2[3][1] = -newSourceOffset.y;
	matrixP[2][0] = newSourceOffset.x;
	matrixP[2][1] = newSourceOffset.y;
	const vec<2> dims = spacing*(vec<2>){(float)width, (float)height};
	// B2 (uses C2)
	{
		mat<4, 4> F2 = matrixC2.Inverted();
		mat<4, 4> C5 = {{
			{1.0f/dims.x, 0.0f, 0.0f, 0.0f},
			{0.0f, -1.0f/dims.y, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.5f, 0.5f, 0.0f, 1.0f}
		}};
		F2 = C5 & F2;
		matrixB2 = F2.Inverted();
	}
	Render::SourceOffsetUpdated();
}

// ----------
// OtherData
// ----------
void OtherData::Load(bool _leftPresent,
					 bool _rightPresent,
					 const char *fn_points3d,
					 const char *fn_points2d,
					 float _enlargement,
					 const char *fn_ocLeft,
					 const char *fn_ocRight){
	
	leftPresent = _leftPresent; rightPresent = _rightPresent;
	
	// points
	{
		uint32_t d;
		GLfloat *arrayPtr;
		
		// 3d points
		arrayPtr = ReadPoints(fn_points3d, d, n3d);
		assert(d == 3);
		points3d = new Render::DrawArray(n3d, 1, (Render::DrawParameter[]){{3, GL_FLOAT, GL_FALSE}}, (uint8_t *)arrayPtr, true);
		arrayPtr = (GLfloat *)malloc(n3d*3*sizeof(GLfloat));
		p3dTransformed = new Render::DrawArray(n3d, 1, (Render::DrawParameter[]){{3, GL_FLOAT, GL_FALSE}}, (uint8_t *)arrayPtr, true);
		
		// enlarged 3d points
		nEnlarged = leftPresent && rightPresent ? (n3d/2)/subSampleEnlarged : n3d/subSampleEnlarged;
		if(leftPresent){
			arrayPtr = (GLfloat *)malloc(nEnlarged*3*sizeof(GLfloat));
			p3dEnlargedLeft = new Render::DrawArray(nEnlarged, 1, (Render::DrawParameter[]){{3, GL_FLOAT, GL_FALSE}}, (uint8_t *)arrayPtr, true);
		}
		if(rightPresent){
			arrayPtr = (GLfloat *)malloc(nEnlarged*3*sizeof(GLfloat));
			p3dEnlargedRight = new Render::DrawArray(nEnlarged, 1, (Render::DrawParameter[]){{3, GL_FLOAT, GL_FALSE}}, (uint8_t *)arrayPtr, true);
		}
		
		// 2d points
		arrayPtr = ReadPoints(fn_points2d, d, n2d);
		assert(d == 2);
		points2d = new Render::DrawArray(n2d, 1, (Render::DrawParameter[]){{2, GL_FLOAT, GL_FALSE}}, (uint8_t *)arrayPtr, true);
		// applying scaling
		for(int i=0; i<n2d; i++) ((vec<2> *)points2d->array)[i] *= 2.0f*(vec<2>){Render::PortWidthInverse(), Render::PortHeightInverse()}/XRay().Spacing();
	}
	
	// otic capsule meshes
	if(leftPresent) leftOticCapsule = ReadPLY(fn_ocLeft, true, true);
	if(rightPresent) rightOticCapsule = ReadPLY(fn_ocRight, true, true);
	
	enlargement = _enlargement;
}
OtherData::~OtherData(){
	delete points3d;
	delete p3dTransformed;
	delete points2d;
	if(p3dEnlargedLeft) delete p3dEnlargedLeft;
	if(p3dEnlargedRight) delete p3dEnlargedRight;
	if(leftOticCapsule) delete leftOticCapsule;
	if(rightOticCapsule) delete rightOticCapsule;
}

void OtherData::Transform3dPoints(const vec<3> &pan, const mat<4, 4> &rotationMatrix/*, const vec<2> &sourceOffset*/) const {
//	float a[4][4];
//	float viewMatrix[4][4];
	
	//XRay().UpdateSourceOffset(sourceOffset);
	
	mat<4, 4> viewMatrix = XRay().C3() & XRay().P() & XRay().C2() & mat<4, 4>::Translation(pan) & rotationMatrix;
//	memcpy(viewMatrix, rotationMatrix, 16*sizeof(float));
//	M4x4_Translation(pan, a);
//	M4x4_PreMultiply(viewMatrix, a);
//	M4x4_PreMultiply(viewMatrix, XRay().C2());
//	M4x4_PreMultiply(viewMatrix, XRay().P());
//	M4x4_PreMultiply(viewMatrix, XRay().C3());
	
	/*
	// debugging
	const vec<3> ctDimensions = CT().Spacing()*(vec<3>){(float)CT().Width(), (float)CT().Height(), (float)CT().Depth()};
	vec<4> end = (0.5f*ctDimensions)|1.0f;
	M4x4_PreMultiply(end, viewMatrix);
	std::cout << "End -> " << end.xyz()/end.w << "\n";
	*/
	
	for(int i=0; i<n3d; i++){
		const vec<4> homogenous = viewMatrix & (((vec<3> *)points3d->array)[i] | 1.0f);
		((vec<3> *)p3dTransformed->array)[i] = (vec<3>){homogenous.x, homogenous.y, homogenous.z} / homogenous.w;
	}
}
void OtherData::CalcEnlargedPointPositions() const {
	if(leftPresent && rightPresent){
		// left
		vec<3> sum = {0.0f, 0.0f, 0.0f};
		for(int i=0; i<n3d/2; i++) sum += ((vec<3> *)p3dTransformed->array)[i];
		vec<3> centroid = sum / (float)(n3d/2);
		for(int i=0; i<nEnlarged; i++){
			const int index = i*subSampleEnlarged;
			((vec<3> *)p3dEnlargedLeft->array)[i] = ((vec<3> *)p3dTransformed->array)[index] + enlargement*(((vec<3> *)p3dTransformed->array)[index] - centroid);
		}
		
		// right
		sum = {0.0f, 0.0f, 0.0f};
		for(int i=n3d/2; i<n3d; i++) sum += ((vec<3> *)p3dTransformed->array)[i];
		centroid = sum / (float)(n3d/2);
		for(int i=0; i<nEnlarged; i++){
			const int index = n3d/2 + i*subSampleEnlarged;
			((vec<3> *)p3dEnlargedRight->array)[i] = ((vec<3> *)p3dTransformed->array)[index] + enlargement*(((vec<3> *)p3dTransformed->array)[index] - centroid);
		}
	} else {
		vec<3> sum = {0.0f, 0.0f, 0.0f};
		for(int i=0; i<n3d; i++) sum += ((vec<3> *)p3dTransformed->array)[i];
		vec<3> centroid = sum / (float)n3d;
		for(int i=0; i<nEnlarged; i++){
			const int index = i*subSampleEnlarged;
			((vec<3> *)(leftPresent ? p3dEnlargedLeft : p3dEnlargedRight)->array)[i] = ((vec<3> *)p3dTransformed->array)[index] + enlargement*(((vec<3> *)p3dTransformed->array)[index] - centroid);
		}
	}
}
bool OtherData::OutsideEnlarged() const {
	float angles[nEnlarged];
	
	if(leftPresent && rightPresent){
		// left
		for(int p=0; p<n2d/2; p++){ // loop through each x-ray points
			// check it is within enlarged x-ray points
			for(int i=0; i<nEnlarged; i++){ // insert angles between enlarged points and this x-ray point in to array in size order
				const float angle = atan2f(
										   (((vec<3> *)p3dEnlargedLeft->array)[i]).y - (((vec<2> *)points2d->array)[p]).y,
										   (((vec<3> *)p3dEnlargedLeft->array)[i]).x - (((vec<2> *)points2d->array)[p]).x
										   );
				// angles array is sorted highest to lowest and is built up from the beginning, with the first `i - 1` values set for each iteration `i`
				int j;
				for(j=0; j<i; j++) if(angle > angles[j]) break;
				memcpy(&angles[j + 1], &angles[j], (i - j)*sizeof(float));
				angles[j] = angle;
			}
			const float thresh = angles[0] - M_PI;
			bool in = false;
			for(int i=0; i<nEnlarged - 1; i++){
				if(angles[i] - angles[i + 1] > M_PI) return true;
				if(angles[i + 1] < thresh){ in = true; break; }
			}
			if(!in) return true;
		}
		
		// right
		for(int p=n2d/2; p<n2d; p++){ // loop through each x-ray points
			// check it is within enlarged x-ray points
			for(int i=0; i<nEnlarged; i++){ // insert angles between enlarged points and this x-ray point in to array in size order
				const float angle = atan2f(
										   (((vec<3> *)p3dEnlargedRight->array)[i]).y - (((vec<2> *)points2d->array)[p]).y,
										   (((vec<3> *)p3dEnlargedRight->array)[i]).x - (((vec<2> *)points2d->array)[p]).x
										   );
				// angles array is sorted highest to lowest and is built up from the beginning, with the first `i - 1` values set for each iteration `i`
				int j;
				for(j=0; j<i; j++) if(angle > angles[j]) break;
				memcpy(&angles[j + 1], &angles[j], (i - j)*sizeof(float));
				angles[j] = angle;
			}
			const float thresh = angles[0] - M_PI;
			bool in = false;
			for(int i=0; i<nEnlarged - 1; i++){
				if(angles[i] - angles[i + 1] > M_PI) return true;
				if(angles[i + 1] < thresh){ in = true; break; }
			}
			if(!in) return true;
		}
	} else {
		for(int p=0; p<n2d; p++){ // loop through each x-ray points
			// check it is within enlarged x-ray points
			for(int i=0; i<nEnlarged; i++){ // insert angles between enlarged points and this x-ray point in to array in size order
				const float angle = atan2f(
										   (((vec<3> *)(leftPresent ? p3dEnlargedLeft : p3dEnlargedRight)->array)[i]).y - (((vec<2> *)points2d->array)[p]).y,
										   (((vec<3> *)(leftPresent ? p3dEnlargedLeft : p3dEnlargedRight)->array)[i]).x - (((vec<2> *)points2d->array)[p]).x
										   );
				// angles array is sorted highest to lowest and is built up from the beginning, with the first `i - 1` values set for each iteration `i`
				int j;
				for(j=0; j<i; j++) if(angle > angles[j]) break;
				memcpy(&angles[j + 1], &angles[j], (i - j)*sizeof(float));
				angles[j] = angle;
			}
			const float thresh = angles[0] - M_PI;
			bool in = false;
			for(int i=0; i<nEnlarged - 1; i++){
				if(angles[i] - angles[i + 1] > M_PI) return true;
				if(angles[i + 1] < thresh){ in = true; break; }
			}
			if(!in) return true;
		}
	}
	
	return false;
}
void OtherData::WriteElectrodeRays(const vec<3> &pan, const mat<4, 4> &rotationMatrix, const char *filename) const {
	std::ofstream outfile = std::ofstream(filename);
	outfile.setf(std::ios::fixed);
	outfile.precision(6);
	
//	float m[4][4];
//	float a[4][4];
//	memcpy(m, CT().C1Inverse(), 16*sizeof(float));
//	M4x4_PreMultiply(m, rotationMatrix);
//	M4x4_Translation(pan, a);
//	M4x4_PreMultiply(m, a);
//	M4x4_Inverse(m, m);
	
	const mat<4, 4> m = (mat<4, 4>::Translation(pan) & rotationMatrix & CT().C1Inverse()).Inverted();
	
	vec<4> p4;
	vec<3> p3;
	
	if(leftPresent && rightPresent){
		// left
		outfile << "left:\n";
		// electrodes / electrode projectors
		for(int i=0; i<n2d/2; i++){
			p4 = {
				0.5f*(float)Render::PortWidth()*XRay().Spacing().x*((vec<2> *)points2d->array)[i].x,
				0.5f*(float)Render::PortHeight()*XRay().Spacing().y*((vec<2> *)points2d->array)[i].y,
				0.0f,
				1.0f
			};
			p4 = m & p4;
			p3 = 0.1f * (vec<3>){p4.x, p4.y, p4.z} / p4.w;
			outfile << "LANDMARK " << p3.x << " " << p3.y << " " << p3.z << " electrode projector " << n2d/2 - i << "\n";
		}
		// source position / electrode projector origin
		p4 = m & (XRay().SourcePosition() | 1.0f);
//		M4x4_PreMultiply(p4, m);
		p3 = 0.1f * (vec<3>){p4.x, p4.y, p4.z} / p4.w;
		outfile << "LANDMARK " << p3.x << " " << p3.y << " " << p3.z << " electrode projector origin";
		
		// right
		outfile << "\nright:\n";
		// electrodes / electrode projectors
		for(int i=n2d/2; i<n2d; i++){
			p4 = {
				0.5f*(float)Render::PortWidth()*XRay().Spacing().x*((vec<2> *)points2d->array)[i].x,
				0.5f*(float)Render::PortHeight()*XRay().Spacing().y*((vec<2> *)points2d->array)[i].y,
				0.0f,
				1.0f
			};
//			M4x4_PreMultiply(p4, m);
			p4 = m & p4;
			p3 = 0.1f * (vec<3>){p4.x, p4.y, p4.z} / p4.w;
			outfile << "LANDMARK " << p3.x << " " << p3.y << " " << p3.z << " electrode projector " << n2d - i << "\n";
		}
		// source position / electrode projector origin
		p4 = m & (XRay().SourcePosition() | 1.0f);
//		M4x4_PreMultiply(p4, m);
		p3 = 0.1f * (vec<3>){p4.x, p4.y, p4.z} / p4.w;
		outfile << "LANDMARK " << p3.x << " " << p3.y << " " << p3.z << " electrode projector origin";
	} else {
		outfile << (leftPresent ? "left" : "right") << " only:\n";
		// electrodes / electrode projectors
		for(int i=0; i<n2d; i++){
			p4 = {
				0.5f*(float)Render::PortWidth()*XRay().Spacing().x*((vec<2> *)points2d->array)[i].x,
				0.5f*(float)Render::PortHeight()*XRay().Spacing().y*((vec<2> *)points2d->array)[i].y,
				0.0f,
				1.0f
			};
//			M4x4_PreMultiply(p4, m);
			p4 = m & p4;
			p3 = 0.1f * (vec<3>){p4.x, p4.y, p4.z} / p4.w;
			outfile << "LANDMARK " << p3.x << " " << p3.y << " " << p3.z << " electrode projector " << n2d - i << "\n";
		}
		// source position / electrode projector origin
		p4 = m & (XRay().SourcePosition() | 1.0f);
//		M4x4_PreMultiply(p4, m);
		p3 = 0.1f * (vec<3>){p4.x, p4.y, p4.z} / p4.w;
		outfile << "LANDMARK " << p3.x << " " << p3.y << " " << p3.z << " electrode projector origin";
	}
	
	outfile.close();
}

Params ReadInitialAlignment(const char *filename){
	FILE *fptr = fopen(filename, "r");
	if(!fptr){ fputs("File opening error", stderr); exit(3); }
	fseek(fptr, 0, SEEK_END);
	long length = ftell(fptr);
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	
	Params ret;
	
	// 32 byte data file
	assert(length == 32);
	fread(&ret.pan.x, 4, 1, fptr);
	fread(&ret.pan.y, 4, 1, fptr);
	fread(&ret.pan.z, 4, 1, fptr);
	fread(&ret.rotation.x, 4, 1, fptr);
	fread(&ret.rotation.y, 4, 1, fptr);
	fread(&ret.rotation.z, 4, 1, fptr);
	fread(&ret.sourceOffset.x, 4, 1, fptr);
	fread(&ret.sourceOffset.y, 4, 1, fptr);
	
	fclose(fptr);
	
	return ret;
}


// ! returned array is malloced, size = d_out*n_out*sizeof(GLfloat);
GLfloat *ReadPoints(const char *filename, uint32_t &d_out, uint32_t &n_out){
	FILE *fptr = fopen(filename, "r");
	if(!fptr){ fputs("File opening error", stderr); exit(3); }
	fseek(fptr, 0, SEEK_END);
	long length = ftell(fptr);
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	
	// 8 byte header
	fread(&d_out, 4, 1, fptr);
	fread(&n_out, 4, 1, fptr);
	length -= 8;
	const int N = d_out*n_out;
	assert(length == sizeof(GLfloat)*N);
	
	GLfloat *ret = (GLfloat *)malloc(length);
	
	size_t result = fread(ret, 1, length, fptr);
	if(result != length){ fputs("Reading error", stderr); exit(3); }
	fclose(fptr);
	
	return ret;
}

