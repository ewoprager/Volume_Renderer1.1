#include <ESDL/ESDL_EventHandler.h>

#include "Data.h"
#include "Render.h"
#include "Plot.h"
#include "Mask.h"
#include "SimMetric.h"
#ifndef SKIP_OPTIMISE
#include "Optimise.h"
#endif

#include "View.h"

void View::Init(const Params &_initialAligment,  float (*_similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), const char *_datasetName){
	initialAligment = _initialAligment;
	similarityMetric = _similarityMetric;
	memcpy(datasetName, _datasetName, strlen(_datasetName));
	
	// manually override stored initial alignment for debugging
	initialAligment = {{-4.3754, -5.01304, -134.259}, {-0.0560102, 0.0285118, 0.0944358}, {-4.28922, -1.13346}};
	
	SDL_Event event;
	event.button.button = SDL_BUTTON_RIGHT;
	event.type = SDL_MOUSEBUTTONDOWN;
	ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::RightDown}, event);
	event.type = SDL_MOUSEBUTTONUP;
	ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::RightUp}, event);
	event.button.button = SDL_BUTTON_LEFT;
	event.type = SDL_MOUSEBUTTONDOWN;
	ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::LeftDown}, event);
	event.type = SDL_MOUSEBUTTONUP;
	ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::LeftUp}, event);
	event.type = SDL_KEYDOWN;
	event.key.keysym.sym = KEY_PRINT; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyPrint}, event);
	event.key.keysym.sym = KEY_CYCLE_VIEW; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyCycleView}, event);
	event.key.keysym.sym = KEY_PRINT_INFO; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyPrintInfo}, event);
	event.key.keysym.sym = KEY_RESET_PARAMS; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyResetParams}, event);
	event.key.keysym.sym = KEY_PLOT; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyPlot}, event);
	event.key.keysym.sym = KEY_SAVE_RECTS; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeySaveRects}, event);
	event.key.keysym.sym = KEY_MASK_MODE; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyMaskMode}, event);
	event.key.keysym.sym = KEY_COMP_RECT_MODE; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyCompRectMode}, event);
	event.key.keysym.sym = KEY_WRITE_RAYS; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyWriteRays}, event);
#ifndef SKIP_OPTIMISE
	event.key.keysym.sym = KEY_PS; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyPS}, event);
	event.key.keysym.sym = KEY_LINE_SCANS; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyLineScans}, event);
	event.key.keysym.sym = KEY_TIME; ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyTime}, event);
#endif
	
	// view uniforms
	glUseProgram(ESDL::shaderPrograms[0].handle);
	glUniform1i(ESDL::shaderPrograms[0].uniformLocations[3], samplesN);
	glUniform1f(ESDL::shaderPrograms[0].uniformLocations[6], 1.0f/(float)samplesN);
	glUniform1f(ESDL::shaderPrograms[0].uniformLocations[5], centre);
	glUniform1f(ESDL::shaderPrograms[0].uniformLocations[4], range);
	glUniform1i(ESDL::shaderPrograms[0].uniformLocations[7], (int)drawMode);
	
	UpdateSourceOffset(params.sourceOffset);
	
	LoadSettings();
}
View::~View(){
	SaveSettings();
}

// ----------
// Mouse Inputs
// ----------
void View::RightDown(SDL_Event event){
	rightDown = true;
	rightStartX = event.button.x;
	rightStartY = event.button.y;
}
void View::RightUp(SDL_Event event){
	if(!rightDown) return;
	if(compRectMode && ESDL::GetKeyDown(KEY_MOVE_COMP_RECT)){
		manualRect = ManualCompRect(event.button.x, event.button.y);
	} else {
		params.rotation = Rotation(event.button.x, event.button.y);
	}
	rightDown = false;
}
void View::LeftDown(SDL_Event event){
	if(maskMode){
		XRayMask().Add({event.button.x, event.button.y, 1600});
		return;
	}
	
	leftDown = true;
	leftStartX = event.button.x;
	leftStartY = event.button.y;
}
void View::LeftUp(SDL_Event event){
	if(!leftDown) return;
	if(compRectMode && ESDL::GetKeyDown(KEY_MOVE_COMP_RECT)){
		manualRect = ManualCompRect(event.button.x, event.button.y);
	} else {
		params.pan = Pan(event.button.x, event.button.y);
	}
	leftDown = false;
}

