#include <head.glsl>
#include <property.glsl>

uniform int u_h;
uniform float u_f;

in vec2 vf_coord;
flat in float vf_rad;
flat in int vf_id;

out vec4 f_color;

void main() {
	if(float(u_h)*vf_rad < 2.0)
		discard;
	
	vec3 color = color(vf_id);
	float r = dot(vf_coord, vf_coord);
	float f = u_f, rf = 1.0/f;
	f_color = vec4(color, (1.0 + rf)/(1.0 + f*r) - rf);
}
