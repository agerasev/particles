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
	}
	virtual ~SolverCPU() {
		delete dprop;
		delete sprop;
		delete[] parts;
		
		//printf("tree: %lf s\n", t_tree/n_tree);
		//printf("grav: %lf s\n", t_grav/n_grav);
	}
	
	virtual void load(Particle parts[]) {
		for(int i = 0; i < size; ++i) {
			this->parts[i] = parts[i];
		}
	}
	
	fvec3 grav(fvec3 apos, fvec3 bpos, float bmass) {
		fvec3 r = apos - bpos;
		float l = sqrt(dot(r,r) + eps*eps);
		return -1e-4*bmass*r/(l*l*l);
	}
	
	fvec3 attract(PartBuf *p, const PartBuf *op) {
		return grav(p->pos, op->pos, op->mass);
	}
	
	fvec3 attract(PartBuf *p, const Branch<const PartBuf*> *b) {
		float l = length(p->pos - b->barycenter);
		if(b->size/l < gth) {
			return grav(p->pos, b->barycenter, b->mass);
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
	
	virtual void solve(float dt, int steps = 1) override {
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
		
		loadTex(parts);
		
		n_tree += 1;
		n_grav += 1;
	}
};
