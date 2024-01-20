#ifndef Optimise_hpp
#define Optimise_hpp

#include "Data.h"

namespace Optimise {

struct OpVals {
	Params params;
	float f;
};

OpVals Scan(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), const char *fileName, bool checkFeasible);

struct PSParams {
	int N; // number of particles
	float w, phiP, phiG; // inertia weight, cognitive coefficient, social coefficient
	
	// operator overloading for easy printing of parameters
	friend std::ostream &operator<<(std::ostream &stream, const PSParams &params){
		return stream << "{" << params.N << ", " << params.w << ", " << params.phiP << ", " << params.phiG << "}";
	}
};
struct Particle {
	Particle(){}
	Particle(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), OpVals &best, const Params &params0, const Params &paramsRange, bool checkFeasible);
	
	void Update(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), OpVals &best, const PSParams &psParams, bool checkFeasible);
	
	void ReCalc(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), OpVals &best);
	
	Params v;
	OpVals current;
	OpVals myBest;
};
OpVals ParticleSwarm(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), int iterations, const PSParams &psParams, const Params &params0, const Params &paramsRange, bool checkFeasible=true, Particle *particles=nullptr);

struct PlotRanges {
	float tr1, tr2, rot;
	friend PlotRanges operator*(float f, const PlotRanges &pr){ return {f*pr.tr1, f*pr.tr2, f*pr.rot}; }
};
void Lines(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), const unsigned short &halfN, const Params &params0, const PlotRanges &ranges/*, bool checkFeasible*/, const char *fileName);

OpVals LocalSearch(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), const Params &params0, const Params &stepSize0, const char *fileName=nullptr);

double TimeDRR();
double TimeSimMetric(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )));

}

#endif /* Optimise_hpp */
