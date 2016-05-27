#include <head.glsl>

#include <property.glsl>

uniform ivec2 u_wh;
uniform float u_f;

flat in int vf_id;
flat in float vf_rad;
flat in vec2 vf_center;

out vec4 f_color;

void main() {
	float r = float(u_wh.y)*vf_rad;
	if(r >= 2.0)
		discard;
	
	float f = u_f, rf = 1.0/f;
	float m = 0.5*r * 3.1415*rf*((1.0 + rf)*log(1.0 + f) - 1.0);
	
	vec2 o = vec2(1,1) - abs(gl_FragCoord.xy - 0.5*(vec2(1,1) + vf_center)*vec2(u_wh));
	
	f_color = vec4(color(vf_id), m*o.x*o.y);
	
}
