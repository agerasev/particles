#include <opencl.h>

#include <export/particle.h>
#include <export/deriv.h>
#include <gravity.h>
#include <export/tree.h>
#include <export/tree_depth.h>

float3 accel_plain(const int id, const Particle p0, global const float *psrc, const int size, const float eps) {
	float3 acc = (float3)(0);
	for(int i = 0; i < size; ++i) {
		const Particle p1 = part_load(i, psrc);
		acc += (id != i)*gravity(p0, p1, eps);
	}
	return acc;
}

#define GTH 0.1
float3 accel_branch(
	const int id, const Particle p, const int bptr, int *next, const float eps,
	global const int *tree, global const int *tree_link, global const float *tree_data
) {
	float3 acc = (float3)(0, 0, 0);
	*next = 0;
	
	Branch b = branch_load(0, tree + bptr);
	if(b.count <= 0) {
		return acc;
	}
	
	BranchData bd = branch_data_load(0, tree_data + b.data);
	float l = length(p.pos - bd.barycenter);
	
	if(bd.size/l < GTH) {
		return gravity_avg(p, bd.barycenter, bd.mass, eps);
	} else {
		if(b.isleaf) {
			
			for(int i = 0; i < b.count; ++i) {
				const int _id = (tree_link + b.link)[i];
				global const float *pdata = tree_data + b.data + BRANCH_DATA_FSIZE + i*PART_FSIZE;
				acc += (id != _id)*gravity(p, part_load(0, pdata), eps);
			}
		} else {
			*next = true;
		}
	}
	return acc;
}

#define RECD MAX_TREE_DEPTH
float3 accel_tree(
	const int id, const Particle p, global const float *psrc, 
	global const int *tree, global const int *tree_link, global const float *tree_data,
	const int size, const float eps
) {
	// recursion emulation
	int2 stack[RECD];
	int sptr = 0;
	
	int next;
	float3 acc = accel_branch(id, p, 0, &next, eps, tree, tree_link, tree_data);
	if(!next)
		return acc;
	
	stack[0] = (int2)(0, 0);
	
	while(true) {
		if(stack[sptr].y >= 8) {
			sptr -= 1;
			if(sptr < 0)
				break;
			continue;
		}
		int2 se = stack[sptr];
		Branch b = branch_load(0, tree + se.x);
		global const int *link = tree_link + b.link;
		int bptr = link[se.y];
		acc += accel_branch(id, p, bptr, &next, eps, tree, tree_link, tree_data);
		stack[sptr].y += 1;
		if(next) {
			sptr += 1;
			if(sptr >= RECD)
				break;
			stack[sptr] = (int2)(bptr, 0);
		}
	}
	return acc;
}

void solve_step(const int id, global const float *psrc, global float *pdst, const float3 dpos, const float3 dvel, const float dt) {
	Particle p = part_load(id, psrc);
	
	p.pos = p.pos + dt*dpos;
	p.vel = p.vel + dt*dvel;
	
	part_store(&p, id, pdst);
}

kernel void solve_plain_euler(global const float *psrc, global float *pdst, const int size, const float eps, const float dt) {
	const int id = get_global_id(0);
	
	Particle p = part_load(id, psrc);
	
	p.pos = p.pos + dt*p.vel;
	p.vel = p.vel + dt*accel_plain(id, p, psrc, size, eps);
	
	part_store(&p, id, pdst);
}

kernel void solve_plain_rk4_d(global const float *psrc, global float *deriv, const int size, const float eps) {
	const int id = get_global_id(0);
	
	Deriv d;
	Particle p = part_load(id, psrc);
	
	d.pos = p.vel;
	d.vel = accel_plain(id, p, psrc, size, eps);
	
	deriv_store(&d, id, deriv);
}

kernel void solve_tree_euler(
	global const float *psrc, global float *pdst, 
	global const int *tree, global const int *tree_link, global const float *tree_data,
	const int size, const float eps, const float dt
) {
	const int id = get_global_id(0);
	
	Particle p = part_load(id, psrc);
	
	p.pos = p.pos + dt*p.vel;
	p.vel = p.vel + dt*accel_tree(id, p, psrc, tree, tree_link, tree_data, size, eps);
	
	part_store(&p, id, pdst);
}

kernel void solve_tree_rk4_d(
	global const float *psrc, global float *deriv,
	global const int *tree, global const int *tree_link, global const float *tree_data,
	const int size, const float eps
) {
	const int id = get_global_id(0);
	
	Deriv d;
	Particle p = part_load(id, psrc);
	
	d.pos = p.vel;
	d.vel = accel_tree(id, p, psrc, tree, tree_link, tree_data, size, eps);
	
	deriv_store(&d, id, deriv);
}

kernel void solve_rk4_v_1_2(global const float *psrc, global float *pdst, global const float *deriv_1_2, const int size, const float dt) {
	const int id = get_global_id(0);
	
	Deriv d = deriv_load(id, deriv_1_2);
	
	solve_step(id, psrc, pdst, 0.5f*d.pos, 0.5f*d.vel, dt);
}

kernel void solve_rk4_v_3(global const float *psrc, global float *pdst, global const float *deriv_3, const int size, const float dt) {
	const int id = get_global_id(0);

	Deriv d = deriv_load(id, deriv_3);
	
	solve_step(id, psrc, pdst, d.pos, d.vel, dt);
}

kernel void solve_rk4_v_4(
	global const float *psrc, global float *pdst, 
	global const float *deriv_1, global const float *deriv_2, global const float *deriv_3, global const float *deriv_4, 
	const int size, const float dt
) {
	const int id = get_global_id(0);
	
	Deriv d1 = deriv_load(id, deriv_1);
	Deriv d2 = deriv_load(id, deriv_2);
	Deriv d3 = deriv_load(id, deriv_3);
	Deriv d4 = deriv_load(id, deriv_4);
	
	solve_step(
		id, psrc, pdst, 
		(d1.pos + 2.0f*d2.pos + 2.0f*d3.pos + d4.pos)/6.0f, 
		(d1.vel + 2.0f*d2.vel + 2.0f*d3.vel + d4.vel)/6.0f, 
		dt
	);
}

kernel void write_gl_tex(global const float *parts, write_only image2d_t image, const int size, const int maxts) {
	const int id = get_global_id(0);
	
	Particle p = part_load(id, parts);
	
	int2 pos;
	int lpos = 2*id;
	
	pos = (int2)(lpos%maxts, lpos/maxts);
	write_imagef(image, pos, (float4)(p.pos, 1.0));
	
	lpos += 1;
	
	pos = (int2)(lpos%maxts, lpos/maxts);
	write_imagef(image, pos, (float4)(p.vel, 1.0));
}
