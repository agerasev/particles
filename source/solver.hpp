#pragma once

#include "gl/texture.hpp"

#include "particle.hpp"

class Solver {
public:
	const int size;
	gl::Texture *sprop = nullptr;
	gl::Texture *dprop = nullptr;
	
	float dt = 1e-2;
	int steps = 1;
	
	Solver(int size) : size(size) {}
	virtual ~Solver() = default;
	
	virtual void load(Particle parts[]) = 0;
	virtual void solve(float dt, int steps = 1) = 0;
	void solve() {
		solve(dt, steps);
	}
};
