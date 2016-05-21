#pragma once

#include <head.glsl>

uniform sampler2D u_sprop;
uniform sampler2D u_dprop;

#define sp u_sprop
#define dp u_dprop

float rad_s(sampler2D smp, int id) {
	return texelFetch(smp, split_id(2*id), 0).r;
}

float mass_s(sampler2D smp, int id) {
	return texelFetch(smp, split_id(2*id), 0).w;
}

vec3 pos_s(sampler2D smp, int id) {
	return texelFetch(smp, split_id(2*id), 0).xyz;
}

vec3 vel_s(sampler2D smp, int id) {
	return texelFetch(smp, split_id(2*id + 1), 0).xyz;
}

vec3 color_s(sampler2D smp, int id) {
	return texelFetch(smp, split_id(2*id + 1), 0).rgb;
}

float rad(int id) {
	return rad_s(sp, id);
}

float mass(int id) {
	return mass_s(sp, id);
}

vec3 pos(int id) {
	return pos_s(dp, id);
}

vec3 vel(int id) {
	return vel_s(dp, id);
}

vec3 color(int id) {
	return color_s(sp, id);
}
