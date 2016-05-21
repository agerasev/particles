#include <head.glsl>
#include "property.glsl"
#include "gravity.glsl"

#include "solve-head.glsl"

vec3 pos_main(int id) {
	return pos(id) + u_dt*vel(id);
}

vec3 vel_main(int id) {
	vec3 acc = vec3(0);
	int i;
	for(i = 0; i < u_count; ++i) {
		acc += gravity_new(id, i);
	}
	return vel(id) + u_dt*acc;
}

#include "solve-main.glsl"
