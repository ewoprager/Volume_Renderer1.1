#include "Data.h"
#include "View.h"
#include "PLY.h"
#include "Plot.h"
#include "SimMetric.h"

#include "Render.h"

namespace Render {

static SDL_Window *window;

static XRData drr;
static uint16_t volumeRenderBuffer[VOLUME_RENDER_BUFFER_SIZE];

// window variables
static int portWidth;
static int portHeight;
static float portWidthInverse;
static float portHeightInverse;

// comparison region
static SDL_Rect compRect; // x, y of the BLHC, and width, height, of the comparison rectangle, measured up and to the right from the BLHC of the screen (because glReadPixels reads with a vertical flip)
static size_t compSize; // size of the comparison rectangle that will be used in the volume render buffer
void SetCompRect(const SDL_Rect &rect){
	compRect = rect;
	compSize = rect.w*rect.h*sizeof(uint16_t);
	if(compSize > VOLUME_RENDER_BUFFER_SIZE){ std::cout << "ERROR: compRect size exceeded maximum memory allocated.\n"; }
}

// GL variables
static GLuint vbo[3];
static const int pixelPackAlignment = 2;

// getters
int PortWidth(){ return portWidth; }
int PortHeight(){ return portHeight; }
float PortWidthInverse(){ return portWidthInverse; }
float PortHeightInverse(){ return portHeightInverse; }
const SDL_Rect &CompRect(){ return compRect; }
const uint16_t (&VolumeRenderBuffer())[VOLUME_RENDER_BUFFER_SIZE] { return volumeRenderBuffer; }
const GLuint &VBO(const int& index){ return vbo[index]; }

static bool takeDrr = false;
bool &ShouldTakeDRR(){ return takeDrr; }

static bool saveRects = false;
static char saveRectsName[100];
void ShouldSaveRects(const char *name){
	if(name) strcpy(saveRectsName, name);
	saveRects = true;
}


static bool moveBoundaries = true;
bool GetMoveBoundaries(){ return moveBoundaries; }
void SetMoveBoundaries(bool value){ moveBoundaries = value; }

static int boundaryOffset = 0;
int GetBoundaryOffset(){ return boundaryOffset; }
void SetBoundaryOffset(int value){ boundaryOffset = value; }

// volume renderer surface
static const DrawArray vertexArray = DrawArray(6, 2, (DrawParameter[]){{3, GL_FLOAT, GL_FALSE}, {2, GL_FLOAT, GL_FALSE}}, (uint8_t *)(GLfloat[]){
	-1.0f, 1.0f, 0.5f,	 0.0f, 0.0f,
	-1.0f,-1.0f, 0.5f,	 0.0f, 1.0f,
	1.0f,-1.0f, 0.5f,	 1.0f, 1.0f,
	-1.0f, 1.0f, 0.5f,	 0.0f, 0.0f,
	1.0f,-1.0f, 0.5f,	 1.0f, 1.0f,
	1.0f, 1.0f, 0.5f,	 1.0f, 0.0f
});

void GLInit(){
	// volume renderer
	ESDL::AddShaderProgram({
		"../Shaders/vert.vert",
		"../Shaders/frag.frag",
		2, // no. of attributes
		{"a_position", "a_texCoord"}, // names
		{1, 1}, // sizes (in nos. of vec<4>s)
		11, // no. of uniforms
		{"u_B1", "u_B2", "u_jumpSize", "u_B1_noTranslation", "u_data", "u_samplesN", "u_nInv", "u_range", "u_centre", "u_drawMode", "u_xRay"} // names
	});
	// points in 2d space / HUD
	ESDL::AddShaderProgram({
		"../Shaders/hud.vert",
		"../Shaders/hud.frag",
		1, // no. of attributes
		{"a_position"}, // names
		{1}, // sizes (in nos. of vec<4>s)
		2, // no. of uniforms
		{"u_colour", "u_depth"} // names
	});
	// otic capsule
	ESDL::AddShaderProgram({
		"../Shaders/oc.vert",
		"../Shaders/oc.frag",
		3, // no. of attributes
		{"a_position", "a_normal", "a_colour"}, // names
		{1, 1, 1}, // sizes (in nos. of vec<4>s)
		2, // no. of uniforms
		{"u_M", "u_M_normal"} // names
	});
	// axes
	ESDL::AddShaderProgram({
		"../Shaders/axes.vert",
		"../Shaders/axes.frag",
		2, // no. of attributes
		{"a_position", "a_colour"}, // names
		{1, 1}, // sizes (in nos. of vec<4>s)
		1, // no. of uniforms
		{"u_depth"} // names
	});
	// plot
	ESDL::AddShaderProgram({
		"../Shaders/plot.vert",
		"../Shaders/axes.frag",
		2, // no. of attributes
		{"a_xPosition", "a_yPosition"}, // names
		{1, 1}, // sizes (in nos. of vec<4>s)
		1, // no. of uniforms
		{"u_depth"} // names
	});
	// mask
	ESDL::AddShaderProgram({
		"../Shaders/mask.vert",
		"../Shaders/mask.frag",
		1, // no. of attributes
		{"a_position"}, // names
		{1}, // sizes (in nos. of vec<4>s)
		0, // no. of uniforms
		{} // names
	});
	
	// creating window
	portWidth = XRay().Width();
	portHeight = XRay().Height();
	portWidthInverse = 1.0f / (float)portWidth;
	portHeightInverse = 1.0f / (float)portHeight;
	const Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
	window = SDL_CreateWindow("Volume Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, portWidth, portHeight, flags);
	int drawableWidth, drawableHeight;
	SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
	assert(drawableWidth == portWidth);
	assert(drawableHeight == portHeight);
	
	SetCompRect({
		(int)(0.15f*(float)portWidth), // x of TLHC
		(int)(0.35f*(float)portHeight), // y of TLHC
		(int)(0.7f*(float)portWidth), // width
		(int)(0.375f*(float)portHeight) // height
	});
	
	// initialising GL with the window and shader programs
	ESDL::InitGL(window);
	
	glGenBuffers(3, vbo);
	
	// GL settings
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_3D);
	glEnable(GL_TEXTURE_2D);
	glPointSize(2.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glReadBuffer(GL_BACK);
	// all our data is 2-byte data; default alignment is 4
	glPixelStorei(GL_PACK_ALIGNMENT, pixelPackAlignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, pixelPackAlignment);
	
	drr.InitGLComponents();
}

void MainInit(){
	
	//float rotationMatrix[4][4];
	//RotationMatrix(ViewManager().RotationNoInput(), rotationMatrix);
	//Data().WriteElectrodeRays(ViewManager().PanNoInput(), rotationMatrix, "/Users/eprager/Documents/Onedrive - University of Cambridge/CUED/4th Year Project/electrodes.txt");
	
	// initialising constant uniforms
	glUseProgram(PROGRAM(SP::main));
	glUniformMatrix4fv(UNIFORM(SP::main, U_main::B2), 1, GL_FALSE, &XRay().B2()[0][0]);
	glUniform1i(UNIFORM(SP::main, U_main::data), 0); // GL_TEXTURE0
	glUniform1i(UNIFORM(SP::main, U_main::xRay), 1); // GL_TEXTURE1 (this does change tho when you press 'x')
	
	Plot::InitMemry();
}
void SourceOffsetUpdated(){
	glUseProgram(PROGRAM(SP::main));
	glUniformMatrix4fv(UNIFORM(SP::main, U_main::B2), 1, GL_FALSE, &XRay().B2()[0][0]);
}

void DrawAxes(const vec<3> &origin, const mat<4, 4> transformation, float size=1.0f){
	glUseProgram(PROGRAM(SP::axes));
	glUniform1f(UNIFORM(SP::axes, U_axes::depth), 0.0);
	
	mat<4, 4> fourPositions = {{
		origin | 1.0f,
		origin + (vec<3>){size, 0.0f, 0.0f} | 1.0f,
		origin + (vec<3>){0.0f, size, 0.0f} | 1.0f,
		origin + (vec<3>){0.0f, 0.0f, size} | 1.0f
	}};
//	vec<3> offset;
//	memcpy(&fourPositions[0][0], &origin, 3*sizeof(float)); fourPositions[0][3] = 1.0f;
//	offset = origin + (vec<3>){size, 0.0f, 0.0f};
//	memcpy(&fourPositions[1][0], &offset, 3*sizeof(float)); fourPositions[1][3] = 1.0f;
//	offset = origin + (vec<3>){0.0f, size, 0.0f};
//	memcpy(&fourPositions[2][0], &offset, 3*sizeof(float)); fourPositions[2][3] = 1.0f;
//	offset = origin + (vec<3>){0.0f, 0.0f, size};
//	memcpy(&fourPositions[3][0], &offset, 3*sizeof(float)); fourPositions[3][3] = 1.0f;
	
	fourPositions = transformation & fourPositions;
	
//	M4x4_PreMultiply(fourPositions, transformation);
	
//	for(int i=0; i<16; i++) fourPositions[i/4][i%4] /= fourPositions[i/4][3];
	for(int c=0; c<4; ++c) fourPositions[c] /= fourPositions[c][3];
	
	static const float colours[4*4] = {
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f
	};
	static const GLuint indices[6] = {
		0, 1,
		0, 2,
		0, 3
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(ATTRIBUTE(SP::axes, A_axes::position), 3, GL_FLOAT, GL_FALSE, 4*sizeof(float), (GLvoid *)0);
	glEnableVertexAttribArray(ATTRIBUTE(SP::axes, A_axes::position));
	glBufferData(GL_ARRAY_BUFFER, 16*sizeof(float), &fourPositions, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(ATTRIBUTE(SP::axes, A_axes::colour), 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (GLvoid *)0);
	glEnableVertexAttribArray(ATTRIBUTE(SP::axes, A_axes::colour));
	glBufferData(GL_ARRAY_BUFFER, 16*sizeof(float), colours, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLuint), indices, GL_STATIC_DRAW);
	glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);
	
}

void DrawVolumeRender(const vec<3> &pan, const mat<4, 4> &rotationMatrix, int samplesN, float range, float centre, const DrawMode &drawMode/*, const vec<2> &sourceOffset*/){
	
	const float nInv = 1.0/(float)samplesN;
	
//	float a[4][4];
//	
//	float F1[4][4];
//	memcpy(F1, CT().C4(), 16*sizeof(float));
//	M4x4_PreMultiply(F1, rotationMatrix);
//	M4x4_Translation(pan, a);
//	M4x4_PreMultiply(F1, a);
//	M4x4_PreMultiply(F1, XRay().C2());
//	float B1[4][4];
//	M4x4_Inverse(F1, B1);
//	float B1_noTranslation[3][3];
//	M4x4_Extract3x3(B1, B1_noTranslation);
	
	const mat<4, 4> F1 = XRay().C2() & mat<4, 4>::Translation(pan) & rotationMatrix & CT().C4();
	const mat<4, 4> B1 = F1.Inverted();
	
	const mat<3, 3> B1_noTranslation = {{
		{B1[0].x, B1[0].y, B1[0].z},
		{B1[1].x, B1[1].y, B1[1].z},
		{B1[2].x, B1[2].y, B1[2].z}
	}};
	
	/*
	 // debugging
	 float B[4][4];
	 memcpy(B, B2, 16*sizeof(float));
	 M4x4_PreMultiply(B, B1);
	 float F[4][4];
	 M4x4_Inverse(B, F);
	 float texToScrn[4][4] = {
	 2.0f, 0.0f, 0.0f, 0.0f,
	 0.0f, -2.0f, 0.0f, 0.0f,
	 0.0f, 0.0f, 1.0f, 0.0f,
	 -1.0f, 1.0f, 0.0f, 1.0f
	 };
	 M4x4_PreMultiply(F, texToScrn);
	 DrawAxes({0.0f, 0.0f, 0.0f}, F);
	 */
	
	// main shader program
	glUseProgram(PROGRAM(SP::main));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	
	// ct data texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, CT().TextureHandle());
	
	// x-ray data texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, XRay().TextureHandle());
	
	// uniforms
	glUniformMatrix4fv(UNIFORM(SP::main, U_main::B1), 1, GL_FALSE, reinterpret_cast<const GLfloat *>(&B1));
	glUniform1f(UNIFORM(SP::main, U_main::jumpSize), CT().DirVecLength()*nInv);
	glUniformMatrix3fv(UNIFORM(SP::main, U_main::B1_nt), 1, GL_FALSE, reinterpret_cast<const GLfloat *>(&B1_noTranslation));
	glUniform1i(UNIFORM(SP::main, U_main::samplesN), samplesN);
	glUniform1f(UNIFORM(SP::main, U_main::nInv), nInv);
	glUniform1f(UNIFORM(SP::main, U_main::range), range);
	glUniform1f(UNIFORM(SP::main, U_main::centre), centre);
	glUniform1i(UNIFORM(SP::main, U_main::drawMode), (int)drawMode);
	glUniform1i(UNIFORM(SP::main, U_main::xRay), drawMode == DrawMode::static_drr_and_shallow_drr ? 2 : 1); // GL_TEXTURE2 vs. GL_TEXTURE1
	
	// attributes
	vertexArray.Enable(ATTRIBUTE(SP::main, A_main::position), 0);
	vertexArray.Enable(ATTRIBUTE(SP::main, A_main::texCoord), 1);
	
	// drawing
	glBufferData(GL_ARRAY_BUFFER, vertexArray.size, vertexArray.array, GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, vertexArray.verticesN);
}

void TakeDRR(){
	glReadPixels(compRect.x, compRect.y, compRect.w, compRect.h, GL_RG, GL_UNSIGNED_BYTE, volumeRenderBuffer);
}
void WholeDRR(){
	static const size_t xraySize = portWidth*portHeight*sizeof(uint16_t);
	static uint16_t *drrData = nullptr;
	free(drrData);
	drrData = (uint16_t *)malloc(xraySize);
	glReadPixels(0, 0, portWidth, portHeight, GL_RED, GL_UNSIGNED_SHORT, drrData);
	drr.Load(portWidth, portHeight, XRay().Spacing(), drrData);
	drr.MakeTexture(false);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, drr.TextureHandle());
}

//void Render(float (XRData::*similarityMetric)(const uint16_t [], const SDL_Rect &) const){
void Render(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int ))){
	glViewport(0, 0, portWidth, portHeight);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// view control with mouse
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	const vec<3> pan = ViewManager().Pan(mouseX, mouseY);
	const mat<4, 4> rotationMatrix = RotationMatrix(ViewManager().Rotation(mouseX, mouseY));
	//const vec<2> sourceOffset = ViewManager().SourceOffset();
	
	if(moveBoundaries){
		if(ViewManager().CompRectMode()) SetCompRect(ViewManager().ManualCompRect(mouseX, mouseY));
		else CalcValidBoundaries(pan, rotationMatrix);
	}
	
	// drawing volume render for buffer
	DrawVolumeRender(pan, rotationMatrix, ViewManager().SamplesN(), ViewManager().Range(), ViewManager().Centre(), DrawMode::deep_drr_only/*, sourceOffset*/);
	
	// buffering
	TakeDRR();
	if(takeDrr){
		WholeDRR();
		takeDrr = false;
	}
	
	if(saveRects){
		static char xrayName[200] = "/Users/eprager/Library/CloudStorage/OneDrive-UniversityofCambridge/CUED/4th Year Project/Output_files/Comp rects/xray_";
		static const unsigned long xrayPrefixLength = strlen(xrayName);
		static char drrName[200] = "/Users/eprager/Library/CloudStorage/OneDrive-UniversityofCambridge/CUED/4th Year Project/Output_files/Comp rects/drr_";
		static const unsigned long drrPrefixLength = strlen(drrName);
		static const char suffix[5] = ".dat";
		
		unsigned long l = strlen(saveRectsName);
		memcpy(xrayName + xrayPrefixLength, saveRectsName, l);
		memcpy(xrayName + xrayPrefixLength + l, suffix, 5);
		memcpy(drrName + drrPrefixLength, saveRectsName, l);
		memcpy(drrName + drrPrefixLength + l, suffix, 5);
		SimMetric::SaveRects(drrName, Array2D<uint16_t>(volumeRenderBuffer, compRect.w, compRect.h),
							 xrayName, XRay().ToArrayWithRect(compRect),
							 [](int j, int i) -> bool { return XRayMask().PosMasked(compRect.x + i, XRay().Height() - compRect.y - j); });
		saveRects = false;
	}
	
	if(ViewManager().Plot()){
		const float f = (ViewManager().DrawMode() == DrawMode::static_drr_and_shallow_drr ?
							similarityMetric(Array2D<uint16_t>(volumeRenderBuffer, compRect.w, compRect.h), drr.ToArrayWithRect(compRect), nullptr) :
							similarityMetric(Array2D<uint16_t>(volumeRenderBuffer, compRect.w, compRect.h), XRay().ToArrayWithRect(compRect), [](int j, int i) -> bool { return XRayMask().PosMasked(compRect.x + i, XRay().Height() - compRect.y - j); })
						);
		
		//std::cout << f << "\n";
		Plot::UpdateMemry(f);
	}
	
	// clearing to then render for display
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// drawing volume render for display
	DrawVolumeRender(pan, rotationMatrix, ViewManager().SamplesN(), ViewManager().Range(), ViewManager().Centre(), ViewManager().DrawMode()/*, sourceOffset*/);
	
	glUseProgram(PROGRAM(SP::hud));
	
#ifdef RENDER_POINTS_3D
	// points in 3d
	glUniform4f(UNIFORM(SP::hud, U_hud::colour), 1.0, 0.0, 0.0, 1.0);
	glUniform1f(UNIFORM(SP::hud, U_hud::depth), 0.2f);
	Data().Transform3dPoints(pan, rotationMatrix/*, ViewManager().SourceOffset()*/);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	Data().P3DTransformed().Enable(ATTRIBUTE(SP::hud, A_hud::position), 0);
	glBufferData(GL_ARRAY_BUFFER, Data().P3DTransformed().size, Data().P3DTransformed().array, GL_STATIC_DRAW);
	glDrawArrays(GL_POINTS, 0, Data().P3DTransformed().verticesN);
#endif
	
#ifdef RENDER_POINTS_ENLARGED
	// enlarged 3d points:
	glUniform4f(UNIFORM(SP::hud, U_hud::colour), 1.0, 1.0, 0.0, 1.0);
	glUniform1f(UNIFORM(SP::hud, U_hud::depth), 0.1f);
	Data().CalcEnlargedPointPositions();
	// left
	if(Data().P3DEnlargedLeft()){
		Data().P3DEnlargedLeft()->Enable(ATTRIBUTE(SP::hud, A_hud::position), 0);
		glBufferData(GL_ARRAY_BUFFER, Data().P3DEnlargedLeft()->size, Data().P3DEnlargedLeft()->array, GL_STATIC_DRAW);
		glDrawArrays(GL_POINTS, 0, Data().P3DEnlargedLeft()->verticesN);
	}
	// right
	if(Data().P3DEnlargedRight()){
		Data().P3DEnlargedRight()->Enable(ATTRIBUTE(SP::hud, A_hud::position), 0);
		glBufferData(GL_ARRAY_BUFFER, Data().P3DEnlargedRight()->size, Data().P3DEnlargedRight()->array, GL_STATIC_DRAW);
		glDrawArrays(GL_POINTS, 0, Data().P3DEnlargedRight()->verticesN);
	}
#endif
	
#ifdef RENDER_POINTS_2D
	// points in 2d
	glUniform4f(UNIFORM(SP::hud, U_hud::colour), 0.0, 1.0, 0.0, 1.0);
	glUniform1f(UNIFORM(SP::hud, U_hud::depth), 0.3f);
	Data().Points2d().Enable(ATTRIBUTE(SP::hud, A_hud::position), 0);
	glBufferData(GL_ARRAY_BUFFER, Data().Points2d().size, Data().Points2d().array, GL_STATIC_DRAW);
	glDrawArrays(GL_POINTS, 0, Data().Points2d().verticesN);
#endif
	

	// axes
//	float a[4][4];
//	float M[4][4];
//	memcpy(M, CT().C1Inverse(), 16*sizeof(float));
//	M4x4_PreMultiply(M, rotationMatrix);
//	M4x4_Translation(pan, a);
//	M4x4_PreMultiply(M, a);
	mat<4, 4> M = mat<4, 4>::Translation(pan) & rotationMatrix & CT().C1Inverse();
	
	// the transformation matrix for the normals is `TRC1' with translation removed:
//	float M_normal[3][3];
//	M4x4_Extract3x3(M, M_normal);
	const mat<3, 3> M_normal = {{
		{M[0].x, M[0].y, M[0].z},
		{M[1].x, M[1].y, M[1].z},
		{M[2].x, M[2].y, M[2].z}
	}};
	
	M = XRay().C3() & XRay().P() & XRay().C2() & M;
	
//	M4x4_PreMultiply(M, XRay().C2());
//	M4x4_PreMultiply(M, XRay().P());
//	M4x4_PreMultiply(M, XRay().C3());
#ifdef RENDER_AXES
	DrawAxes({-85.3f, -90.0f, 16.5f}, M, 80.0f);
	// need z-flip about this origin
	
	/*
	{
		float M_[4][4];
		M4x4_Inverse(CT().C1Inverse(), M_);
		M4x4_PreMultiply(M_, M);
		const vec<3> dims = (vec<3>){(float)CT().Width(), (float)CT().Height(), (float)CT().Depth()}*CT().Spacing();
		DrawAxes(-0.5f*dims, M_, (float)CT().Depth()*CT().Spacing().z);
	}
	*/
#endif
	
#ifdef RENDER_OTIC_CAPSULES
	glUseProgram(PROGRAM(SP::oc));
	glUniformMatrix4fv(UNIFORM(SP::oc, U_oc::M), 1, GL_FALSE, &M[0][0]);
	glUniformMatrix3fv(UNIFORM(SP::oc, U_oc::M_normal), 1, GL_FALSE, &M_normal[0][0]);
	// left otic capsule
	if(Data().LeftOC()){
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		Data().LeftOC()->Enable(ATTRIBUTE(SP::oc, A_oc::position), 0);
		Data().LeftOC()->Enable(ATTRIBUTE(SP::oc, A_oc::normal), 1);
		Data().LeftOC()->Enable(ATTRIBUTE(SP::oc, A_oc::colour), 2);
		glBufferData(GL_ARRAY_BUFFER, Data().LeftOC()->size, Data().LeftOC()->array, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Data().LeftOC()->indicesSize, Data().LeftOC()->indexArray, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, Data().LeftOC()->indicesN, GL_UNSIGNED_INT, 0);
	}
	// right otic capsule
	if(Data().RightOC()){
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		Data().RightOC()->Enable(ATTRIBUTE(SP::oc, A_oc::position), 0);
		Data().RightOC()->Enable(ATTRIBUTE(SP::oc, A_oc::normal), 1);
		Data().RightOC()->Enable(ATTRIBUTE(SP::oc, A_oc::colour), 2);
		glBufferData(GL_ARRAY_BUFFER, Data().RightOC()->size, Data().RightOC()->array, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Data().RightOC()->indicesSize, Data().RightOC()->indexArray, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, Data().RightOC()->indicesN, GL_UNSIGNED_INT, 0);
	}
#endif
	
#ifdef RENDER_MEMRY
	Plot::DrawMemry();
#endif
	
#ifdef RENDER_COMP_RECT
	// drawing comparison rectangle
	glUseProgram(PROGRAM(SP::plot));
	glUniform1f(UNIFORM(SP::plot, U_plot::depth), 0.0);
	
	vec<4> xs = {(float)compRect.x, (float)compRect.x, (float)(compRect.x + compRect.w), (float)(compRect.x + compRect.w)};
	xs *= 2.0f/(float)portWidth;
	xs -= {1.0f, 1.0f, 1.0f, 1.0f};
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(ATTRIBUTE(SP::plot, A_plot::xPos), 1, GL_FLOAT, GL_FALSE, 1*sizeof(float), (GLvoid *)0);
	glEnableVertexAttribArray(ATTRIBUTE(SP::plot, A_plot::xPos));
	glBufferData(GL_ARRAY_BUFFER, sizeof(xs), &xs, GL_STATIC_DRAW);
	
	vec<4> ys = {(float)compRect.y, (float)(compRect.y + compRect.h), (float)compRect.y, (float)(compRect.y + compRect.h)};
	ys *= 2.0f/(float)portHeight;
	ys -= {1.0f, 1.0f, 1.0f, 1.0f};
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(ATTRIBUTE(SP::plot, A_plot::yPos), 1, GL_FLOAT, GL_FALSE, 1*sizeof(float), (GLvoid *)0);
	glEnableVertexAttribArray(ATTRIBUTE(SP::plot, A_plot::yPos));
	glBufferData(GL_ARRAY_BUFFER, sizeof(ys), &ys, GL_STATIC_DRAW);
	
	static const GLuint is[8] = {
		0, 1,
		0, 2,
		1, 3,
		2, 3
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(is), is, GL_STATIC_DRAW);
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, 0);
#endif
	
	if(ViewManager().MaskMode()) XRayMask().Draw();
	
	
	// swapping buffers
	SDL_GL_SwapWindow(window);
}

