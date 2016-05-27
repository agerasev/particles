#pragma once

#include <opencl.h>
#include <export/particle.h>

#define BRANCH_DATA_FSIZE (2*3 + 2)
#define BRANCH_DATA_SIZE (BRANCH_DATA_FSIZE*sizeof(float))

typedef struct {
	float3 center;
	float3 barycenter;
	float size;
	float mass;
} BranchData;

BranchData branch_data_load(int pos, __global const float *data) {
	BranchData bd;
	__global const float *fdata = data + pos*BRANCH_DATA_FSIZE;
	bd.center = vload3(0, fdata);
	bd.barycenter = vload3(1, fdata);
	bd.size = fdata[6 + 0];
	bd.mass = fdata[6 + 1];
	return bd;
}

void branch_data_store(BranchData *bd, int pos, __global float *data) {
	__global float *fdata = data + pos*BRANCH_DATA_FSIZE;
	vstore3(bd->center, 0, fdata);
	vstore3(bd->barycenter, 1, fdata);
	fdata[6 + 0] = bd->size;
	fdata[6 + 1] = bd->mass;
}

#define BRANCH_ISIZE 4
#define BRANCH_SIZE (BRANCH_ISIZE*sizeof(int))

typedef struct {
	int count;
	int data;
	int isleaf;
	int link;
} Branch;

Branch branch_load(int pos, __global const int *data) {
	Branch b;
	__global const int *idata = data + pos*BRANCH_ISIZE;
	b.count  = idata[0];
	b.data   = idata[1];
	b.isleaf = idata[2];
	b.link   = idata[3];
	return b;
}

void branch_store(Branch *b, int pos, __global int *data) {
	__global int *idata = data + pos*BRANCH_ISIZE;
	idata[0] = b->count;
	idata[1] = b->data;
	idata[2] = b->isleaf;
	idata[3] = b->link;
}
