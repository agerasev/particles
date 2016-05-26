#pragma once

#include <cstdio>
#include <ctime>

#include <vector>

#include <omp.h>

#include "solver.hpp"
#include "particle.hpp"
#include "octree.hpp"

class SolverCPU : public Solver {
private:
	class PartBuf : public Particle {
	private:
		void _copy(const Particle &p) {
			color = p.color;
			mass = p.mass;
			rad = p.rad;
			
			pos = p.pos;
			vel = p.vel;
			cpos = p.pos;
			cvel = p.vel;
		}
	
	public:
		fvec3 cpos, cvel;
		static const int bufs = 4;
		fvec3 dpos[bufs], dvel[bufs];
		
		PartBuf() = default;
		PartBuf(const Particle &p) {
			_copy(p);
		}
		PartBuf &operator = (const Particle &p) {
			_copy(p);
			return *this;
		}
	};
	
	typedef Branch<const PartBuf*> PBranch;
	
	PartBuf *parts;
	
	float *buffer;
	
	// profiling
	double t_tree = 0.0, t_grav = 0.0;
	int n_tree = 0, n_grav = 0;
	
public:
	SolverCPU(int size) : Solver(size) {
		parts = new PartBuf[size];
		sprop = new gl::Texture();
		sprop->init(2, split_size(size*ps).data(), gl::Texture::RGBA32F);
		dprop = new gl::Texture();
		dprop->init(2, split_size(size*ps).data(), gl::Texture::RGBA32F);
		
		ivec2 texs = split_size(size*ps);
		buffer = new float[4*texs[0]*texs[1]];
	}
	virtual ~SolverCPU() {
		delete[] buffer;
		
		delete dprop;
		delete sprop;
		delete[] parts;
		
		printf("tree: %lf s\n", t_tree/n_tree);
		printf("grav: %lf s\n", t_grav/n_grav);
	}
	
	virtual void load(Particle parts[]) {
		for(int i = 0; i < size; ++i) {
			this->parts[i] = parts[i];
		}
	}
	
	fvec3 grav(PartBuf *p, const PartBuf *op) {
		fvec3 r = p->pos - op->pos;
		float e = eps*(p->rad + op->rad);
		float l = sqrt(dot(r,r) + e*e);
		return -1e-4*op->mass*r/(l*l*l);
	}
	
	fvec3 grav_avg(PartBuf *p, const fvec3 &pos, const float mass) {
		fvec3 r = p->pos - pos;
		float e = eps*p->rad;
		float l = sqrt(dot(r,r) + e*e);
		return -1e-4*mass*r/(l*l*l);
	}
	
	fvec3 elast(PartBuf *p0, const PartBuf *p1) {
		fvec3 p = p0->pos - p1->pos;
		fvec3 v = p0->vel - p1->vel;
		float r = p0->rad + p1->rad;
		float e = eps*r;
		float l = sqrt(dot(p,p) + e*e);
		float d = r - l;
		fvec3 f = (d > 0.0)*d*(p/l);
		f = f - 1e1*f*dot(f, v);
		return 1e2*f/p0->mass;
	}
	
	fvec3 attract(PartBuf *p, const PartBuf *op) {
		return (p->id != op->id)*(grav(p, op));// + elast(p, op));
	}
	
	fvec3 attract(PartBuf *p, const Branch<const PartBuf*> *b) {
		float l = length(p->pos - b->barycenter);
		if(b->size/l < gth) {
			return grav_avg(p, b->barycenter, b->mass);
		} else {
			fvec3 acc = nullfvec3;
			if(b->leaf) {
				for(int i = 0; i < int(b->buffer.size()); ++i) {
					acc += attract(p, b->buffer[i]);
				}
			} else {
				for(int i = 0; i < 8; ++i) {
					acc += attract(p, b->next[i]);
				}
			}
			return acc;
		}
	}
	
	PBranch *buildTree() {
		PBranch *trunk = new PBranch(nullfvec3, tree_size, tree_depth);
		double begin = omp_get_wtime();
		for(int i = 0; i < size; ++i) {
			trunk->add(&parts[i]);
		}
		trunk->update();
		t_tree += double(omp_get_wtime() - begin);
		return trunk;
	}
	