// ----------
// Key Inputs
// ----------
// print the latest plot value and the parameters
void View::KeyPrint(SDL_Event event){
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	const Params _params = {Pan(mouseX, mouseY), Rotation(mouseX, mouseY), params.sourceOffset};
	std::cout << "F = " << Plot::Latest() << " at\n";
	_params.Print();
	const SDL_Rect &compRect = Render::CompRect();
		std::cout << "Comp rect: [" << compRect.w << " x " << compRect.h << "] at (" << compRect.x << ", " << compRect.y << ").\n";
}
// cycle view mode: [drr on xray, drr alone, drr on drr snapshot]
void View::KeyCycleView(SDL_Event event){
	drawMode = (enum DrawMode)(((int)drawMode + 1) % DRAW_MODES_N);
	if(drawMode == DrawMode::static_drr_and_shallow_drr) Render::ShouldTakeDRR() = true;
	if(drawMode != DrawMode::xray_and_shallow_drr && maskMode) DeactivateMaskMode();
}
// print feasibility / [mask mode] print if mouse position is masked
void View::KeyPrintInfo(SDL_Event event){
	if(maskMode){
		int x, y;
		SDL_GetMouseState(&x, &y);
		std::cout << XRayMask().PosMasked(x, y) << "\n";
		return;
	}
	std::cout << (Data().OutsideEnlarged() ? "Not feasible" : "Feasible") << ".\n";
}
// set parameters to the initial alignment
void View::KeyResetParams(SDL_Event event){
	params = initialAligment;
	UpdateSourceOffset(params.sourceOffset);
}
// toggle continuous evaluation and plot
void View::KeyPlot(SDL_Event event){
	plot = !plot;
}
// save comparison rectangles
void View::KeySaveRects(SDL_Event event){
	Render::ShouldSaveRects("manual");
}
// toggle mask mode
void View::KeyMaskMode(SDL_Event event){
	if(drawMode != DrawMode::xray_and_shallow_drr) return;
	if(maskMode) DeactivateMaskMode(); else ActivateMaskMode();
}
// [mask mode] clear mask
void View::KeyClearMask(SDL_Event event){
	XRayMask().Clear();
}
void View::ActivateMaskMode(){
	SDL_Event event;
	event.type = SDL_KEYDOWN;
	event.key.keysym.sym = KEY_CLEAR_MASK;
	inputIdL = ESDL::AddEventCallback((MemberFunction<View, void, SDL_Event>){this, &View::KeyClearMask}, event);
	leftDown = false;
	maskMode = true;
	std::cout << "Mask mode activated.\n";
}
void View::DeactivateMaskMode(){
	if(inputIdL >= 0) ESDL::RemoveEventCallback(inputIdL);
	maskMode = false;
	std::cout << "Mask mode deactivated.\n";
}
// toggle comp rect mode
void View::KeyCompRectMode(SDL_Event event){
	compRectMode = !compRectMode;
	std::cout << "Comp rect mode " << (compRectMode ? "activated" : "deactivated") << ".\n";
}
// write the electrode projectors
void View::KeyWriteRays(SDL_Event event){
//#ifndef SKIP_OPTIMISE
	//const Params range = {{0.1f, 0.1f, 0.5f}, {0.01f, 0.01f, 0.01f}, {0.5f, 0.5f}}; // very larger = *50, large = *10, normal = *1, small = *.2
	
	/*
	Render::SetBoundaryOffset(20);
	Render::SetMoveBoundaries(true);
	Render::Render(similarityMetric);
	Render::SetMoveBoundaries(false);
	
	std::cout << "Performing gradient descent...\n";
	params = Optimise::GradientDescent(similarityMetric, params, range, "/Users/eprager/Library/CloudStorage/OneDrive-UniversityofCambridge/CUED/4th Year Project/Output_files/gradientDescent.txt").params;
	std::cout << "Done.\n";
	
	Render::Render(similarityMetric);
	*/
	
	SDL_Rect rect = Render::CompRect();
	const int wBorder = rect.w/20;
	const int hBorder = rect.h/20;
	rect.x -= wBorder; rect.y -= hBorder; rect.w += 2*wBorder; rect.h += 2*hBorder;
	Render::SaveScreenshot("/Users/eprager/Library/CloudStorage/OneDrive-UniversityofCambridge/CUED/4th Year Project/Output_files/rayAlignmentScreenshot.bmp", rect);
	
	Data().WriteElectrodeRays(params.pan, Render::RotationMatrix(params.rotation), "/Users/eprager/Library/CloudStorage/OneDrive-UniversityofCambridge/CUED/4th Year Project/Output_files/electrodeRays.txt");
	
	/*
	std::cout << "Line scanning...\n";
	Optimise::Lines(similarityMetric, 199, params, 0.2f*range, "/Users/eprager/Library/CloudStorage/OneDrive-UniversityofCambridge/CUED/4th Year Project/Output_files/lineScan.dat");
	std::cout << "Done.\n";
	 */
	Render::SetBoundaryOffset(0);
	Render::SetMoveBoundaries(true);
//#endif
}
// run particle swarm
void View::KeyPS(SDL_Event event){
#ifndef SKIP_OPTIMISE
	
	const Params range = {{0.1f, 0.1f, 0.5f}, {0.01f, 0.01f, 0.01f}, {0.5f, 0.5f}};
	
	const Params params0 = params;
	Optimise::PSParams psParams = {1000, 0.28f, 2.525f, 1.225f}; // N, w, phiP, phiG; {2000, 0.28f, 2.525f, 1.225f}
	Optimise::Particle particles[psParams.N];
	Optimise::OpVals found = Optimise::ParticleSwarm(similarityMetric, 15, psParams, params0, 2.0f*range, false, particles);
	if(found.f > 1.0f){
		std::cout << "Error: No valid solution found.\n";
		return;
	}
	params = found.params;
	std::cout << "Best achieved = " << found.f << " at\n";
	found.params.Print();
	std::cout << "\n";
	
	Render::SetBoundaryOffset(0);
	Render::SetMoveBoundaries(true);
	
#endif
}
#define SE(T) ((double)(UTime() - T)*1.0e-6) // seconds elapsed ranges for plotting
// save line scans along each dimension for each sim. metric
void View::KeyLineScans(SDL_Event event){
#ifndef SKIP_OPTIMISE
	const Optimise::PlotRanges ranges = {0.07f, 0.5f, 0.005f};
	const Params paramsRange = {{ranges.tr1, ranges.tr1, ranges.tr2}, {ranges.rot, ranges.rot, ranges.rot}, {ranges.tr2, ranges.tr2}};
	
	unsigned long T1, T2, T3;
	
	const int simMetricsN = 4;
	static float (*const simMetrics[simMetricsN])(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )) = {
		&SimMetric::HackyGradient,
		&SimMetric::CorrelationRatio,
		&SimMetric::CorrelationCoefficient,
		&SimMetric::GradientCorrelation
	};
	static const char *simMetricNames[simMetricsN] = {
		"heuristic_gradient",
		"correlation_ratio",
		"correlation_coefficient",
		"gradient_correlation"
	};
	
	//const int rangeScalesN = 1;
	//static const float rangeScales[rangeScalesN] = {5.0f};
	const float rangeScale = 5.0f;
	
	const char *const prefix = "/Users/eprager/Library/CloudStorage/OneDrive-UniversityofCambridge/CUED/4th Year Project/Output_files/Line scans/";
	const char *const suffix = ".dat";
	const char *const screenShotSuffix = "_ss.bmp";
	static char buf[200];
	char *position = buf;
	const size_t prelen = strlen(prefix);
	const size_t suflen = strlen(suffix) + 1; // ' + 1' to include the null terminator
	const size_t sssuflen = strlen(screenShotSuffix) + 1; // ' + 1' to include the null terminator
	
	memcpy(position, prefix, prelen);
	position += prelen;
	static const size_t datasetNameLen = strlen(datasetName);
	memcpy(position, datasetName, datasetNameLen);
	position += datasetNameLen;
	*position = '/'; position++;
	
	const Params params0 = params;
	
	std::cout << "Doing line scans with range scale " << rangeScale << ":\n";
	T1 = UTime();
	for(int i=0; i<simMetricsN; i++){
		const size_t l = strlen(simMetricNames[i]);
		memcpy(position, simMetricNames[i], l);
		char *const positionI = position + l;
		
		std::cout << "Sim. metric '" << simMetricNames[i] << "':\n";
		
		T2 = UTime();
		std::cout << "Performing gradient descent...\n";
		params = Optimise::LocalSearch(simMetrics[i], params0, 5.0f*paramsRange).params;
		std::cout << "Done; took " << SE(T2) << "s.\n";
		
		Render::Render(simMetrics[i]);
		
		memcpy(positionI, screenShotSuffix, sssuflen);
		Render::SaveScreenshot(buf, Render::CompRect());
		
		//snprintf(positionI, 10, "_%2.1f", rangeScales[j]);
		memcpy(positionI, suffix, suflen);

		T3 = UTime();
		//std::cout << "Scanning with range scale = " << rangeScales[j] << "...\n";
		Optimise::Lines(simMetrics[i], 99, params, rangeScale*ranges, buf);
		std::cout << "Done; took " << SE(T3) << "s.\n";
		
		std::cout << "\n";
		
	}
	
	std::cout << "Finished line scans; took " << SE(T1)/60.0 << " minutes.\n";
