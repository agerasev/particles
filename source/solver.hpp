#pragma once

#include "gl/texture.hpp"

#include "particle.hpp"

#include "opencl.hpp"
#include <export/particle.h>

class Solver {
public:
	static const int
	RK4 = (1 << 0);
	struct Properties {
		bool RK4 = false;
		float G = 1e-4f;
	};
	
public:
	int size;
	
	int features;
	
	float dt = 1e-2;
	
	float gth = 0.1;
	
	Solver(int size, int features) : size(size), features(features) {}
	virtual ~Solver() = default;
	
	virtual void load(ParticleCPU parts[]) = 0;
	virtual void store(const ParticleCPU parts[]) = 0;
	
	virtual void solve(float dt) = 0;
	void solve() {
		solve(dt);
	}
};
