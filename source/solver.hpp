#pragma once

#include "gl/texture.hpp"

#include "particle.hpp"

class Solver {
public:
	const int size;
	gl::Texture *sprop = nullptr;
	gl::Texture *dprop = nullptr;
	const int sph = 2, dph = 2;
	float *buffer;
	
	float dt = 1e-2;
	int steps = 1;
	
	Solver(int size) : size(size) {
		buffer = new float[4*size*(sph > dph ? sph : dph)];
	}
	virtual ~Solver() {
		delete[] buffer;
	}
	
	template <typename T>
	void loadTex(T parts[]) {
		float *buf = buffer;
		
		for(int i = 0; i < size; ++i) {
			T &p = parts[i];
	
			buf[4*(i*sph + 0) + 0] = p.rad;
			buf[4*(i*sph + 0) + 1] = 0.0f;
			buf[4*(i*sph + 0) + 2] = 0.0f;
			buf[4*(i*sph + 0) + 3] = p.mass;
		
			// color
			buf[4*(i*sph + 1) + 0] = p.color.x();
			buf[4*(i*sph + 1) + 1] = p.color.y();
			buf[4*(i*sph + 1) + 2] = p.color.z();
			buf[4*(i*sph + 1) + 3] = 1.0f;
		}
		sprop->write(
			buf, nullivec2.data(), ivec2(size*sph, 1).data(), 
			gl::Texture::RGBA, gl::FLOAT
		);
		
		for(int i = 0; i < size; ++i) {
			T &p = parts[i];

			//position
			buf[4*(i*dph + 0) + 0] = p.pos.x();
			buf[4*(i*dph + 0) + 1] = p.pos.y();
			buf[4*(i*dph + 0) + 2] = p.pos.z();
			buf[4*(i*dph + 0) + 3] = 1.0f;

			// velocity
			buf[4*(i*dph + 1) + 0] = p.vel.x();
			buf[4*(i*dph + 1) + 1] = p.vel.y();
			buf[4*(i*dph + 1) + 2] = p.vel.z();
			buf[4*(i*dph + 1) + 3] = 1.0f;
		}
		dprop->write(
			buf, nullivec2.data(), ivec2(size*dph, 1).data(), 
			gl::Texture::RGBA, gl::FLOAT
		);
	}

	virtual void load(Particle parts[]) = 0;
	virtual void solve(float dt, int steps = 1) = 0;
	void solve() {
		solve(dt, steps);
	}
};
