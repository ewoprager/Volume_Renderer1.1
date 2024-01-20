#ifndef View_hpp
#define View_hpp

#include "Tools.h"
#include "Header.h"

// ----------
// Inputs
// ----------
//#define KEY_ (SDLK_)

// Single-press commands
#define KEY_PRINT (SDLK_p)
#define KEY_CYCLE_VIEW (SDLK_x)
#define KEY_PRINT_INFO (SDLK_c)
#define KEY_RESET_PARAMS (SDLK_v)
#define KEY_PLOT (SDLK_b)
#define KEY_SAVE_RECTS (SDLK_n)
#define KEY_MASK_MODE (SDLK_m)
#define KEY_CLEAR_MASK (SDLK_l)
#define KEY_COMP_RECT_MODE (SDLK_COMMA)
#define KEY_WRITE_RAYS (SDLK_SLASH)
#ifndef SKIP_OPTIMISE
#define KEY_PS (SDLK_PERIOD)
#define KEY_LINE_SCANS (SDLK_SEMICOLON)
#define KEY_TIME (SDLK_QUOTE)
#endif

// Press-and-hold commands
#define KEY_WINDOW_CENTRE_UP (SDLK_t)
#define KEY_WINDOW_CENTRE_DOWN (SDLK_g)
#define KEY_WINDOW_RANGE_UP (SDLK_r)
#define KEY_WINDOW_RANGE_DOWN (SDLK_f)
#define KEY_SAMPLES_N_UP (SDLK_y)
#define KEY_SAMPLES_N_DOWN (SDLK_h)
#define KEY_ROLL_UP (SDLK_u)
#define KEY_ROLL_DOWN (SDLK_j)
#define KEY_Z_UP (SDLK_i)
#define KEY_Z_DOWN (SDLK_k)
#define KEY_SOURCE_X_UP (SDLK_d)
#define KEY_SOURCE_X_DOWN (SDLK_a)
#define KEY_SOURCE_Y_UP (SDLK_w)
#define KEY_SOURCE_Y_DOWN (SDLK_s)

// Modifiers
#define KEY_MOVE_COMP_RECT (SDLK_LSHIFT)

/* view modes, cycled through with 'KEY_CYCLE_VIEW':
 xray_and_shallow_drr: X-ray image in red and green channels simultaneously (same 8-bit value in each), live DRR in blue channel
 deep_drr_only: 16-bit live DRR split across the red and green channels; used for comparison
 static_drr_and_shallow_drr: static DRR simulatenously in red and green channels for comparison with live DRR in blue channel
 shallow_drr_only: 8-bit live DRR in red, green and blue channels simultanously
 */
enum class DrawMode { xray_and_shallow_drr, deep_drr_only, static_drr_and_shallow_drr, shallow_drr_only };
#define DRAW_MODES_N 4

class View {
public:
	void Init(const Params &_initialAligment, float (*_similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), const char *_datasetName);
	~View();
	
	void LeftDown(SDL_Event event); // move x and y parameters / [left shift down] move manual comp rect
	void LeftUp(SDL_Event event);
	void RightDown(SDL_Event event); // move pitch and yaw parameters / [left shift down] change manual comp rect width and height
	void RightUp(SDL_Event event);
	
	void KeyPrint(SDL_Event event); // print the latest plot value and the parameters
	void KeyCycleView(SDL_Event event); // cycle view mode: [drr on xray, drr alone, drr on drr snapshot]
	void KeyPrintInfo(SDL_Event event); // print feasibility / [mask mode] print if mouse position is masked
	void KeyResetParams(SDL_Event event); // set parameters to the initial alignment
	void KeyPlot(SDL_Event event); // toggle continuous evaluation and plot
	void KeySaveRects(SDL_Event event); // save comparison rectangles
	void KeyMaskMode(SDL_Event event); // toggle mask mode
	void KeyClearMask(SDL_Event event); // [mask mode] clear mask
	void KeyCompRectMode(SDL_Event event); // toggle comp rect mode
	void KeyWriteRays(SDL_Event event); // write the electrode projectors
	void KeyPS(SDL_Event event); // run particle swarm, followed by a gradient descent, then save line scans along each dimension
	void KeyLineScans(SDL_Event event); // save line scans along each dimension for each sim. metric
	void KeyTime(SDL_Event event); // time the taking of DRRs and then evaluation of each similarity metric
	
	vec<3> Rotation(int mouseX, int mouseY) const;
	const vec<3> &RotationNoInput() const { return params.rotation; }
	vec<3> Pan(int mouseX, int mouseY) const;
	const vec<3> &PanNoInput() const { return params.pan; }
	//void RotationMatrix(float out[4][4], int mouseX, int mouseY) const;
	//void RotationMatrixNoInput(float out[4][4]) const;
	const vec<2> &SourceOffset() const { return params.sourceOffset; }
	SDL_Rect ManualCompRect(int mouseX, int mouseY) const;
	
	void Update();
	
	float Centre() const { return centre; }
	float Range() const { return range; }
	int SamplesN() const { return samplesN; }
	const DrawMode &DrawMode() const { return drawMode; }
	bool Plot() const { return plot; }
	bool MaskMode() const { return maskMode; }
	bool CompRectMode() const { return compRectMode; }
	
	void LoadSettings();
	void SaveSettings();
	
	Params params = {{0.0f, 0.0f, -131.164}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}; // saved&loaded
	
	// N18:
	//{{-9.65031, -31.4651, -96.4318}, {0.00379649, 0.0787029, -0.165692}, {-3.09748, -0.702257}};
	
	// E1N11:
	//{{12.1101, -8.23623, -93.6409}, {0.138352, -0.43264, 0.0317284}, {-6.64991, 0.804447}};
	
	// first set:
	// 0.793944 at
	//{{-4.3754, -5.01304, -134.259}, {-0.0560102, 0.0285118, 0.0944358}, {-4.28922, -1.13346}};
	
	// zero
	//{{0.0f, 0.0f, -131.164}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}};
	
private:
	static constexpr float rotSensitivity = 0.001f;
	static constexpr float panSensitivity = 0.1f;
	
	int rightStartX = 0;
	int rightStartY = 0;
	bool rightDown = false;
	
	int leftStartX = 0;
	int leftStartY = 0;
	bool leftDown = false;
	
	float centre = 0.92f; // saved&loaded
	float range = 0.16f; // saved&loaded
	int samplesN = 600; // saved&loaded
	enum DrawMode drawMode = DrawMode::xray_and_shallow_drr; // saved&loaded
	bool plot = true; // saved&loaded
	
	// mask mode
	bool maskMode = false;
	int inputIdL = -1;
	void ActivateMaskMode();
	void DeactivateMaskMode();
	
	// comp rect mode
	bool compRectMode = false; // saved&loaded
	SDL_Rect manualRect = {0, 0, 100, 100}; // x, y, width, height; saved&loaded
	
	Params initialAligment;
	float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int ));
	
	char datasetName[MAX_DATASET_NAME_LENGTH];
};

#endif /* View_hpp */
