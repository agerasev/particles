#pragma once

#include <opencl.h>

#include <export/particle.h>

#define G_CONST 1e-4f

float3 gravity(const Particle p0, const Particle p1) {
	float3 r = p0.pos - p1.pos;
	float e = (p0.rad + p1.rad);
	float l = sqrt(dot(r,r) + e*e);
	return -G_CONST*p1.mass*r/(l*l*l);
}

float3 gravity_avg(const Particle p, const float3 pos, const float mass) {
	float3 r = p.pos - pos;
	float e = p.rad;
	float l = sqrt(dot(r,r) + e*e);
	return -G_CONST*mass*r/(l*l*l);
}
