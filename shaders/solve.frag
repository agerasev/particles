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
		float im = imass(sp, id);
		vec3 osc = -im*pos(dp, id);
		vec3 coll = vec3(0.0, 0.0, 0.0);
		int i;
		for(i = 0; i < u_count; ++i) {
			if(i == id) {
				continue;
			}
			vec3 rpos = pos(dp, id) - pos(dp, i);
			float rads = rad(sp, id) + rad(sp, i);
			float dist = length(rpos);
			if(dist < rads) {
				coll += 1e3*im*(rads - dist)*rpos/dist;
			} 
		}
		res = vel(dp, id) + u_dt*(osc + coll);
	}
	f_FragColor = vec4(res, 1.0);
}
