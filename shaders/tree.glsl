#pragma once

#include "head.glsl"

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


vec3 grav_part(int id, int tdi) {
	vec4 dp = get_tree_data(tdi);
	vec4 dv = get_tree_data(tdi + 1);
	return gravity_gen(id, dp.xyz, dv.xyz, dp.w, dv.w);
}

vec3 grav_branch(int id, int ti, out bool next) {
	vec3 p = pos(id);
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
		return gravity_gen(id, bp, vec3(0), bm, 0.0);
	} else {
		if(b[2] != 0) {
			vec3 acc = vec3(0);
			int i;
			for(i = 0; i < b[0]; ++i) {
				acc += grav_part(id, b[1] + 2 + 2*i);
			}
			return acc;
		} else {
			next = true;
			return vec3(0);
		}
	}
	return vec3(0);
}

#define RECD 32
vec3 grav_tree(int id) {
	// recursion emulation
	ivec2 stack[RECD];
	int sptr = 0;
	
	bool next;
	vec3 acc = grav_branch(id, 0, next);
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
		acc += grav_branch(id, nbti, next);
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
