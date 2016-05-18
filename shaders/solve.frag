#include <head.glsl>
#include "property.glsl"

uniform sampler2D u_sprop;
uniform sampler2D u_dprop;

#define sp u_sprop
#define dp u_dprop

uniform float u_dt;
uniform int u_count;

out vec4 f_FragColor;

void main(void) {
	int id = int(gl_FragCoord.x);
	int var = int(gl_FragCoord.y);
	vec3 res = vec3(0.0, 0.0, 0.0);
	if(var == 0) {
		// position
		res = pos(dp, id) + u_dt*vel(dp, id);
	} else if (var == 1) {
		// velocity
		vec3 acc = vec3(0.0, 0.0, 0.0);
		int i;
		for(i = 0; i < u_count; ++i) {
			if(i == id) {
				continue;
			}
			float m = mass(sp, id);
			vec3 r = pos(dp, id) - pos(dp, i);
			float eps = rad(sp, id) + rad(sp, i);
			float l = sqrt(dot(r,r) + eps*eps);
			acc -= 1e-4*m*r/(l*l*l);
		}
		res = vel(dp, id) + u_dt*acc;
	}
	f_FragColor = vec4(res, 1.0);
}
