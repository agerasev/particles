#pragma once

#include <opencl.h>

#define PROP_INT_SIZE
#define PROP_FLOAT_SIZE
#define PROP_SIZE

struct Prop {
	int count;
	float g; // Gravity constant
	float gth; // Gravity Tree tHreshold
};

Prop prop_load(__global const char *prop_data) {
	Prop prop;
	prop.G = prop_data[0];
	prop.GTH = prop_data[1];
	return deriv;
}

void deriv_store(const Prop *prop, __global char *prop_data) {
	prop_data[0] = prop->G;
	prop_data[1] = prop->GTH;
}
