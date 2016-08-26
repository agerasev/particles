#pragma once

#include <opencl.h>

#include <export/particle.h>

#define GRAV_CONST 1e-4f

#define WEIGHT_POWER 1e0f

#define PRES_CONST 1e-1f
#define VISC_CONST 1e-3f

float3 gravity(const Particle p0, const Particle p1) {
	float3 r = p0.pos - p1.pos;
	float s = (p0.rad + p1.rad);
	float l2 = dot(r,r);
	float l = sqrt(l2);
	float ls = sqrt(l2 + s*s);
	float lr = l/s;
	float w = exp(-WEIGHT_POWER*lr*lr);
	return 
		- GRAV_CONST*p1.mass*r/(ls*ls*ls) +
		w*(PRES_CONST*r/ls + VISC_CONST*(p1.vel - p0.vel));
}

float3 gravity_avg(const Particle p, const float3 pos, const float mass) {
	float3 r = p.pos - pos;
	float e = p.rad;
	float l = sqrt(dot(r,r) + e*e);
	return -GRAV_CONST*mass*r/(l*l*l);
}