#endif
}
// time the taking of DRRs and then evaluation of each similarity metric
void View::KeyTime(SDL_Event event){
#ifndef SKIP_OPTIMISE
	std::cout << "Timing processess with comparison rectangle size [" << Render::CompRect().w << " x " << Render::CompRect().h << "] = " << Render::CompRect().w*Render::CompRect().h << " total pixels.\n";
	
	// timing just the rendering
	Params params;
	glViewport(0, 0, Render::PortWidth(), Render::PortHeight());
	glClearColor(1.0, 1.0, 1.0, 1.0);
	unsigned long T = UTime();
	for(int i=0; i<10000; i++){
		params = Params::FromFunction([](){ return Uniform(-1.0f, 1.0f); });
		UpdateSourceOffset(params.sourceOffset);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Render::DrawVolumeRender(params.pan, Render::RotationMatrix(params.rotation), ViewManager().SamplesN(), ViewManager().Range(), ViewManager().Centre(), DrawMode::deep_drr_only/*, params.sourceOffset*/);
	}
	const double renderTime = (double)(UTime() - T)*1.0e-7;
	std::cout << "Takes on average " << renderTime << "ms to just render a DRR.\n";
	
	const double drrTime = Optimise::TimeDRR();
	
	std::cout << "Takes on average " << drrTime << "ms to render and read a DRR.\n";
	
	const int simMetricsN = 4;
	static float (*const simMetrics[simMetricsN])(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )) = {
		&SimMetric::HackyGradient,
		&SimMetric::CorrelationRatio,
		&SimMetric::CorrelationCoefficient,
		&SimMetric::GradientCorrelation
	};
	static const char *simMetricNames[simMetricsN] = {
		"heuristic_gradient",
		"correlation_ratio",
		"correlation_coefficient",
		"gradient_correlation"
	};
	
	for(int i=0; i<simMetricsN; i++) std::cout << "Takes on average " << Optimise::TimeSimMetric(simMetrics[i]) - drrTime << "ms to evaluate the " << simMetricNames[i] << "\n";
	
