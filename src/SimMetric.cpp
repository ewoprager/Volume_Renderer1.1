#include <map>

#include "mattresses.h"

#include "SimMetric.h"

//#define INTENSITY_BASED_EXCLUSION
#ifdef INTENSITY_BASED_EXCLUSION
#define DRR_LOWER_THRESH 17000
#define XRAY_LOWER_THRESH 2800
// DRR should be the first array, X, X-ray should be the second array, Y.
#endif

namespace SimMetric {

float SumAbsoluteDifferences(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )){
	assert(arrayX.Width() == arrayY.Width());
	assert(arrayX.Height() == arrayY.Height());
	const unsigned int w = arrayX.Width();
	const unsigned int h = arrayX.Height();
	
	int skipped = 0;
	float ret = 0.0f;
	for(int j=0; j<h; j++) for(int i=0; i<w; i++){
		if(exclude) if(exclude(j, i)){ skipped++; continue; }
#ifdef INTENSITY_BASED_EXCLUSION
		if(arrayX(j, i) < DRR_LOWER_THRESH){ skipped++; continue; }
		if(arrayY(j, i) < XRAY_LOWER_THRESH){ skipped++; continue; }
#endif
		ret += fabs((float)(arrayX(j, i) - arrayY(j, i)));
	}
	
	//std::cout << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h << "; " << i0 << "\n";
	
	ret /= (double)(w*h - skipped);
	
	return ret;
}

float MutualInformation(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )){
	assert(arrayX.Width() == arrayY.Width());
	assert(arrayX.Height() == arrayY.Height());
	const unsigned int w = arrayX.Width();
	const unsigned int h = arrayX.Height();
	
	static const uint32_t N = UINT16_MAX + 1;
	
	std::map<uint32_t, uint32_t> hist = std::map<uint32_t, uint32_t>();
	
	static uint16_t xMarginal[N];
	static uint16_t yMarginal[N];
	memset(xMarginal, 0, N*sizeof(uint16_t));
	memset(yMarginal, 0, N*sizeof(uint16_t));
	
	int skipped = 0;
	for(int j=0; j<h; j++) for(int i=0; i<w; i++){
		if(exclude) if(exclude(j, i)){ skipped++; continue; }
		
		const uint16_t x = arrayX(j, i);
		const uint16_t y = arrayY(j, i);
#ifdef INTENSITY_BASED_EXCLUSION
		if(x < DRR_LOWER_THRESH){ skipped++; continue; }
		if(y < XRAY_LOWER_THRESH){ skipped++; continue; }
#endif
		const uint32_t key = ((uint32_t)x << 16) | (uint32_t)y;
		hist[key]++;
		xMarginal[x]++;
		yMarginal[y]++;
	}
	
	//unsigned long T;
	
	const float nInv = 1.0f/(float)(w*h - skipped);
	
	float xEntropy_mn, yEntropy_mn, jointEntropy_mn;
	
	xEntropy_mn = 0.0f;
	for(int i=0; i<N; i++) if(xMarginal[i]) xEntropy_mn += (float)xMarginal[i]*log2((float)xMarginal[i]*nInv);
	
	yEntropy_mn = 0.0f;
	for(int i=0; i<N; i++) if(yMarginal[i]) yEntropy_mn += (float)yMarginal[i]*log2((float)yMarginal[i]*nInv);
	
	// calculating joint entropy, optimising by first taking histogram of the histogram
	static const int histHistSize = 1000;
	static uint16_t histHist[histHistSize];
	//T = UTime();
	memset(histHist, 0, histHistSize*sizeof(uint16_t));
	for(auto it=hist.begin(); it!=hist.end(); it++) histHist[it->second]++;
	jointEntropy_mn = 0.0f;
	for(int i=1; i<histHistSize; i++) if(histHist[i]) jointEntropy_mn += (float)(histHist[i]*i)*log2((float)i*nInv);
	//T = UTime() - T;
	//std::cout << "New gvs " << jointEntropy_mn << " in " << T << "\n";
	
	return - (jointEntropy_mn - xEntropy_mn - yEntropy_mn)*nInv;
}

