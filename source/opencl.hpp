#pragma once

#define OPENCL_H

#define __global
#define __local
#define __constant

typedef unsigned char uchar;
typedef unsigned int  uint;

#include <la/vec.hpp>

typedef fvec2 float2;
typedef fvec3 float3;
typedef fvec4 float4;
typedef ivec2 uint2;
typedef ivec3 uint3;
typedef ivec4 uint4;
typedef ivec2 int2;
typedef ivec3 int3;
typedef ivec4 int4;

template <typename T>
vec<T,2> vload2(int offset, const T *data) {
	vec<T,2> ret;
	const T *ptr = data + offset*2;
	ret[0] = ptr[0];
	ret[1] = ptr[1];
	return ret;
}

template <typename T>
vec<T,3> vload3(int offset, const T *data) {
	vec<T,3> ret;
	const T *ptr = data + offset*3;
	ret[0] = ptr[0];
	ret[1] = ptr[1];
	ret[2] = ptr[2];
	return ret;
}

template <typename T>
vec<T,4> vload4(int offset, const T *data) {
	vec<T,4> ret;
	const T *ptr = data + offset*4;
	ret[0] = ptr[0];
	ret[1] = ptr[1];
	ret[2] = ptr[2];
	ret[3] = ptr[3];
	return ret;
}

template <typename T>
void vstore2(vec<T,2> vector, int offset, T *data) {
	T *ptr = data + offset*2;
	ptr[0] = vector[0];
	ptr[1] = vector[1];
}

template <typename T>
void vstore3(vec<T,3> vector, int offset, T *data) {
	T *ptr = data + offset*3;
	ptr[0] = vector[0];
	ptr[1] = vector[1];
	ptr[2] = vector[2];
}

template <typename T>
void vstore4(vec<T,4> vector, int offset, T *data) {
	T *ptr = data + offset*4;
	ptr[0] = vector[0];
	ptr[1] = vector[1];
	ptr[2] = vector[2];
	ptr[3] = vector[3];
}