#endif
}

// ----------
// Parameter getters
// ----------
vec<3> View::Rotation(int mouseX, int mouseY) const {
	if(ESDL::GetKeyDown(KEY_MOVE_COMP_RECT) || !rightDown) return params.rotation;
	const float x = (float)(mouseX - rightStartX); // right positive
	const float y = -(float)(mouseY - rightStartY); // up positive
	//const float c = cos(rotation.x);
	//const float s = -sin(rotation.x);
	//vec<3> ret = rotation + rotSensitivity*(vec<3>){0.0f, c*y - s*x, - c*x - s*y};
	vec<3> ret = params.rotation + rotSensitivity*(vec<3>){0.0f, -y, x};
	if(ret.y > M_PI) ret.y = M_PI;
	if(ret.y < -M_PI) ret.y = -M_PI;
	return ret;
}
vec<3> View::Pan(int mouseX, int mouseY) const {
	if(ESDL::GetKeyDown(KEY_MOVE_COMP_RECT) || !leftDown) return params.pan;
	const vec<3> ret = params.pan + panSensitivity*(vec<3>){(float)(mouseX - leftStartX), -(float)(mouseY - leftStartY), 0.0f};
	return ret;
}
SDL_Rect View::ManualCompRect(int mouseX, int mouseY) const {
	if(!ESDL::GetKeyDown(KEY_MOVE_COMP_RECT)) return manualRect;
	SDL_Rect ret = manualRect;
	if(leftDown){
		ret.x += (float)(mouseX - leftStartX); // right positive
		ret.y -= (float)(mouseY - leftStartY); // up positive
	}
	if(rightDown){
		ret.w += (float)(mouseX - rightStartX); // right positive
		if(ret.w < 10) ret.w = 10;
		ret.h -= (float)(mouseY - rightStartY); // up positive
		if(ret.h < 10) ret.h = 10;
	}
	return ret;
}

