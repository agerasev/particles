#include <head.glsl>
#include <property.glsl>

in vec2 vf_coord;
in float vf_shadow;
flat in int vf_id;

out vec4 f_color;

void main() {
	vec3 color = vf_shadow*color(vf_id);
	if(dot(vf_coord, vf_coord) <= 1.0) {
		gl_FragDepth = gl_FragCoord.z;
		f_color = vec4(color, 1.0);
	} else {
		gl_FragDepth = 1.0;
		f_color = vec4(color, 0.0);
	}
}
