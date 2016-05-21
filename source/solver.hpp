#pragma once

#include "gl/texture.hpp"

#include "particle.hpp"

class Solver {
public:
	const int size;
	gl::Texture *sprop = nullptr;
	gl::Texture *dprop = nullptr;
	const int ps = 2;
	float *buffer;
	
	float dt = 1e-2;
	int steps = 1;
	
	float gth = 0.1;
	float eps = 1e1;
	
	float tree_size = 16.0;
	int tree_depth = 16;
	
	int maxts = 0;
	
	ivec2 split_size(int s) {
		if(s <= maxts) {
			return ivec2(s, 1);
		} else {
			return ivec2(maxts, (s - 1)/maxts + 1);
		}
	}
	ivec2 split_id(int id) {
		return ivec2(id%maxts, id/maxts);
	}
	
	Solver(int size) : size(size) {
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxts);
		printf("GL_MAX_TEXTURE_SIZE: %d\n", maxts);
		ivec2 texs = split_size(size*ps);
		buffer = new float[4*texs[0]*texs[1]];
	}
	virtual ~Solver() {
		delete[] buffer;
	}
	
	template <typename T>
	void loadTex(T parts[]) {
		float *buf = buffer;
		
		for(int i = 0; i < size; ++i) {
			T &p = parts[i];
	
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
			T &p = parts[i];

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

	virtual void load(Particle parts[]) = 0;
	virtual void solve(float dt, int steps = 1) = 0;
	void solve() {
		solve(dt, steps);
	}
};
