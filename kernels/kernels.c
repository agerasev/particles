#include <opencl.h>

#include <export/particle.h>
#include <export/deriv.h>

float3 accel_plain(const int id, const Part p0, global const float *psrc, const int size, const float eps) {
	float3 acc = (float3)(0);
	for(int i = 0; i < size; ++i) {
		Part p1 = part_load(i, psrc);
		
		float3 r = p0.pos - p1.pos;
		float e = eps*(p0.rad + p1.rad);
		float l = sqrt(dot(r,r) + e*e);
		acc += (id != i)*-1e-4f*p1.mass*r/(l*l*l);
	}
	return acc;
}

kernel void solve_plain_euler(global const float *psrc, global float *pdst, const int size, const float eps, const float dt) {
	const int id = get_global_id(0);
	if(id >= size)
		return;
	
	Part p = part_load(id, psrc);
	
	p.pos = p.pos + dt*p.vel;
	p.vel = p.vel + dt*accel_plain(id, p, psrc, size, eps);
	
	part_store(&p, id, pdst);
}

kernel void solve_plain_rk4_d(global const float *psrc, global float *deriv, const int size, const float eps) {
	const int id = get_global_id(0);
	if(id >= size)
		return;
	
	Deriv d;
	Part p = part_load(id, psrc);
	
	d.pos = p.vel;
	d.vel = accel_plain(id, p, psrc, size, eps);
	
	deriv_store(&d, id, deriv);
}

kernel void solve_rk4_v_1_2(global const float *psrc, global float *pdst, global const float *deriv_1_2, const int size, const float dt) {
	const int id = get_global_id(0);
	if(id >= size)
		return;
	
	Part p = part_load(id, psrc);
	Deriv d = deriv_load(id, deriv_1_2);
	
	p.pos = p.pos + 0.5f*dt*d.pos;
	p.vel = p.vel + 0.5f*dt*d.vel;
	
	part_store(&p, id, pdst);
}

kernel void solve_rk4_v_3(global const float *psrc, global float *pdst, global const float *deriv_3, const int size, const float dt) {
	const int id = get_global_id(0);
	if(id >= size)
		return;
	
	Part p = part_load(id, psrc);
	Deriv d = deriv_load(id, deriv_3);
	
	p.pos = p.pos + dt*d.pos;
	p.vel = p.vel + dt*d.vel;
	
	part_store(&p, id, pdst);
}

kernel void solve_rk4_v_4(
	global const float *psrc, global float *pdst, 
	global const float *deriv_1, global const float *deriv_2, global const float *deriv_3, global const float *deriv_4, 
	const int size, const float dt
) {
	const int id = get_global_id(0);
	if(id >= size)
		return;
	
	Part p = part_load(id, psrc);
	Deriv d1 = deriv_load(id, deriv_1);
	Deriv d2 = deriv_load(id, deriv_2);
	Deriv d3 = deriv_load(id, deriv_3);
	Deriv d4 = deriv_load(id, deriv_4);
	
	p.pos = p.pos + dt/6.0f*(d1.pos + 2.0f*d2.pos + 2.0f*d3.pos + d4.pos);
	p.vel = p.vel + dt/6.0f*(d1.vel + 2.0f*d2.vel + 2.0f*d3.vel + d4.vel);
	
	part_store(&p, id, pdst);
}

kernel void write_gl_tex(global const float *parts, write_only image2d_t image, const int size, const int maxts) {
	const int id = get_global_id(0);
	if(id >= size)
		return;
	
	Part p = part_load(id, parts);
	
	int2 pos;
	int lpos = 2*id;
	
	pos = (int2)(lpos%maxts, lpos/maxts);
	write_imagef(image, pos, (float4)(p.pos, 1.0));
	
	lpos += 1;
	
	pos = (int2)(lpos%maxts, lpos/maxts);
	write_imagef(image, pos, (float4)(p.vel, 1.0));
}
