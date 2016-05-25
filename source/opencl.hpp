#pragma once

#define OPENCL_H

#define __global
#define __local
#define __constant

typedef unsigned char uchar;
typedef unsigned int  uint;

template <typename T, int N>
struct _CL_TN {
	T data[N];
};

typedef _CL_TN<float,2> float2;
typedef _CL_TN<float,3> float3;
typedef _CL_TN<float,4> float4;
typedef _CL_TN<uint,2> uint2;
typedef _CL_TN<uint,3> uint3;
typedef _CL_TN<uint,4> uint4;
typedef _CL_TN<int,2> int2;
typedef _CL_TN<int,3> int3;
typedef _CL_TN<int,4> int4;

template <typename T>
_CL_TN<T,2> vload2(int offset, const T *data) {
	_CL_TN<T,2> ret;
	const T *ptr = data + offset*2;
	ret.data[0] = ptr[0];
	ret.data[1] = ptr[1];
	return ret;
}

template <typename T>
_CL_TN<T,3> vload3(int offset, const T *data) {
	_CL_TN<T,3> ret;
	const T *ptr = data + offset*3;
	ret.data[0] = ptr[0];
	ret.data[1] = ptr[1];
	ret.data[2] = ptr[2];
	return ret;
}

template <typename T>
_CL_TN<T,4> vload4(int offset, const T *data) {
	_CL_TN<T,4> ret;
	const T *ptr = data + offset*4;
	ret.data[0] = ptr[0];
	ret.data[1] = ptr[1];
	ret.data[2] = ptr[2];
	ret.data[3] = ptr[3];
	return ret;
}

template <typename T>
void vstore2(_CL_TN<T,2> vector, int offset, T *data) {
	T *ptr = data + offset*2;
	ptr[0] = vector.data[0];
	ptr[1] = vector.data[1];
}

template <typename T>
void vstore3(_CL_TN<T,3> vector, int offset, T *data) {
	T *ptr = data + offset*3;
	ptr[0] = vector.data[0];
	ptr[1] = vector.data[1];
	ptr[2] = vector.data[2];
}

template <typename T>
void vstore4(_CL_TN<T,4> vector, int offset, T *data) {
	T *ptr = data + offset*4;
	ptr[0] = vector.data[0];
	ptr[1] = vector.data[1];
	ptr[2] = vector.data[2];
	ptr[3] = vector.data[3];
}
