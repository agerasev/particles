#include <head.glsl>
#include "property.glsl"
#include "gravity.glsl"

#include "solve-head.glsl"
#include "tree.glsl"

vec3 pos_main(int id) {
	return pos(id) + u_dt*vel(id);
}

vec3 vel_main(int id) {
	return vel(id) + u_dt*grav_tree(id);
}

#include "solve-main.glsl"
