#include <random>
#include <fstream>

#include "Render.h"
#include "Tools.h"
#include "Data.h"
#include "View.h"

void Params::Print() const {
	std::cout << "{{" << pan.x << ", " << pan.y << ", " << pan.z << "}, {" << rotation.x << ", " << rotation.y << ", " << rotation.z << "}, {" << sourceOffset.x << ", " << sourceOffset.y << "}};\n";
}

#ifndef SKIP_OPTIMISE
#include "Optimise.h"

#define BY_INDEX(arr, i) (((float *)(&arr))[i])

namespace Optimise {

OpVals Scan(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), const char *fileName, bool checkFeasible){
	
	std::ofstream outFile = std::ofstream(fileName);
	if(!outFile.is_open()){
		std::cout << "ERROR: File not opened.\n";
		return {0.0f};
	}
	
	const vec<3> &pan0 = ViewManager().PanNoInput();
	const vec<3> &rotation0 = ViewManager().RotationNoInput();
	const vec<2> &sourceOffset0 = ViewManager().SourceOffset();
	
	// for comparing with DRR
	//float rotationMatrix[4][4];
	//RotationMatrix(rotation0, rotationMatrix);
	//DrawVolumeRender(pan0, rotationMatrix, 256, 0.215f, 0.85f, DrawMode::none);
	//TakeDRR();
	//CopyDRR();
	
	
	unsigned long T = UTime();
	
	OpVals opVals, best;
	best.f = 2.0f;
	
	const unsigned long n = 49;
	const float nm1Inv = 1.0/(float)(n - 1);
	const vec<3> panRange = {1.0f, 1.0f, 15.0f};
	//const vec<3> rotRange = {0.01f, 0.1f, 0.05f};
	//const vec<2> soRange = {5.0f, 5.0f};
	
	unsigned long omissions = 0;
	
	//const float two = 2.0f;
	
	for(int iY=0; iY<n; iY++){
		for(int iX=0; iX<n; iX++){
			//for(int iZ=0; iZ<n; iZ++) for(int iR=0; iR<n; iR++) for(int iE=0; iE<n; iE++) for(int iA=0; iA<n; iA++) {
			opVals.params = {
				{
					pan0.x - 0.5f*panRange.x + (float)iX*panRange.x*nm1Inv,
					pan0.y - 0.5f*panRange.y + (float)iY*panRange.y*nm1Inv,
					pan0.z/* - 0.5f*panRange.z + (float)iZ*panRange.z*nm1Inv*/
				},
				{
					rotation0.x/* - 0.5f*rotRange.x + (float)iR*rotRange.x*nm1Inv*/,
					rotation0.y/* - 0.5f*rotRange.y + (float)iE*rotRange.y*nm1Inv*/,
					rotation0.z/* - 0.5f*rotRange.z + (float)iA*rotRange.z*nm1Inv*/
				},
				{
					sourceOffset0.x/* - 0.5f*soRange.x + (float)iSX*soRange.x*nm1Inv*/,
					sourceOffset0.y/* - 0.5f*soRange.y + (float)iSY*soRange.y*nm1Inv*/
				}
			};
			if(checkFeasible){
				UpdateSourceOffset(opVals.params.sourceOffset);
				Data().Transform3dPoints(opVals.params.pan, Render::RotationMatrix(opVals.params.rotation));
				Data().CalcEnlargedPointPositions();
				
				if(Data().OutsideEnlarged()){
					outFile << "2.0 ";
					//outFile.write((char *)&two, sizeof(float));
					omissions++;
					continue;
				}
			}
			
			opVals.f = Render::F(similarityMetric, opVals.params);
			if(opVals.f < best.f) best = opVals;
			
			outFile << opVals.f << " ";
			//outFile.write((char *)&opVals.f, sizeof(float));
			//}
			std::cout << "- done iY = " << iX << " / " << n - 1 << ",\n";
		}
		outFile << "\n";
		std::cout << "    -> Done iX = " << iY << " / " << n - 1 << ".\n";
	}
	
	T = UTime() - T;
	
	const unsigned long tot = n*n/**n*n*n*n*n*n*/;
	std::cout << "Omitted " << omissions << " / " << tot << " = " << 100.0*(double)omissions/(double)tot << "%\n";
	
	std::cout << "Ran in " << (double)T*1e-6 << " s = " << (double)T*1e-6/60.0f << " mins.\n";
	
	std::cout << "Range covered = " << (vec<2>){panRange.x, panRange.y} << " centered on " << (vec<2>){pan0.x, pan0.y} << "\n";
	
	outFile.close();
	
	return best;
}

