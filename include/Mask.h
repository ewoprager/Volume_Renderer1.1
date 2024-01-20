#ifndef Mask_hpp
#define Mask_hpp

#include "Header.h"

#define MAX_CELL_SHAPES (16)

struct Circle {
	int x, y, sqRadius;
};
struct Cell {
	int N;
	Circle circles[MAX_CELL_SHAPES];
	
	bool PosMasked(int x, int y) const;
};
class Mask {
public:
	void Init(int _width, int _height, int _maxShapeRadius=40);
	~Mask();
	
	void Clear() const;
	void Add(const Circle &circle) const;
	
	bool PosMasked(int x, int y) const;
	
	void Draw() const;
	
private:
	int maxShapeRadius;
	int width, height;
	int rows, columns;
	
	Cell *cells = nullptr; // row-major 2d array of cells
	
	vec<2> XRtoScreen(const vec<2> &v) const;
};

#endif /* Mask_hpp */
