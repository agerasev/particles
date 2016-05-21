#include <head.glsl>
#include "property.glsl"
#include "gravity.glsl"

#include "solve-head.glsl"
#include "tree.glsl"

vec3 pos_main(int id) {
	return vel(id);
}

vec3 vel_main(int id) {
	return grav_tree(id);
}

#include "solve-main.glsl"
