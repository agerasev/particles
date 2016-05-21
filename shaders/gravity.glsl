#pragma once

#include <head.glsl>
#include <property.glsl>

uniform float u_eps;

vec3 gravity(vec3 ap, vec3 bp, float bm) {
	vec3 r = ap - bp;
	float l = sqrt(dot(r,r) + u_eps*u_eps);
	return -1e-4*bm*r/(l*l*l);
}

vec3 gravity_new(int i, int j) {
	vec3 r = pos(i) - pos(j);
	float e = u_eps*(rad(i) + rad(j));
	float l = sqrt(dot(r,r) + e*e);
	return -1e-4*mass(j)*r/(l*l*l);
}

