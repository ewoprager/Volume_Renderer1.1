#ifndef SimMetric_hpp
#define SimMetric_hpp

#include "Tools.h"

namespace SimMetric {

float SumAbsoluteDifferences	(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )=nullptr);
float MutualInformation			(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )=nullptr);
float CorrelationRatio			(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )=nullptr);
float CorrelationCoefficient	(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )=nullptr);
float HackyGradient				(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )=nullptr);
float GradientCorrelation		(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )=nullptr);

void SaveRects(const char *filenameX, const Array2D<uint16_t> &arrayX, const char *filenameY, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )=nullptr);

}

#endif /* SimMetric_hpp */
