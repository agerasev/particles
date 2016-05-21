#include <head.glsl>

#include "solve-head.glsl"

void main(void) {
	int lfc = restore_id(ivec2(gl_FragCoord.xy));
	int id = lfc/2;
	int var = lfc%2;
	
	if(id < u_count) {
		vec3 res;
		if(var == 0) {
			res = pos_main(id);
		} else if(var == 1) {
			res = vel_main(id);
		}
		f_FragColor = vec4(res, 1.0);
	}
}
