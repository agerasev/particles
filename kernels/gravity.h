#pragma once

#include <opencl.h>

float3 gravity(const Particle p0, const Particle p1, const float eps) {
	float3 r = p0.pos - p1.pos;
	float e = eps*(p0.rad + p1.rad);
	float l = sqrt(dot(r,r) + e*e);
	return -1e-4f*p1.mass*r/(l*l*l);
}
