#include <opencl.h>

#include <export/particle.h>

kernel void solve_plain_euler(
	global const float *sp, global float *dp,
	const int size, const float eps, const float dt
) {
	const int id = get_global_id(0);
	if(id >= size)
		return;
	
	Part p0 = part_load(id, sp);
	
	float3 acc = (float3)(0);
	for(int i = 0; i < size; ++i) {
		Part p1 = part_load(i, sp);
		
		float3 r = p0.pos - p1.pos;
		float e = eps*(p0.rad + p1.rad);
		float l = sqrt(dot(r,r) + e*e);
		acc += -1e-4f*p1.mass*r/(l*l*l);
	}
	
	p0.pos = p0.pos + dt*p0.vel;
	p0.vel = p0.vel + dt*acc;
	
	part_store(&p0, id, dp);
}
