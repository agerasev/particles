#pragma once

#include "solver.hpp"

class Octree {
	
};

class SolverCPU : public Solver {
private:
	Octree tree;
	
	
public:
	SolverCPU(int size) : Solver(size) {
		
	}
	virtual ~SolverCPU() {
		
	}
	
	virtual void solve(float dt, int steps = 1) override {
		
	}
};
