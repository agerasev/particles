#pragma once

#include <vector>

#include "solver.hpp"
#include "particle.hpp"

class PartBuf : public Particle {
private:
	void _copy(const Particle &p) {
		color = p.color;
		mass = p.mass;
		rad = p.rad;
		pos = p.pos;
		vel = p.vel;
	}

public:
	static const int bufs = 1;
	fvec3 bpos[bufs], bvel[bufs];
	PartBuf() = default;
	PartBuf(const Particle &p) {
		_copy(p);
	}
	PartBuf &operator = (const Particle &p) {
		_copy(p);
		return *this;
	}
};

class Octree {
	
};

class SolverCPU : public Solver {
private:
	Octree tree;
	PartBuf *parts;
	
public:
	SolverCPU(int size) : Solver(size) {
		parts = new PartBuf[size];
		sprop = new gl::Texture();
		sprop->init(2, ivec2(size, sph).data(), gl::Texture::RGBA32F);
		dprop = new gl::Texture();
		dprop->init(2, ivec2(size, dph).data(), gl::Texture::RGBA32F);
	}
	virtual ~SolverCPU() {
		delete dprop;
		delete sprop;
		delete[] parts;
	}
	
	virtual void load(Particle parts[]) {
		for(int i = 0; i < size; ++i) {
			this->parts[i] = parts[i];
		}
	}
	
	virtual void solve(float dt, int steps = 1) override {
		for(int i = 0; i < size; ++i) {
			// position
			parts[i].bpos[0] = parts[i].pos + dt*parts[i].vel;
			// velocity
			fvec3 acc = nullfvec3;
			for(int j = 0; j < size; ++j) {
				if(i == j)
					continue;
				float m = parts[j].mass;
				fvec3 r = parts[i].pos - parts[j].pos;
				float eps = parts[i].rad + parts[j].rad;
				float l = sqrt(dot(r,r) + eps*eps);
				acc -= 1e-4*m*r/(l*l*l);
			}
			parts[i].bvel[0] = parts[i].vel + dt*acc;
		}
		for(int i = 0; i < size; ++i) {
			parts[i].pos = parts[i].bpos[0];
			parts[i].vel = parts[i].bvel[0];
		}
		loadTex(parts);
	}
};
