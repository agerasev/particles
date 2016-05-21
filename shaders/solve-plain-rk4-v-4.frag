#include <head.glsl>
#include "property.glsl"
#include "gravity.glsl"

#include "solve-head.glsl"

uniform sampler2D u_deriv_1;
uniform sampler2D u_deriv_2;
uniform sampler2D u_deriv_3;
uniform sampler2D u_deriv_4;

vec3 pos_main(int id) {
	return pos(id) + u_dt/6.0*(pos_s(u_deriv_1, id) + 2.0*pos_s(u_deriv_2, id) + 2.0*pos_s(u_deriv_3, id) + pos_s(u_deriv_4, id));
}

vec3 vel_main(int id) {
	return vel(id) + u_dt/6.0*(vel_s(u_deriv_1, id) + 2.0*vel_s(u_deriv_2, id) + 2.0*vel_s(u_deriv_3, id) + vel_s(u_deriv_4, id));
}

#include "solve-main.glsl"