void CalcValidBoundaries(const vec<3> &pan, const mat<4, 4> &rotationMatrix){
//	float a[4][4];
//	float M[4][4];
//	memcpy(M, CT().C4(), 16*sizeof(float));
//	M4x4_PreMultiply(M, rotationMatrix);
//	M4x4_Translation(pan, a);
//	M4x4_PreMultiply(M, a);
//	M4x4_PreMultiply(M, XRay().C2());
//	M4x4_PreMultiply(M, XRay().P());
	
	const mat<4, 4> M = XRay().P() & XRay().C2() & mat<4, 4>::Translation(pan) & rotationMatrix & CT().C4();
	
	const mat<4, 4> bottom = M & (mat<4, 4>){{
		{0.0f, 0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f},
		{1.0f, 1.0f, 0.0f, 1.0f}
	}};
	const mat<4, 4> top = M & (mat<4, 4>){{
		{0.0f, 0.0f, 1.0f, 1.0f},
		{1.0f, 0.0f, 1.0f, 1.0f},
		{0.0f, 1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f, 1.0f}
	}};
//	M4x4_PreMultiply(bottom, M);
//	M4x4_PreMultiply(top, M);
	
	// coordinates of the 8 corners of the CT data cuboid, in X-ray space
	vec<3> lll = *(vec<3> *)(&bottom[0][0])/bottom[0][3];
	vec<3> lhl = *(vec<3> *)(&bottom[1][0])/bottom[1][3];
	vec<3> llh = *(vec<3> *)(&bottom[2][0])/bottom[2][3];
	vec<3> lhh = *(vec<3> *)(&bottom[3][0])/bottom[3][3];
	vec<3> hll = *(vec<3> *)(&top[0][0])/top[0][3];
	vec<3> hhl = *(vec<3> *)(&top[1][0])/top[1][3];
	vec<3> hlh = *(vec<3> *)(&top[2][0])/top[2][3];
	vec<3> hhh = *(vec<3> *)(&top[3][0])/top[3][3];
	
	// 4 limits of the valid rectangle, in X-ray space
	float left, right, under, over;
	
	left = lll.x;
	if(llh.x > left) left = llh.x;
	if(hll.x > left) left = hll.x;
	if(hlh.x > left) left = hlh.x;
	
	right = lhl.x;
	if(lhh.x < right) right = lhh.x;
	if(hhl.x < right) right = hhl.x;
	if(hhh.x < right) right = hhh.x;
	
	// transforming limits from X-ray space into vertically flipped X-ray pixels (because glReadPixels reads vertically flipped)
	left = left/XRay().Spacing().x + 0.5f*(float)XRay().Width();
	right = right/XRay().Spacing().x + 0.5f*(float)XRay().Width();
	
	left += boundaryOffset;
	right -= boundaryOffset;
	
	if(right - left < 10) return;
	
	under = lll.y;
	if(lhl.y > under) under = lhl.y;
	if(llh.y > under) under = llh.y;
	if(lhh.y > under) under = lhh.y;
	
	over = hll.y;
	if(hhl.y < over) over = hhl.y;
	if(hlh.y < over) over = hlh.y;
	if(hhh.y < over) over = hhh.y;
	
	under = under/XRay().Spacing().y + 0.5f*(float)XRay().Height();
	over = over/XRay().Spacing().y + 0.5f*(float)XRay().Height();
	
	under += boundaryOffset;
	over -= boundaryOffset;
	
	if(over - under < 10) return;
	
	//std::cout << left << ", " << right << ", " << under << ", " << over << "\n";
	
	//if(left < 0.3*(float)XRay().Width()) left = 0.3*(float)XRay().Width();
	//if(right > 0.7*(float)XRay().Width()) right = 0.7*(float)XRay().Width();
	
	SetCompRect({
		(int)(left), // x of BLHC
		(int)(under), // y of BLHC
		(int)(right - left), // width
		(int)(over - under) // height
	});
}
mat<4, 4> RotationMatrix(const vec<3> &rotation){
//	M4x4_xRotation(-0.5f*M_PI, out);
//	float a[4][4];
//	M4x4_yRotation(rotation.z, a);
//	M4x4_PreMultiply(out, a);
//	M4x4_xRotation(rotation.y, a);
//	M4x4_PreMultiply(out, a);
//	M4x4_zRotation(rotation.x, a);
//	M4x4_PreMultiply(out, a);
	
	return mat<4, 4>::ZRotation(rotation.x) & mat<4, 4>::XRotation(rotation.y) & mat<4, 4>::YRotation(rotation.z) & mat<4, 4>::XRotation(-0.5f * float(M_PI));
}