float CorrelationRatio(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )){
	assert(arrayX.Width() == arrayY.Width());
	assert(arrayX.Height() == arrayY.Height());
	const unsigned int w = arrayX.Width();
	const unsigned int h = arrayX.Height();
	
	static const unsigned long xMax = UINT16_MAX + 1;
	
	const unsigned long N = w*h;
	
	static double sumYOmegais[xMax];
	memset(sumYOmegais, 0, xMax*sizeof(double));
	
	static unsigned long Nis[xMax];
	memset(Nis, 0, xMax*sizeof(unsigned long));
	
	int skipped = 0;
	
	KahanSum sumYOmega = 0.0;
	KahanSum sumYOmega2 = 0.0;
	for(int j=0; j<h; j++) for(int i=0; i<w; i++){
		if(exclude) if(exclude(j, i)){ skipped++; continue; }
#ifdef INTENSITY_BASED_EXCLUSION
		if(arrayX(j, i) < DRR_LOWER_THRESH){ skipped++; continue; }
		if(arrayY(j, i) < XRAY_LOWER_THRESH){ skipped++; continue; }
#endif
		const double yOmega = (double)arrayX(j, i);
		const double yOmega2 = yOmega*yOmega;
		
		sumYOmega += yOmega;
		sumYOmega2 += yOmega2;
		
		const uint16_t y = arrayY(j, i);
		Nis[y]++;
		sumYOmegais[y] += yOmega;
	}
	// turning sum(Y(omega)^2) into Ni*sigma_i^2 and summing:
	KahanSum ret = sumYOmega2;
	for(int i=0; i<xMax; i++) if(Nis[i]) ret -= sumYOmegais[i]*sumYOmegais[i]/(double)Nis[i];
	
	return ret/(sumYOmega2 - sumYOmega*sumYOmega/(double)(N - skipped));
}

float CorrelationCoefficient(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )){
	assert(arrayX.Width() == arrayY.Width());
	assert(arrayX.Height() == arrayY.Height());
	const unsigned int w = arrayX.Width();
	const unsigned int h = arrayX.Height();
	
	KahanSum sumX, sumY, sumX2, sumY2, sumXY;
	
	int skipped = 0;
	for(int j=0; j<h; j++) for(int i=0; i<w; i++){
		if(exclude) if(exclude(j, i)){ skipped++; continue; }
#ifdef INTENSITY_BASED_EXCLUSION
		if(arrayX(j, i) < DRR_LOWER_THRESH){ skipped++; continue; }
		if(arrayY(j, i) < XRAY_LOWER_THRESH){ skipped++; continue; }
#endif
		const double x = (double)arrayX(j, i);
		const double y = (double)arrayY(j, i);

		sumX += x;
		sumY += y;
		sumX2 += x*x;
		sumY2 += y*y;
		sumXY += x*y;
	}
	const double N = (double)(w*h - skipped);
	//const double b = (N*sumXY - sumX*sumY)/(N*sumX2 - sumX*sumX);
	//std::cout << "drr = " << (sumY - b*sumX)/N << " + " << b << " * xray\n";
	return 1.0f - (float)(N*sumXY - sumX*sumY)/(sqrt(N*sumX2 - sumX*sumX)*sqrt(N*sumY2 - sumY*sumY));
}

