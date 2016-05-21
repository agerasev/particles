#include <head.glsl>
#include "property.glsl"
#include "gravity.glsl"

#include "solve-head.glsl"

uniform sampler2D u_deriv_1_2;

vec3 pos_main(int id) {
	return pos_s(dp, id) + 0.5*u_dt*pos_s(u_deriv_1_2, id);
}

vec3 vel_main(int id) {
	return vel_s(dp, id) + 0.5*u_dt*vel_s(u_deriv_1_2, id);
}

#include "solve-main.glsl"
