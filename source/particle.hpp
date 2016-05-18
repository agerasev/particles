#pragma once

#include <la/vec.hpp>

class Particle {
public:
	// static properties
	float mass;
	float rad;
	fvec3 color; 
	
	// dynamic properties
	fvec3 pos;
	fvec3 vel;
};