float HackyGradient(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )){
	assert(arrayX.Width() == arrayY.Width());
	assert(arrayX.Height() == arrayY.Height());
	const unsigned int w = arrayX.Width();
	const unsigned int h = arrayX.Height();
	
	static const int grad_k = 4;
	static const float gkM1Inv = 1.0f/(float)(grad_k - 1);
	static const int m = grad_k / 2; // spacing between regions for which the vectors are calculated; must be <= grad_k
	const int wm = (w - grad_k) / m + 1; // no. of cols of vectors
	const int hm = (h - grad_k) / m + 1; // no. of rows of vectors
	
	// kernel; it is applied horizontally for the horizontal gradient, and applied vertically for the vertical gradient
	float v[grad_k];
	for(int l=0; l<grad_k; l++) v[l] = -1.0f + 2.0f*(float)l*gkM1Inv;
	
	// KahanSum is just a double that also stores its error value (and overloads `operator+=(...)` appropriately etc...)
	// the sum of dot products;
	KahanSum ret;
	
	// for the pmcc
	KahanSum sumX, sumY, sumX2, sumY2, sumXY;
	
	int skipped = 0;
	for(int jm=0; jm<hm; jm++) for(int im=0; im<wm; im++){
		const int i = im*m;
		const int j = jm*m;
		
		if(exclude) if(exclude(j + m/2, i + m/2)){ skipped++; continue; }
#ifdef INTENSITY_BASED_EXCLUSION
		if(arrayX(j + m/2, i + m/2) < DRR_LOWER_THRESH){ skipped++; continue; }
		if(arrayY(j + m/2, i + m/2) < XRAY_LOWER_THRESH){ skipped++; continue; }
#endif
		// applying the kernel for the horizontal and vertical components of the vector for this cell, one each for the two images
		vec<2> gX = {0.0f, 0.0f};
		vec<2> gY = {0.0f, 0.0f};
		for(int l1=0; l1<grad_k; l1++) for(int l2=0; l2<grad_k; l2++){
			gX += v[l1]*(vec<2>){(float)arrayX(j + l2, i + l1), (float)arrayX(j + l1, i + l2)};
			gY += v[l1]*(vec<2>){(float)arrayY(j + l2, i + l1), (float)arrayY(j + l1, i + l2)};
		}
		
		// getting the magnitudes to eventually calculate the pmcc of the magnitudes between the two images
		const float x = sqrt(gX.SqMag());
		const float y = sqrt(gY.SqMag());
		sumX += x; sumY += y; sumX2 += x*x; sumY2 += y*y; sumXY += x*y;
		
		// skipping if either are close to 0
		if(x < 1e-3f){ skipped++; continue; }
		if(y < 1e-3f){ skipped++; continue; }
		
		// summing the dot product of the two vectors, normalised
		ret += Dot(gX/x, gY/y);
	}
	// finally computing the pmcc of the vector magnitudes, and combining with the average of the vector dot products
	const double av = ret/(double)(wm*hm - skipped);
	
	const double N = (double)(wm*hm);
	const float pmcc = (float)(N*sumXY - sumX*sumY)/(sqrt(N*sumX2 - sumX*sumX)*sqrt(N*sumY2 - sumY*sumY));
	
	return 1.0f - sqrt(pmcc)*av;
}