	virtual void solve(float dt) override {
		double begin;
		PBranch *trunk;
		
		#pragma omp parallel for
		for(int i = 0; i < size; ++i) {
			parts[i].cpos = parts[i].pos;
			parts[i].cvel = parts[i].vel;
		}
		
		// stage 1
		trunk = buildTree();
		begin = omp_get_wtime();
		#pragma omp parallel for
		for(int i = 0; i < size; ++i) {
			parts[i].dpos[0] = parts[i].vel;
			parts[i].dvel[0] = attract(&parts[i], trunk);
			parts[i].pos = parts[i].cpos + 0.5*dt*parts[i].dpos[0];
			parts[i].vel = parts[i].cvel + 0.5*dt*parts[i].dvel[0];
		}
		t_grav += double(omp_get_wtime() - begin);
		delete trunk;
		
		// stage 2
		trunk = buildTree();
		begin = omp_get_wtime();
		#pragma omp parallel for
		for(int i = 0; i < size; ++i) {
			parts[i].dpos[1] = parts[i].vel;
			parts[i].dvel[1] = attract(&parts[i], trunk);
			parts[i].pos = parts[i].cpos + 0.5*dt*parts[i].dpos[1];
			parts[i].vel = parts[i].cvel + 0.5*dt*parts[i].dvel[1];
		}
		t_grav += double(omp_get_wtime() - begin);
		delete trunk;
		
		// stage 3
		trunk = buildTree();
		begin = omp_get_wtime();
		#pragma omp parallel for
		for(int i = 0; i < size; ++i) {
			parts[i].dpos[2] = parts[i].vel;
			parts[i].dvel[2] = attract(&parts[i], trunk);
			parts[i].pos = parts[i].cpos + dt*parts[i].dpos[2];
			parts[i].vel = parts[i].cvel + dt*parts[i].dvel[2];
		}
		t_grav += double(omp_get_wtime() - begin);
		delete trunk;
		
		// stage 4
		trunk = buildTree();
		begin = omp_get_wtime();
		#pragma omp parallel for
		for(int i = 0; i < size; ++i) {
			parts[i].dpos[3] = parts[i].vel;
			parts[i].dvel[3] = attract(&parts[i], trunk);
			parts[i].pos = parts[i].cpos + dt/6.0*(parts[i].dpos[0] + 2.0*parts[i].dpos[1] + 2.0*parts[i].dpos[2] + parts[i].dpos[3]);
			parts[i].vel = parts[i].cvel + dt/6.0*(parts[i].dvel[0] + 2.0*parts[i].dvel[1] + 2.0*parts[i].dvel[2] + parts[i].dvel[3]);
		}
		t_grav += double(omp_get_wtime() - begin);
		delete trunk;
		
		n_tree += 1;
		n_grav += 1;
		
		// load to gl textures
		
		float *buf = buffer;
		
		for(int i = 0; i < size; ++i) {
			Particle &p = parts[i];
	
			buf[4*(i*ps + 0) + 0] = p.rad;
			buf[4*(i*ps + 0) + 1] = 0.0f;
			buf[4*(i*ps + 0) + 2] = 0.0f;
			buf[4*(i*ps + 0) + 3] = p.mass;
		
			// color
			buf[4*(i*ps + 1) + 0] = p.color.x();
			buf[4*(i*ps + 1) + 1] = p.color.y();
			buf[4*(i*ps + 1) + 2] = p.color.z();
			buf[4*(i*ps + 1) + 3] = 1.0f;
		}
		
		sprop->write(
			buf, nullivec2.data(), split_size(size*ps).data(), 
			gl::Texture::RGBA, gl::FLOAT
		);
		
		for(int i = 0; i < size; ++i) {
			Particle &p = parts[i];

			//position
			buf[4*(i*ps + 0) + 0] = p.pos.x();
			buf[4*(i*ps + 0) + 1] = p.pos.y();
			buf[4*(i*ps + 0) + 2] = p.pos.z();
			buf[4*(i*ps + 0) + 3] = 1.0f;

			// velocity
			buf[4*(i*ps + 1) + 0] = p.vel.x();
			buf[4*(i*ps + 1) + 1] = p.vel.y();
			buf[4*(i*ps + 1) + 2] = p.vel.z();
			buf[4*(i*ps + 1) + 3] = 1.0f;
		}
		
		dprop->write(
			buf, nullivec2.data(), split_size(size*ps).data(), 
			gl::Texture::RGBA, gl::FLOAT
		);
	}
};