Particle::Particle(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), OpVals &best, const Params &params0, const Params &paramsRange, bool checkFeasible){
	Params paramsOff;
	for(int p=0; p<8; p++) BY_INDEX(paramsOff, p) = Uniform(-0.5f*BY_INDEX(paramsRange, p), 0.5f*BY_INDEX(paramsRange, p));
	
	if(checkFeasible){
		float f = 2.0f;
		do {
			f *= 0.5f;
			if(f < 0.01f){
				std::cout << "f now very small.\n";
				current.params = params0;
				break;
			}
			current.params = params0 + f*paramsOff;
			// check for feasibility
			UpdateSourceOffset(current.params.sourceOffset);
			Data().Transform3dPoints(current.params.pan, Render::RotationMatrix(current.params.rotation)/*, current.params.sourceOffset*/);
			Data().CalcEnlargedPointPositions();
		} while(Data().OutsideEnlarged());
	}
	
	for(int p=0; p<8; p++) BY_INDEX(v, p) = Uniform(-BY_INDEX(paramsRange, p), BY_INDEX(paramsRange, p));
	
	myBest = {current.params, Render::F(similarityMetric, current.params)};
	if(myBest.f < best.f) best = myBest;
}
void Particle::Update(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), OpVals &best, const PSParams &psParams, bool checkFeasible){
	Params rPs = Params::FromFunction([](){ return Uniform(0.0f, 1.0f); });
	Params rGs = Params::FromFunction([](){ return Uniform(0.0f, 1.0f); });
	v = psParams.w*v + psParams.phiP*rPs*(myBest.params - current.params) + psParams.phiG*rGs*(best.params - current.params);
	
	const Params old = current.params;
	current.params += v;
	
	if(checkFeasible){
		// check for feasibility
		UpdateSourceOffset(current.params.sourceOffset);
		Data().Transform3dPoints(current.params.pan, Render::RotationMatrix(current.params.rotation)/*, current.params.sourceOffset*/);
		Data().CalcEnlargedPointPositions();
		if(Data().OutsideEnlarged()){
			current.params = old;
			//v = {{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}; // making sure velocity is consistent with positional change, even if constraints active
			v *= -0.5f;
			return;
		}
	}
	// updating my and global optima if appropriate
	current.f = Render::F(similarityMetric, current.params);
	if(current.f < myBest.f){
		myBest = current;
		if(myBest.f < best.f) best = myBest;
	}
}
void Particle::ReCalc(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), OpVals &best){
	current.f = Render::F(similarityMetric, current.params);
	if(current.f < myBest.f){
		myBest = current;
		if(myBest.f < best.f) best = myBest;
	}
}
OpVals ParticleSwarm(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), int iterations, const PSParams &psParams, const Params &params0, const Params &paramsRange, bool checkFeasible, Particle *particles/*, const Print &print, int *counts, float *bests*/){
	
	Render::SetBoundaryOffset(20);
	Render::SetMoveBoundaries(true);
	Render::ShouldSaveRects("initial");
	Render::Render(similarityMetric); // set boundaries using initial alignment
	Render::SetMoveBoundaries(false);
	
	OpVals best = {ViewManager().params, Render::F(similarityMetric, ViewManager().params)};
	
	std::cout << "Initialised at: F = " << best.f << " at " <<  best.params << "\n";
	
	if(checkFeasible){
		// check for feasibility of initial point
		UpdateSourceOffset(params0.sourceOffset);
		Data().Transform3dPoints(params0.pan, Render::RotationMatrix(params0.rotation)/*, params0.sourceOffset*/);
		Data().CalcEnlargedPointPositions();
		if(Data().OutsideEnlarged()){
			std::cout << "Error: Initial point not feasible.\n";
			return {0.0f};
		}
	}
	
	const bool alloc = !particles;
	if(alloc) particles = (Particle *)malloc(psParams.N * sizeof(Particle));
	
	//evaluations = 0;
	
	unsigned long T = UTime();
	
	for(int i=0; i<psParams.N; i++) particles[i] = Particle(similarityMetric, best, params0, paramsRange, checkFeasible);
	SetViewParams(best.params);
	Render::Render(similarityMetric);
	
	/*if(print == Print::avBest){
	 bests[evaluations/500 - 1] += best.f;
	 counts[evaluations/500 - 1]++;
	 }
	 
	 if(print == Print::avBest) if(!counts || !bests) std::cout << "ERROR: No file stream provided.\n";
	 std::ofstream file;
	 if(print == Print::all || print == Print::best){
	 file = std::ofstream(print == Print::all ? fileNameAll : fileNameBest);
	 if(!file.is_open()) std::cout << "ERROR: File not opened.\n";
	 }*/
	
	//XRay().grad_k = 30;
	for(int it=0; it<iterations; it++){
		for(int i=0; i<psParams.N; i++){
			particles[i].Update(similarityMetric, best, psParams, checkFeasible);
			//if(print == Print::all) file << particles[i].current.x[0] << " " << particles[i].current.x[1] << " " << particles[i].current.f << " ";
		}
		
		/*if(print == Print::all) file << best.x[0] << " " << best.x[1] << " " << best.f << "\n";
		 else if(print == Print::best) file << evaluations << " " << best.f << "\n";
		 else if(print == Print::avBest){
		 bests[evaluations/500 - 1] += best.f;
		 counts[evaluations/500 - 1]++;
		 }*/
		
		SetViewParams(best.params);
		Render::Render(similarityMetric);
		std::cout << "It " << it << ": best = " << best.f << " at " <<  best.params << "\n";
		/*
		 if(!((it + 1) % ((iterations - 5) / 2))){
		 XRay().grad_k -= 12;
		 best.f = 2.0f;
		 for(int i=0; i<psParams.N; i++) particles[i].ReCalc(similarityMetric, best);
		 }
		 */
	}
	
	Render::ShouldSaveRects("final");
	Render::Render(similarityMetric);
	
	T = UTime() - T;
	
	std::cout << "Ran in " << (float)T*1e-6f << " s = " << (float)T*1e-6f/60.0f << " mins.\n";
	
	std::cout << "PS params = " << psParams << "\n";
	
	std::cout << "Initial centre = " << params0 << "; range = " << paramsRange << "\n";
	
	//if(print == Print::all || print == Print::best) file.close();
	
	if(alloc) free(particles);
	
	return best;
}

