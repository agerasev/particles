#include <head.glsl>

#include <property.glsl>

uniform sampler2D u_sprop;
uniform sampler2D u_dprop;

flat in int vf_id;

out vec4 f_color;

void main() {
	f_color = vec4(color(u_sprop, vf_id), 1.0);
}
