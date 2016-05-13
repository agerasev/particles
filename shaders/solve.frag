#version 130

uniform sampler2D u_sprop;
uniform sampler2D u_dprop;
uniform float u_dt;
uniform int u_count;

out vec4 f_FragColor;

float rad(int id) {
	return texelFetch(u_sprop, ivec2(id, 0), 0).r;
}

float imass(int id) {
	return texelFetch(u_sprop, ivec2(id, 0), 0).w;
}

vec3 pos(int id) {
	return texelFetch(u_dprop, ivec2(id, 0), 0).xyz;
}

vec3 vel(int id) {
	return texelFetch(u_dprop, ivec2(id, 1), 0).xyz;
}


void main(void) {
	int id = int(gl_FragCoord.x);
	int var = int(gl_FragCoord.y);
	vec3 res = vec3(0.0, 0.0, 0.0);
	if(var == 0) {
		// position
		res = pos(id) + u_dt*vel(id);
	} else if (var == 1) {
		// velocity
		float im = imass(id);
		vec3 osc = -im*pos(id);
		vec3 coll = vec3(0.0, 0.0, 0.0);
		int i;
		for(i = 0; i < u_count; ++i) {
			if(i == id) {
				continue;
			}
			vec3 rpos = pos(id) - pos(i);
			float rads = rad(id) + rad(i);
			float dist = length(rpos);
			if(dist < rads) {
				coll += 1e3*im*(rads - dist)*rpos/dist;
			} 
		}
		res = vel(id) + u_dt*(osc + coll);
	}
	f_FragColor = vec4(res, 1.0);
}
