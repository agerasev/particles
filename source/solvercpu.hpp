#pragma once

#include <cstdio>

#include <vector>

#include "solver.hpp"
#include "particle.hpp"

class PartBuf : public Particle {
private:
	void _copy(const Particle &p) {
		color = p.color;
		mass = p.mass;
		rad = p.rad;
		pos = p.pos;
		vel = p.vel;
	}

public:
	static const int bufs = 1;
	fvec3 bpos[bufs], bvel[bufs];
	PartBuf() = default;
	PartBuf(const Particle &p) {
		_copy(p);
	}
	PartBuf &operator = (const Particle &p) {
		_copy(p);
		return *this;
	}
};

class Branch {
public:
	fvec3 center;
	float size;
	
	static const int max = 8;
	bool leaf = true;
	int depth;
	
	std::vector<const PartBuf*> buffer;
	
	Branch *next[8] = {nullptr};
	
	fvec3 barycenter;
	float mass;
	int count = 0;
	
	Branch(fvec3 center, float size, int depth) 
	: center(center), size(size), depth(depth) {
		buffer.reserve(max);
	}
	~Branch() {
		if(!leaf) {
			for(int i = 0; i < 8; ++i) {
				delete next[i];
			}
		}
	}
	void init_next() {
		leaf = false;
		for(int i = 0; i < 8; ++i) {
			fvec3 dir;
			for(int j = 0; j < 3; ++j) {
				dir[j] = ((i >> j) & 1) - 0.5;
			}
			next[i] = new Branch(
				center + dir*size,
				0.5*size, depth - 1
			);
		}
	}
	int get_next(fvec3 pos) const {
		fvec3 dir = pos - center;
		int i = 0;
		for(int j = 0; j < 3; ++j) {
			i += (1 << j)*(dir[j] >= 0);
		}
		return i;
	}
	void add_next(const PartBuf *p) {
		next[get_next(p->pos)]->add(p);
	}
	void add_current(const PartBuf *p) {
		if(int(buffer.size()) >= max && depth > 0) {
			init_next();
			add_next(p);
			for(int i = 0; i < int(buffer.size()); ++i) {
				add_next(buffer[i]);
			}
			buffer.clear();
		} else {
			buffer.push_back(p);
		}
	}
	void add(const PartBuf *p) {
		if(leaf) {
			add_current(p);
		} else {
			add_next(p);
		}
	}
	void update() {
		barycenter = nullfvec3;
		mass = 0.0f;
		count = 0;
		if(!leaf) {
			for(int i = 0; i < 8; ++i) {
				next[i]->update();
				if(next[i]->count > 0) {
					count += next[i]->count;
					barycenter += next[i]->barycenter*next[i]->mass;
					mass += next[i]->mass;
				}
			}
			barycenter /= mass;
		} else {
			count = buffer.size();
			if(count > 0) {
				for(int i = 0; i < int(buffer.size()); ++i) {
					barycenter += buffer[i]->pos*buffer[i]->mass;
					mass += buffer[i]->mass;
				}
				barycenter /= mass;
			}
		}
	}
};

class SolverCPU : public Solver {
private:
	PartBuf *parts;
	const float gth = 0.5;
	
public:
	SolverCPU(int size) : Solver(size) {
		parts = new PartBuf[size];
		sprop = new gl::Texture();
		sprop->init(2, ivec2(size*sph, 1).data(), gl::Texture::RGBA32F);
		dprop = new gl::Texture();
		dprop->init(2, ivec2(size*dph, 1).data(), gl::Texture::RGBA32F);
	}
	virtual ~SolverCPU() {
		delete dprop;
		delete sprop;
		delete[] parts;
	}
	
	virtual void load(Particle parts[]) {
		for(int i = 0; i < size; ++i) {
			this->parts[i] = parts[i];
		}
	}
	
	static fvec3 accel(fvec3 apos, fvec3 bpos, float bmass) {
		fvec3 r = apos - bpos;
		float eps = 2e-2;
		float l = sqrt(dot(r,r) + eps*eps);
		return -1e-4*bmass*r/(l*l*l);
	}
	
	fvec3 attract(PartBuf *p, const PartBuf *op) {
		if(p == op)
			return nullfvec3;
		return accel(p->pos, op->pos, op->mass);
	}
	
	fvec3 attract(PartBuf *p, const Branch *b) {
		float l = length(p->pos - b->barycenter);
		if(b->size/l < gth) {
			return accel(p->pos, b->barycenter, b->mass);
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
	
	virtual void solve(float dt, int steps = 1) override {
		Branch trunk(nullfvec3, 1.0, 16);
		for(int i = 0; i < size; ++i) {
			trunk.add(&parts[i]);
		}
		trunk.update();
		
		for(int i = 0; i < size; ++i) {
			parts[i].bpos[0] = parts[i].pos + dt*parts[i].vel;
			parts[i].bvel[0] = parts[i].vel + dt*attract(&parts[i], &trunk);
		}
		for(int i = 0; i < size; ++i) {
			parts[i].pos = parts[i].bpos[0];
			parts[i].vel = parts[i].bvel[0];
		}
		loadTex(parts);
	}
};