// ----------
// Update
// ----------
void View::Update(){
	const int centreInput = ESDL::GetKeyDown(KEY_WINDOW_CENTRE_UP) - ESDL::GetKeyDown(KEY_WINDOW_CENTRE_DOWN);
	if(centreInput){
		static const float centrePrecision = 1e-3f;//1e-3f;
		centre += centrePrecision*(float)centreInput;
		if(centre < centrePrecision) centre = centrePrecision;
		else if(centre > 1.0f - centrePrecision) centre = 1.0f - centrePrecision;
		//glUseProgram(ESDL::shaderPrograms[0].handle);
		//glUniform1f(ESDL::shaderPrograms[0].uniformLocations[5], centre);
		std::cout << "Centre: " << centre << "\n";
	}
	const int rangeInput = ESDL::GetKeyDown(KEY_WINDOW_RANGE_UP) - ESDL::GetKeyDown(KEY_WINDOW_RANGE_DOWN);
	if(centreInput || rangeInput){
		static const float rangePrecision = 1e-3f;//1e-3f;
		range += rangePrecision*(float)rangeInput;
		if(range < rangePrecision) range = rangePrecision;
		else {
			const float ab = fabs(centre - 0.5f);
			const float rangeMax = 1.0f - ab - ab;
			if(range > rangeMax) range = rangeMax;
		}
		//glUseProgram(ESDL::shaderPrograms[0].handle);
		//glUniform1f(ESDL::shaderPrograms[0].uniformLocations[4], range);
		std::cout << "Range: " << range << "\n";
	}
	
	const int sampleInput = ESDL::GetKeyDown(KEY_SAMPLES_N_UP) - ESDL::GetKeyDown(KEY_SAMPLES_N_DOWN);
	if(sampleInput){
		samplesN += sampleInput;
		if(samplesN < 10) samplesN = 10;
		else if(samplesN > 2000) samplesN = 2000;
		//glUseProgram(ESDL::shaderPrograms[0].handle);
		//glUniform1i(ESDL::shaderPrograms[0].uniformLocations[3], samplesN);
		//glUniform1f(ESDL::shaderPrograms[0].uniformLocations[6], 1.0f/(float)samplesN);
		std::cout << "No. of samples: " << samplesN << "\n";
	}
	
	const int rollInput = ESDL::GetKeyDown(KEY_ROLL_UP) - ESDL::GetKeyDown(KEY_ROLL_DOWN); // u clockwise, j anticlockwise
	if(rollInput){
		params.rotation.x -= 0.003f*(float)rollInput;
		//std::cout << "Roll: " << rotation.x << "\n";
	}
	
	const int zOffInput = ESDL::GetKeyDown(KEY_Z_UP) - ESDL::GetKeyDown(KEY_Z_DOWN);
	if(zOffInput){
		params.pan.z += 0.1f*(float)zOffInput;
		std::cout << "zOff: " << params.pan.z << "\n";
	}
	
	const int xSrcOffInput = ESDL::GetKeyDown(KEY_SOURCE_X_UP) - ESDL::GetKeyDown(KEY_SOURCE_X_DOWN);
	if(xSrcOffInput){
		params.sourceOffset.x -= 0.1f*(float)xSrcOffInput;
		std::cout << "sourceOffset.x: " << params.sourceOffset.x << "\n";
	}
	const int ySrcOffInput = ESDL::GetKeyDown(KEY_SOURCE_Y_UP) - ESDL::GetKeyDown(KEY_SOURCE_Y_DOWN);
	if(ySrcOffInput){
		params.sourceOffset.y += 0.1f*(float)ySrcOffInput;
		std::cout << "sourceOffset.y: " << params.sourceOffset.y << "\n";
	}
	if(xSrcOffInput || ySrcOffInput) UpdateSourceOffset(params.sourceOffset);
}

