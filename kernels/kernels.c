#include <opencl.h>

#include <export/particle.h>
#include <export/deriv.h>
#include <gravity.h>

float3 accel_plain(const int id, const Particle p0, global const float *psrc, const int size, const float eps) {
	float3 acc = (float3)(0);
	for(int i = 0; i < size; ++i) {
		const Particle p1 = part_load(i, psrc);
		acc += (id != i)*gravity(p0, p1, eps);
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
