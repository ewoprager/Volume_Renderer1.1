#include "Render.h"

#include "Mask.h"

void Mask::Init(int _width, int _height, int _maxShapeRadius){
	width = _width;
	height = _height;
	maxShapeRadius = _maxShapeRadius;
	
	rows = (int)ceilf((float)_height/(float)_maxShapeRadius);
	columns = (int)ceilf((float)_width/(float)_maxShapeRadius);
	
	cells = (Cell *)malloc(rows*columns*sizeof(Cell));
	for(int i=0; i<rows*columns; i++) cells[i].N = 0;
}
Mask::~Mask(){
	if(cells) free(cells);
}

void Mask::Clear() const {
	for(int r=0; r<rows; r++) for(int c=0; c<columns; c++) cells[columns*r + c].N = 0;
}
void Mask::Add(const Circle &circle) const {
	if(circle.x < 0 || circle.y < 0 || circle.x >= width || circle.y >= height){ std::cout << "ERROR: Circle is outside bounds.\n"; return; }
	if(sqrtf((float)circle.sqRadius) > (float)maxShapeRadius + 1e-3f){ std::cout << "ERROR: Given circle is too big.\n"; return; }
	const int cellIndex = columns*(circle.y / maxShapeRadius) + circle.x / maxShapeRadius;
	if(cells[cellIndex].N == MAX_CELL_SHAPES){ std::cout << "ERROR: Cell full.\n"; return; }
	cells[cellIndex].circles[cells[cellIndex].N++] = circle;
}

bool Cell::PosMasked(int x, int y) const {
	for(int i=0; i<N; i++){
		const int dx = circles[i].x - x;
		const int dy = circles[i].y - y;
		if((float)(dx*dx + dy*dy) < circles[i].sqRadius) return true;
	}
	return false;
}
bool Mask::PosMasked(int x, int y) const {
	const int r0 = y / maxShapeRadius;
	const int c0 = x / maxShapeRadius;
	for(int r=(r0 > 0 ? r0 - 1 : r0); r<=(r0 < rows - 1 ? r0 + 1 : r0); r++) for(int c=(c0 > 0 ? c0 - 1 : c0); c<=(c0 < columns - 1 ? c0 + 1 : c0); c++) if(cells[columns*r + c].PosMasked(x, y)) return true;
	return false;
}

#define BUFFER_SIZE (10000)
vec<2> Mask::XRtoScreen(const vec<2> &v) const {
	return {2.0f*v.x/(float)width - 1.0f, -2.0f*v.y/(float)height + 1.0f};
}
void Mask::Draw() const {
	if(!cells) return;
	static const float sideLength = 10.0f;
	static vec<2> vertexBuffer[BUFFER_SIZE];
	int verticesN = 0;
	for(int i=0; i<rows*columns; i++) for(int s=0; s<cells[i].N; s++){
		const vec<2> centre = {(float)cells[i].circles[s].x, (float)cells[i].circles[s].y};
		const float r = sqrtf((float)cells[i].circles[s].sqRadius);
		int n = r*2.0f*M_PI/sideLength;
		if(n < 3) n = 3;
		const float delta = 2.0f*M_PI/(float)n;
		float angle = 0.0f;
		for(int v=0; v<n; v++){
			if(verticesN + 3 > BUFFER_SIZE){ std::cout << "WARNING: Buffer full.\n"; break; }
			vertexBuffer[verticesN++] = XRtoScreen(centre);
			vertexBuffer[verticesN++] = XRtoScreen(centre + r*(vec<2>){cosf(angle), sinf(angle)});
			angle -= delta;
			vertexBuffer[verticesN++] = XRtoScreen(centre + r*(vec<2>){cosf(angle), sinf(angle)});
		}
	}
	Render::DrawArray drawArray = Render::DrawArray(verticesN, 1, (Render::DrawParameter[]){{2, GL_FLOAT, GL_FALSE}}, (uint8_t *)vertexBuffer);
	
	glUseProgram(PROGRAM(Render::SP::mask));
	glBindBuffer(GL_ARRAY_BUFFER, Render::VBO(0));
	drawArray.Enable(ATTRIBUTE(Render::SP::mask, Render::A_mask::position), 0);
	glBufferData(GL_ARRAY_BUFFER, drawArray.size, drawArray.array, GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, drawArray.verticesN);
}
