#pragma once

#include <opencl.h>

#define DERIV_FSIZE (2*3)
#define DERIV_SIZE  (DERIV_FSIZE*sizeof(float))

typedef struct {
	float3 pos;
	float3 vel;
} Deriv;

Deriv deriv_load(int offset, __global const float *deriv_data) {
	Deriv deriv;
	__global const float *fdata = deriv_data + offset*DERIV_FSIZE;
	deriv.pos = vload3(0, fdata);
	deriv.vel = vload3(1, fdata);
	return deriv;
}

void deriv_store(Deriv *deriv, int offset, __global float *deriv_data) {
	__global float *fdata = deriv_data + offset*DERIV_FSIZE;
	vstore3(deriv->pos, 0, fdata);
	vstore3(deriv->vel, 1, fdata);
}