//float F(float (XRData::*similarityMetric)(const uint16_t [], const SDL_Rect &) const, const Params &params){
	
float F(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), const Params &params){
	UpdateSourceOffset(params.sourceOffset);
	const mat<4, 4> rotationMatrix = RotationMatrix(params.rotation);
	if(moveBoundaries) CalcValidBoundaries(params.pan, rotationMatrix);
	
	glViewport(0, 0, portWidth, portHeight);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawVolumeRender(params.pan, rotationMatrix, ViewManager().SamplesN(), ViewManager().Range(), ViewManager().Centre(), DrawMode::deep_drr_only/*, params.sourceOffset*/);
	TakeDRR();
	
	return similarityMetric(Array2D<uint16_t>(volumeRenderBuffer, compRect.w, compRect.h), XRay().ToArrayWithRect(compRect), [](int j, int i) -> bool { return XRayMask().PosMasked(compRect.x + i, XRay().Height() - compRect.y - j); });
}

void SaveScreenshot(const char *fileName, const SDL_Rect &rect){
	glPixelStorei(GL_PACK_ALIGNMENT, 4); // SDL surfaces seem to require a pack alignment of 4 (rows of 2d arrays must start at the beginning of a word)
	glReadBuffer(GL_FRONT); // want to read from the rendered buffer, not the back buffer
	
	// creating SDL surface with pixel format compatible with GL_RGB, GL_UNSIGNED_BYTE
	SDL_Surface *surface = SDL_CreateRGBSurface(0, rect.w, rect.h, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
	if(!surface){ std::cout << "ERROR: Surface creation failed.\n"; return; }
	
	// reading pixel data into surface
	glReadPixels(rect.x, rect.y, rect.w, rect.h, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
	
	// flipping data vertically; 'surface->pitch' is the byte width of the 2d array stored, which is different to `3*portWidth` because of the pack alignment
	Flip2DByteArrayVertically((uint8_t *)surface->pixels, surface->pitch, rect.h);
	
	// saving bitmap image
	SDL_SaveBMP(surface, fileName);
	SDL_FreeSurface(surface);
	
	std::cout << "Screenshot saved to " << fileName << ".\n";
	
	// resetting GL settings
	glReadBuffer(GL_BACK);
	glPixelStorei(GL_PACK_ALIGNMENT, pixelPackAlignment);
}

}
