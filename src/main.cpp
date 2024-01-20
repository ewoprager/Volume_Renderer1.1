//#include <iostream>

#include <ESDL/ESDL_EventHandler.h>

#include "mattresses.h"

#include "SimMetric.h"
#include "Header.h"
#include "Data.h"
#include "View.h"
#include "Render.h"
#include "Mask.h"

#ifndef SKIP_OPTIMISE
#include "Optimise.h"
#endif

// instances
static CTDicomData ct;
static XRDicomData xRay;
static OtherData data;
static View viewManager;
static Mask xrayMask;

// instance getters
const CTDicomData &CT(){ return ct; }
const XRDicomData &XRay(){ return xRay; }
const OtherData &Data(){ return data; }
const View &ViewManager(){ return viewManager; }
const Mask &XRayMask(){ return xrayMask; }

// instance modifiers
void UpdateSourceOffset(const vec<2> &newSourceOffset){ xRay.UpdateSourceOffset(newSourceOffset); }
void SetViewParams(const Params &newParams){ viewManager.params = newParams; UpdateSourceOffset(viewManager.params.sourceOffset); }

int main(int argc, const char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO); // initialises SDL
	
	ESDL::InitEventHandler(); // sets event handling constants and key array to false
	
	xRay.Load("../Inputs/xray.dat"); // loads X-ray data, C2, P, C3 and B2 transformation matrices
	xRay.FlipY(); // X-Ray data is henceforth flipped vertically for comparison with data read via 'glReadPixels'
	
	xrayMask.Init(xRay.Width(), xRay.Height());
	
	bool leftPresent, rightPresent;
	char datasetName[MAX_DATASET_NAME_LENGTH];
	ct.Load("../Inputs/ct.dat", leftPresent, rightPresent, datasetName); // loads CT data, C4 transformation matrix
	
	Render::GLInit(); // needs X-ray size for window size, sets derived constants, initialises OpenGL
	
	data.Load(leftPresent,
			  rightPresent,
			  "../Inputs/points3d.dat",
			  "../Inputs/points2d.dat",
			  0.3f,
			  "../Inputs/left otic capsule.ply",
			  "../Inputs/right otic capsule.ply"
			  ); // loads segmented features, needs window constants
	
	xRay.InitGLComponents();
	ct.InitGLComponents();
	
	xRay.MakeTexture(true); // need X-ray data, CT loaded
	ct.MakeTexture(true); // needs CT data
	
	Params initialAligment = ReadInitialAlignment("../Inputs/initialAlignment.dat"); // reads initial alignment parameters
	
	std::cout << "Initial alignment: " << initialAligment << "\n";
	
	float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )) = &SimMetric::HackyGradient;
	
	viewManager.Init(initialAligment, similarityMetric, datasetName); // initialises the view object, needs the initial alignment parameters and for the xrData object to be intiailised
	
	Render::MainInit(); // sets some render constants, needs window constants, view object, OpenGL initialised
	
	while(!ESDL::HandleEvents()){ // app loop; quits on ESC key or cross-off of window
		
		viewManager.Update(); // updates the view
		
		Render::Render(similarityMetric); // renders
	}
	
	return 0;
}
