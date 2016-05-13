#include <head.glsl>

uniform sampler2D u_sprop;

in vec2 vf_coord;
in float vf_shadow;
flat in int vf_id;

out vec4 f_color;

void main() {
	vec3 color = vf_shadow*texelFetch(u_sprop, ivec2(vf_id, 1), 0).rgb;
	if(dot(vf_coord, vf_coord) <= 1.0) {
		f_color = vec4(color, 1.0);
	} else {
		f_color = vec4(color, 0.0);
	}
}
