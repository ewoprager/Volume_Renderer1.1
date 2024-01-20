#ifndef Render_hpp
#define Render_hpp

#include "Header.h"
#include "Tools.h"

#define RENDER_POINTS_3D
#define RENDER_POINTS_ENLARGED
#define RENDER_POINTS_2D
//#define RENDER_AXES
#define RENDER_OTIC_CAPSULES
#define RENDER_MEMRY
#define RENDER_COMP_RECT

namespace Render {

// enums for shader programs and their attributes and uniforms
enum class SP {main, hud, oc, axes, plot, mask};
enum class A_main {position, texCoord};
enum class U_main {B1, B2, jumpSize, B1_nt, data, samplesN, nInv, range, centre, drawMode, xRay};
enum class A_hud {position};
enum class U_hud {colour, depth};
enum class A_oc {position, normal, colour};
enum class U_oc {M, M_normal};
enum class A_axes {position, colour};
enum class U_axes {depth};
enum class A_plot {xPos, yPos};
enum class U_plot {depth};
enum class A_mask {position};
#define PROGRAM(program) (ESDL::shaderPrograms[(int)program].handle)
#define ATTRIBUTE(program, name) (ESDL::shaderPrograms[(int)program].attribLocations[(int)name])
#define UNIFORM(program, name) (ESDL::shaderPrograms[(int)program].uniformLocations[(int)name])

#define MAX_VS 16
struct DrawParameter {
	GLsizei n;
	GLenum glType;
	GLboolean normalised;
};
struct DrawArray {
	DrawArray(int _verticesN, int _parametersN, const DrawParameter *_parameters, uint8_t *_array, bool _arrayAllocated=false) : verticesN(_verticesN), parametersN(_parametersN), array(_array), arrayAllocated(_arrayAllocated) {
		memcpy(parameters, _parameters, _parametersN*sizeof(DrawParameter));
		stride = 0;
		for(int i=0; i<_parametersN; i++){
			parameterOffsets[i] = stride;
			stride += parameters[i].n*GLTypeSize(parameters[i].glType);
		}
		size = stride*_verticesN;
	}
	~DrawArray(){
		if(arrayAllocated) free(array);
	}
	int verticesN;
	GLsizei stride;
	int parametersN;
	DrawParameter parameters[MAX_VS];
	GLintptr parameterOffsets[MAX_VS];
	uint8_t *const array;
	GLsizei size;
	bool arrayAllocated;
	
	void Enable(GLuint index, int parameterI) const {
		glVertexAttribPointer(index, parameters[parameterI].n, parameters[parameterI].glType, parameters[parameterI].normalised, stride, (GLvoid *)parameterOffsets[parameterI]);
		glEnableVertexAttribArray(index);
	}
};
struct DrawElementArray {
	DrawElementArray() : array(nullptr), indexArray(nullptr) {}
	DrawElementArray(int _verticesN, int _parametersN, const DrawParameter *_parameters, uint8_t *_array, int _indicesN, GLuint *_indexArray, bool _arraysAllocated=false) : verticesN(_verticesN), parametersN(_parametersN), array(_array), indicesN(_indicesN), indexArray(_indexArray), arraysAllocated(_arraysAllocated) {
		memcpy(parameters, _parameters, _parametersN*sizeof(DrawParameter));
		stride = 0;
		for(int i=0; i<_parametersN; i++){
			parameterOffsets[i] = stride;
			stride += parameters[i].n*GLTypeSize(parameters[i].glType);
		}
		size = stride*_verticesN;
		indicesSize = sizeof(GLuint)*_indicesN;
	}
	~DrawElementArray(){
		if(arraysAllocated){
			free(array);
			free(indexArray);
		}
	}
	int verticesN;
	GLsizei stride;
	int parametersN;
	DrawParameter parameters[MAX_VS];
	GLintptr parameterOffsets[MAX_VS];
	uint8_t *const array;
	GLuint *const indexArray;
	GLsizei size;
	int indicesN;
	GLsizei indicesSize;
	bool arraysAllocated;
	
	void Enable(GLuint index, int parameterI) const {
		glVertexAttribPointer(index, parameters[parameterI].n, parameters[parameterI].glType, parameters[parameterI].normalised, stride, (GLvoid *)parameterOffsets[parameterI]);
		glEnableVertexAttribArray(index);
	}
};

void GLInit();
void MainInit();
void Render(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )));

mat<4, 4> RotationMatrix(const vec<3> &rotation);
void DrawVolumeRender(const vec<3> &pan, const mat<4, 4> &rotationMatrix, int samplesN, float range, float centre, const DrawMode &drawMode/*, const vec<2> &sourceOffset*/);
void TakeDRR();
void WholeDRR();
float F(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), const Params &params);

// getters
int PortWidth();
int PortHeight();
float PortWidthInverse();
float PortHeightInverse();
const SDL_Rect &CompRect();
#define VOLUME_RENDER_BUFFER_SIZE (1000*1000*sizeof(uint16_t))
const uint16_t (&VolumeRenderBuffer())[VOLUME_RENDER_BUFFER_SIZE];
GLuint VBO(const int& index);

bool &ShouldTakeDRR();
void ShouldSaveRects(const char *name=nullptr);

bool GetMoveBoundaries();
void SetMoveBoundaries(bool value);
int GetBoundaryOffset();
void SetBoundaryOffset(int value);

void SourceOffsetUpdated();

void CalcValidBoundaries(const vec<3> &pan, const mat<4, 4> &rotationMatrix);

void SaveScreenshot(const char *fileName, const SDL_Rect &rect={0, 0, PortWidth(), PortHeight()});

}

#endif /* Render_hpp */
