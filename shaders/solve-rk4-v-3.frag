#include <head.glsl>
#include "property.glsl"
#include "gravity.glsl"

#include "solve-head.glsl"

uniform sampler2D u_deriv_3;

vec3 pos_main(int id) {
	return pos_s(dp, id) + u_dt*pos_s(u_deriv_3, id);
}

vec3 vel_main(int id) {
	return vel_s(dp, id) + u_dt*vel_s(u_deriv_3, id);
}

#include "solve-main.glsl"
