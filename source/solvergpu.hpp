#pragma once

#include "solver.hpp"

#include <cmath>

#include <random>
#include <map>
#include <string>

#include "gl/program.hpp"
#include "gl/texture.hpp"
#include "gl/framebuffer.hpp"


class SolverGPU : public Solver {
public:
	static const int NSTAGES = 2;
	gl::FrameBuffer *dprops[NSTAGES];
	
	std::map<std::string, gl::Program*> &progs;
	
	std::minstd_rand rand_engine;
	std::uniform_real_distribution<> rand_dist;
	
	float random() {
		return float(rand_dist(rand_engine));
	}
	
public:
	SolverGPU(int size, std::map<std::string, gl::Program*> &progs)
	: Solver(size), progs(progs), rand_dist(0.0, 1.0) {
		int sarr[] = {size, 0};
		int zoarr[] = {0, 0};
		float *buf;
		
		int side = ceil(pow(double(size), 1.0/3.0));
		
		sprop = new gl::Texture();
		sarr[1] = 2;
		sprop->init(2, sarr, gl::Texture::RGBA8);
		buf = new float[4*sarr[0]*sarr[1]];
		for(int j = 0; j < sarr[1]; ++j) {
			for(int i = 0; i < sarr[0]; ++i) {
				float x = float(i%side)/(side - 1) - 0.5;
				float y = float((i/side)%side)/(side - 1) - 0.5;
				float z = float(i/(side*side))/(side - 1) - 0.5f;
				if(j == 0) {
					buf[4*(j*sarr[0] + i) + 0] = 1e-2; // radius
					buf[4*(j*sarr[0] + i) + 1] = 0.0f;
					buf[4*(j*sarr[0] + i) + 2] = 0.0f;
					buf[4*(j*sarr[0] + i) + 3] = 1.0f; // inverse mass
				} else if(j == 1) {
					// color
					buf[4*(j*sarr[0] + i) + 0] = x + 0.5;
					buf[4*(j*sarr[0] + i) + 1] = y + 0.5;
					buf[4*(j*sarr[0] + i) + 2] = z + 0.5;
					buf[4*(j*sarr[0] + i) + 3] = 1.0f;
				}
			}
		}
		sprop->write(buf, zoarr, sarr, gl::Texture::RGBA, gl::FLOAT);
		delete[] buf;
		
		sarr[1] = 2;
		for(int k = 0; k < NSTAGES; ++k) {
			gl::FrameBuffer *fb = new gl::FrameBuffer();
			fb->init(gl::Texture::RGBA32F, sarr[0], sarr[1]);
			if(k == 0) {
				float *buf = new float[4*sarr[0]*sarr[1]];
				for(int j = 0; j < sarr[1]; ++j) {
					for(int i = 0; i < sarr[0]; ++i) {
						float x = float(i%side)/(side - 1) - 0.5;
						float y = float((i/side)%side)/(side - 1) - 0.5;
						float z = float(i/(side*side))/(side - 1) - 0.5f;
						if(j == 0) {
							//position
							buf[4*(j*sarr[0] + i) + 0] = x;
							buf[4*(j*sarr[0] + i) + 1] = y;
							buf[4*(j*sarr[0] + i) + 2] = z;
							buf[4*(j*sarr[0] + i) + 3] = 1.0f;
						} else if(j == 1) {
							// velocity
							float tv = 0.1f, v = 0.2f;
							buf[4*(j*sarr[0] + i) + 0] = tv*(random() - 0.5) - v*y;
							buf[4*(j*sarr[0] + i) + 1] = tv*(random() - 0.5) + v*x;
							buf[4*(j*sarr[0] + i) + 2] = 0.0f;
							buf[4*(j*sarr[0] + i) + 3] = 1.0f;
						}
					}
				}
				fb->getTexture()->write(buf, zoarr, sarr, gl::Texture::RGBA, gl::FLOAT);
				delete[] buf;
			}
			dprops[k] = fb;
		}
		
		dprop = dprops[0]->getTexture();
	}
	
	virtual ~SolverGPU() {
		dprop = nullptr;
		for(int k = 0; k < NSTAGES; ++k) {
			delete dprops[k];
		}
		
		delete sprop;
	}
	
	virtual void load(Particle parts[]) {
		for(int i = 0; i < size; ++i) {
			
		}
	}
	
	virtual void solve(float dt, int steps = 1) override {
		gl::FrameBuffer *fb = nullptr;
		gl::Program *prog = nullptr;
		
		for(int i = 0; i < steps; ++i) {
			fb = dprops[1];
			fb->bind();
			prog = progs["solve"];
			prog->setUniform("u_sprop", sprop);
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			prog->setUniform("u_dt", dt/steps);
			prog->setUniform("u_count", size);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			dprops[1] = dprops[0];
			dprops[0] = fb;
		}
		
		dprop = dprops[0]->getTexture();
	}
};
