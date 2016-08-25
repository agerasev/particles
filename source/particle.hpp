#pragma once

#include <la/vec.hpp>

#include "opencl.hpp"
#include <export/particle.h>

class ParticleCPU {
public:
	int id;
	
	// static properties
	float mass;
	float rad;
	fvec3 color; 
	
	// dynamic properties
	fvec3 pos;
	fvec3 vel;
	
	void update() {
		rad = 2e-2*pow(mass, 1.0/3.0);
	}
	
	void store(Particle *p) const {
		p->mass = mass;
		p->rad = rad;
		p->pos = pos;
		p->vel = vel;
	}
	
	void load(const Particle *p) {
		mass = p->mass;
		rad = p->rad;
		pos = p->pos;
		vel = p->vel;
	}
};