void Lines(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int , int )), const unsigned short &halfN, const Params &params0, const PlotRanges &ranges/*, bool checkFeasible*/, const char *fileName){
	
	const Params paramsRange = {{ranges.tr1, ranges.tr1, ranges.tr2}, {ranges.rot, ranges.rot, ranges.rot}, {ranges.tr2, ranges.tr2}};
	
	const int N = 1 + halfN + halfN;
	const float nM1Inv = 1.0f/(float)(N - 1);
	static const int D = 8;
	
	const float centreF = Render::F(similarityMetric, params0);
	
	FILE *fptr = fopen(fileName, "w");
	if(!fptr){ fputs("File opening error", stderr); exit(3); }
	
	// 11 + D*4 byte file header
	// 4; int: N
	// 4; int: D
	// D*4; floats: parameter centres
	// 3; floats: ranges
	fwrite(&N, sizeof(int), 1, fptr);
	fwrite(&D, sizeof(int), 1, fptr);
	fwrite(&params0, sizeof(float), D, fptr);
	fwrite(&ranges, sizeof(float), 3, fptr);
	
	// data
	for(int p=0; p<8; p++){
		Params pCopy = params0;
		BY_INDEX(pCopy, p) -= 0.5f*BY_INDEX(paramsRange, p);
		const float delta = BY_INDEX(paramsRange, p)*nM1Inv;
		for(int i=0; i<N; i++){
			if(i == halfN){ // the centre
				fwrite(&centreF, sizeof(float), 1, fptr);
			} else { // not the centre
				const float f = Render::F(similarityMetric, pCopy);
				fwrite(&f, sizeof(float), 1, fptr);
			}
			BY_INDEX(pCopy, p) += delta;
		}
	}
	
	fclose(fptr);
}