// ----------
// Saving and loading; !Important: Delete 'saveFile.dat' from the build directory after modifying the following functions!
// ----------
void View::LoadSettings(){
	FILE *fptr = fopen("saveFile.dat", "r");
	if(!fptr){
		std::cout << "No save file to load.\n";
		return;
	}
	
	fread(&params, sizeof(Params), 1, fptr);
	UpdateSourceOffset(params.sourceOffset);
	fread(&centre, sizeof(float), 1, fptr);
	fread(&range, sizeof(float), 1, fptr);
	fread(&samplesN, sizeof(int), 1, fptr);
	int dm;
	fread(&dm, sizeof(int), 1, fptr);
	drawMode = (enum DrawMode)dm;
	fread(&plot, sizeof(bool), 1, fptr);
	fread(&compRectMode, sizeof(bool), 1, fptr);
	fread(&manualRect, sizeof(SDL_Rect), 1, fptr);
	
	fclose(fptr);
	std::cout << "Settings loaded.\n";
}
void View::SaveSettings(){
	FILE *fptr = fopen("saveFile.dat", "w");
	if(!fptr){ fputs("Error opening file to save.", stderr); exit(3); }
	
	fwrite(&params, sizeof(Params), 1, fptr);
	fwrite(&centre, sizeof(float), 1, fptr);
	fwrite(&range, sizeof(float), 1, fptr);
	fwrite(&samplesN, sizeof(int), 1, fptr);
	int dm = (int)drawMode;
	fwrite(&dm, sizeof(int), 1, fptr);
	fwrite(&plot, sizeof(bool), 1, fptr);
	fwrite(&compRectMode, sizeof(bool), 1, fptr);
	fwrite(&manualRect, sizeof(SDL_Rect), 1, fptr);
	
	fclose(fptr);
	std::cout << "Settings saved.\n";
}
