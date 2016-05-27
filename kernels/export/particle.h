#pragma once

#include <opencl.h>

#define PART_FSIZE (2 + 2*3)
#define PART_SIZE  (PART_FSIZE*sizeof(float))

typedef struct {
	float3 pos;
	float3 vel;
	float mass;
	float rad;
} Particle;

Particle part_load(int offset, __global const float *part_data) {
	Particle part;
	__global const float *fdata = part_data + offset*PART_FSIZE;
	part.pos = vload3(0, fdata);
	part.vel = vload3(1, fdata);
	part.mass = fdata[6 + 0];
	part.rad = fdata[6 + 1];
	return part;
}

void part_store(Particle *part, int offset, __global float *part_data) {
	__global float *fdata = part_data + offset*PART_FSIZE;
	vstore3(part->pos, 0, fdata);
	vstore3(part->vel, 1, fdata);
	fdata[6 + 0] = part->mass;
	fdata[6 + 1] = part->rad;
}