double TimeDRR(){
	Params params;
	
	glViewport(0, 0, Render::PortWidth(), Render::PortHeight());
	glClearColor(1.0, 1.0, 1.0, 1.0);
	
	unsigned long T = UTime();
	for(int i=0; i<1000; i++){
		params = Params::FromFunction([](){ return Uniform(-1.0f, 1.0f); });
		UpdateSourceOffset(params.sourceOffset);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		Render::DrawVolumeRender(params.pan, Render::RotationMatrix(params.rotation), ViewManager().SamplesN(), ViewManager().Range(), ViewManager().Centre(), DrawMode::deep_drr_only/*, params.sourceOffset*/);
		Render::TakeDRR();
	}
	return (double)(UTime() - T)*1.0e-6;
}
double TimeSimMetric(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int, int))){
	Params params;
	
	unsigned long T = UTime();
	for(int i=0; i<1000; i++){
		params = Params::FromFunction([](){ return Uniform(-1.0f, 1.0f); });
		Render::F(similarityMetric, params);
	}
	return (double)(UTime() - T)*1.0e-6;
}

OpVals LocalSearch(float (*similarityMetric)(const Array2D<uint16_t> &, const Array2D<uint16_t> &, bool (*)(int, int)), const Params &params0, const Params &stepSize0, const char *fileName){
	
	std::ofstream outFile;
	if(fileName){
		outFile = std::ofstream(fileName);
		if(!outFile.is_open()){
			std::cout << "ERROR: File not opened.\n";
			return {0.0f};
		}
	}
	
	//unsigned long T;
	
	OpVals adjacent, base, bestAdjacent;
	
	base.params = params0;
	base.f = Render::F(similarityMetric, base.params);
	
	Params stepSize = stepSize0;
	
	int reductions = 0;
	const int maxReductions = 28;
	const float reductionRatio = 0.8f;
	const int maxIt = 500;
	int i;
	//unsigned long T = UTime();
	
	for(i=0; i<maxIt; i++){
		bestAdjacent = base;
		bool improvement = false;
		for(int p=0; p<8; p++){
			adjacent.params = base.params;
			BY_INDEX(adjacent.params, p) += BY_INDEX(stepSize, p);
			adjacent.f = Render::F(similarityMetric, adjacent.params);
			if(adjacent.f < bestAdjacent.f){
				bestAdjacent = adjacent;
				improvement = true;
			}
			
			adjacent.params = base.params;
			BY_INDEX(adjacent.params, p) -= BY_INDEX(stepSize, p);
			adjacent.f = Render::F(similarityMetric, adjacent.params);
			if(adjacent.f < bestAdjacent.f){
				bestAdjacent = adjacent;
				improvement = true;
			}
		}
		
		if(!improvement){
			if(reductions >= maxReductions) break;
			stepSize *= reductionRatio;
			reductions++;
		} else {
			base = bestAdjacent;
			if(fileName){
				outFile << base.f << " ";
				for(int p=0; p<8; p++) outFile << BY_INDEX(base.params, p) << " ";
				outFile << "\n";
			}
		}
	}
	//T = UTime() - T;
	//std::cout << "Did " << i << " iterations in " << (float)T*1e-6f << " seconds.\n";
	
	if(fileName) outFile.close();
	
	return base;
}

}

#endif
