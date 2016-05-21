#include <head.glsl>
#include "property.glsl"
#include "gravity.glsl"

uniform isampler2D u_tree;
uniform isampler2D u_tree_link;
uniform sampler2D u_tree_data;

uniform float u_gth;

ivec4 get_branch(int ti) {
	return texelFetch(u_tree, split_id(ti), 0);
}

ivec4 get_tree_link(int tli) {
	return texelFetch(u_tree_link, split_id(tli), 0);
}

vec4 get_tree_data(int tdi) {
	return texelFetch(u_tree_data, split_id(tdi), 0);
}

uniform float u_dt;
uniform int u_count;

out vec4 f_FragColor;

vec3 grav_part(vec3 p, int tdi) {
	vec4 d = get_tree_data(tdi);
	return gravity(p, d.xyz, d.w);
}

vec3 grav_branch(vec3 p, int ti, out bool next) {
	next = false;
	
	ivec4 b = get_branch(ti);
	if(b[0] <= 0) {
		return vec3(0);
	}
	
	vec4 bpd = get_tree_data(b[1]);
	vec4 bcd = get_tree_data(b[1] + 1);
	vec3 bp = bpd.xyz;
	float bm = bpd.w;
	float l = length(p - bp);
	
	if(bcd.w/l < u_gth) {
		return gravity(p, bp, bm);
	} else {
		if(b[2] != 0) {
			vec3 acc = vec3(0);
			int i;
			for(i = 0; i < b[0]; ++i) {
				acc += grav_part(p, b[1] + 2 + i);
			}
			return acc;
		} else {
			next = true;
			return vec3(0);
		}
	}
	return vec3(0);
}

#define RECD 16
vec3 grav_tree(vec3 p) {
	// recursion emulation
	ivec2 stack[RECD];
	int sptr = 0;
	
	bool next;
	vec3 acc = grav_branch(p, 0, next);
	if(!next) {
		return acc;
	}
	stack[0] = ivec2(0, 0);
	
	while(true) {
		if(stack[sptr].y >= 8) {
			sptr -= 1;
			if(sptr < 0)
				break;
			continue;
		}
		ivec2 se = stack[sptr];
		ivec4 lnk = get_tree_link(get_branch(se.x).w + int(se.y >= 4));
		int lnkb[4] = int[4](lnk[0], lnk[1], lnk[2], lnk[3]);
		int nbti = lnkb[se.y % 4];
		acc += grav_branch(p, nbti, next);
		stack[sptr].y += 1;
		if(next) {
			sptr += 1;
			if(sptr >= RECD)
				break;
			stack[sptr] = ivec2(nbti, 0);
		}
	}
	return acc;
}

void main(void) {
	int lfc = restore_id(ivec2(gl_FragCoord.xy));
	int id = lfc/2;
	int var = lfc%2;
	
	if(id < u_count) {
		vec3 res;
		if(var == 0) {
			// position
			res = pos(id) + u_dt*vel(id);
		} else if(var == 1) {
			// velocity
			res = vel(id) + u_dt*grav_tree(pos(id));
		}
		f_FragColor = vec4(res, 1.0);
	}
}
