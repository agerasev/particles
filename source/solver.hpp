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
		
		for(int j = 0; j < sph; ++j) {
			for(int i = 0; i < size; ++i) {
				T &p = parts[i];
				if(j == 0) {
					buf[4*(j*size + i) + 0] = p.rad;
					buf[4*(j*size + i) + 1] = 0.0f;
					buf[4*(j*size + i) + 2] = 0.0f;
					buf[4*(j*size + i) + 3] = p.mass;
				} else if(j == 1) {
					// color
					buf[4*(j*size + i) + 0] = p.color.x();
					buf[4*(j*size + i) + 1] = p.color.y();
					buf[4*(j*size + i) + 2] = p.color.z();
					buf[4*(j*size + i) + 3] = 1.0f;
				}
			}
		}
		sprop->write(
			buf, nullivec2.data(), ivec2(size, sph).data(), 
			gl::Texture::RGBA, gl::FLOAT
		);
		
		for(int j = 0; j < dph; ++j) {
			for(int i = 0; i < size; ++i) {
				T &p = parts[i];
				if(j == 0) {
					//position
					buf[4*(j*size + i) + 0] = p.pos.x();
					buf[4*(j*size + i) + 1] = p.pos.y();
					buf[4*(j*size + i) + 2] = p.pos.z();
					buf[4*(j*size + i) + 3] = 1.0f;
				} else if(j == 1) {
					// velocity
					buf[4*(j*size + i) + 0] = p.vel.x();
					buf[4*(j*size + i) + 1] = p.vel.y();
					buf[4*(j*size + i) + 2] = p.vel.z();
					buf[4*(j*size + i) + 3] = 1.0f;
				}
			}
		}
		dprop->write(
			buf, nullivec2.data(), ivec2(size, sph).data(), 
			gl::Texture::RGBA, gl::FLOAT
		);
	}

	virtual void load(Particle parts[]) = 0;
	virtual void solve(float dt, int steps = 1) = 0;
	void solve() {
		solve(dt, steps);
	}
};
