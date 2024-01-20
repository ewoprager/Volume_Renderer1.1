#ifndef PLY_hpp
#define PLY_hpp

#include "Tools.h"
#include "Render.h"

Render::DrawElementArray *ReadPLY(const char *fileName, bool reverseWinding=false, bool flipNormals=false);

#endif /* PLY_hpp */