float GradientCorrelation(const Array2D<uint16_t> &arrayX, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )){
	assert(arrayX.Width() == arrayY.Width());
	assert(arrayX.Height() == arrayY.Height());
	const unsigned int w = arrayX.Width();
	const unsigned int h = arrayX.Height();
	
	KahanSum sumXi, sumYi, sumX2i, sumY2i, sumXYi;
	KahanSum sumXj, sumYj, sumX2j, sumY2j, sumXYj;
	
	int skipped = 0;
	for(int j=1; j<h - 1; j++) for(int i=1; i<w - 1; i++){
		if(exclude) if(exclude(j, i)){ skipped++; continue; }
#ifdef INTENSITY_BASED_EXCLUSION
		if(arrayX(j, i) < DRR_LOWER_THRESH){ skipped++; continue; }
		if(arrayY(j, i) < XRAY_LOWER_THRESH){ skipped++; continue; }
#endif
		const float xi = 2.0f*((float)arrayX(j, i + 1) - (float)arrayX(j, i - 1)) +
		(float)arrayX(j + 1, i + 1) +
		(float)arrayX(j - 1, i + 1) -
		(float)arrayX(j + 1, i - 1) -
		(float)arrayX(j - 1, i - 1);
		const float xj = 2.0f*((float)arrayX(j + 1, i) - (float)arrayX(j - 1, i)) +
		(float)arrayX(j + 1, i + 1) +
		(float)arrayX(j + 1, i - 1) -
		(float)arrayX(j - 1, i + 1) -
		(float)arrayX(j - 1, i - 1);
		const float yi = 2.0f*((float)arrayY(j, i + 1) - (float)arrayY(j, i - 1)) +
		(float)arrayY(j + 1, i + 1) +
		(float)arrayY(j - 1, i + 1) -
		(float)arrayY(j + 1, i - 1) -
		(float)arrayY(j - 1, i - 1);
		const float yj = 2.0f*((float)arrayY(j + 1, i) - (float)arrayY(j - 1, i)) +
		(float)arrayY(j + 1, i + 1) +
		(float)arrayY(j + 1, i - 1) -
		(float)arrayY(j - 1, i + 1) -
		(float)arrayY(j - 1, i - 1);
		
		sumXi += xi; sumYi += yi; sumX2i += xi*xi; sumY2i += yi*yi; sumXYi += xi*yi;
		sumXj += xj; sumYj += yj; sumX2j += xj*xj; sumY2j += yj*yj; sumXYj += xj*yj;
	}
	const double N = (double)((w - 2)*(h - 2) - skipped);
	//const double b = (N*sumXY - sumX*sumY)/(N*sumX2 - sumX*sumX);
	//std::cout << "drr = " << (sumY - b*sumX)/N << " + " << b << " * xray\n";
	const float ri = (float)(N*sumXYi - sumXi*sumYi)/(sqrt(N*sumX2i - sumXi*sumXi)*sqrt(N*sumY2i - sumYi*sumYi));
	const float rj = (float)(N*sumXYj - sumXj*sumYj)/(sqrt(N*sumX2j - sumXj*sumXj)*sqrt(N*sumY2j - sumYj*sumYj));
	
	return 1.0f - 0.5f*(ri + rj);
}

void SaveRects(const char *filenameX, const Array2D<uint16_t> &arrayX, const char *filenameY, const Array2D<uint16_t> &arrayY, bool (*exclude)(int , int )){
	assert(arrayX.Width() == arrayY.Width());
	assert(arrayX.Height() == arrayY.Height());
	const unsigned int w = arrayX.Width();
	const unsigned int h = arrayX.Height();
	
	const int n = w*h;
	
	uint16_t xBuf[n];
	uint16_t yBuf[n];
	
	for(int j=0; j<h; j++) for(int i=0; i<w; i++){
		const int outIndex = w*j + i;
		
		if(exclude) if(exclude(j, i)){
			xBuf[outIndex] = 0;
			yBuf[outIndex] = 0;
			continue;
		}
#ifdef INTENSITY_BASED_EXCLUSION
		if(arrayX(j, i) < DRR_LOWER_THRESH){ xBuf[outIndex] = 0; yBuf[outIndex] = 0; continue; }
		if(arrayY(j, i) < XRAY_LOWER_THRESH){ xBuf[outIndex] = 0; yBuf[outIndex] = 0; continue; }
#endif
		xBuf[outIndex] = arrayX(j, i);
		yBuf[outIndex] = arrayY(j, i);
	}
	
	FILE *fptr = fopen(filenameX, "w");
	if(!fptr){ fputs("File opening error", stderr); exit(3); }
	
	fwrite(xBuf, sizeof(uint16_t), n, fptr);
	
	fclose(fptr);
	
	fptr = fopen(filenameY, "w");
	if(!fptr){ fputs("File opening error", stderr); exit(3); }
	
	fwrite(yBuf, sizeof(uint16_t), n, fptr);
	
	fclose(fptr);
	
	std::cout << "width = " << w << "\n";
}

}
