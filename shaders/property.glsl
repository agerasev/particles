#pragma once
#include <head.glsl>

float rad(sampler2D sp, int id) {
	return texelFetch(sp, ivec2(id, 0), 0).r;
}

float mass(sampler2D sp, int id) {
	return texelFetch(sp, ivec2(id, 0), 0).w;
}

vec3 pos(sampler2D dp, int id) {
	return texelFetch(dp, ivec2(id, 0), 0).xyz;
}

vec3 vel(sampler2D dp, int id) {
	return texelFetch(dp, ivec2(id, 1), 0).xyz;
}
