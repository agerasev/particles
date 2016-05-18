#pragma once

#include "solver.hpp"

#include <cmath>

#include <map>
#include <string>

#include "gl/program.hpp"
#include "gl/texture.hpp"
#include "gl/framebuffer.hpp"

#include <la/vec.hpp>

#include "glbank.hpp"

class SolverGPU : public Solver {
public:
	static const int stages = 2;
	gl::FrameBuffer *dprops[stages];
	const int sph = 2, dph = 2;
	
	GLBank *bank;
	
public:
	SolverGPU(int size, GLBank *bank)
	: Solver(size), bank(bank) {
		sprop = new gl::Texture();
		sprop->init(2, ivec2(size, sph).data(), gl::Texture::RGBA8);
		
		for(int k = 0; k < stages; ++k) {
			gl::FrameBuffer *fb = new gl::FrameBuffer();
			fb->init(gl::Texture::RGBA32F, size, dph);
			dprops[k] = fb;
		}
		
		dprop = dprops[0]->getTexture();
	}
	
	virtual ~SolverGPU() {
		dprop = nullptr;
		for(int k = 0; k < stages; ++k) {
			delete dprops[k];
		}
		
		delete sprop;
	}
	
	virtual void load(Particle parts[]) {
		int sarr[] = {size, 0};
		int zoarr[] = {0, 0};
		float *buf;
		
		sarr[1] = sph;
		buf = new float[4*sarr[0]*sarr[1]];
		for(int j = 0; j < sarr[1]; ++j) {
			for(int i = 0; i < sarr[0]; ++i) {
				Particle &p = parts[i];
				if(j == 0) {
					buf[4*(j*sarr[0] + i) + 0] = p.rad;
					buf[4*(j*sarr[0] + i) + 1] = 0.0f;
					buf[4*(j*sarr[0] + i) + 2] = 0.0f;
					buf[4*(j*sarr[0] + i) + 3] = p.mass;
				} else if(j == 1) {
					// color
					buf[4*(j*sarr[0] + i) + 0] = p.color.x();
					buf[4*(j*sarr[0] + i) + 1] = p.color.y();
					buf[4*(j*sarr[0] + i) + 2] = p.color.z();
					buf[4*(j*sarr[0] + i) + 3] = 1.0f;
				}
			}
		}
		sprop->write(buf, zoarr, sarr, gl::Texture::RGBA, gl::FLOAT);
		delete[] buf;
		
		sarr[1] = dph;
		for(int k = 0; k < stages; ++k) {
			gl::FrameBuffer *fb = dprops[k];
			if(k == 0) {
				float *buf = new float[4*sarr[0]*sarr[1]];
				for(int j = 0; j < sarr[1]; ++j) {
					for(int i = 0; i < sarr[0]; ++i) {
						Particle &p = parts[i];
						if(j == 0) {
							//position
							buf[4*(j*sarr[0] + i) + 0] = p.pos.x();
							buf[4*(j*sarr[0] + i) + 1] = p.pos.y();
							buf[4*(j*sarr[0] + i) + 2] = p.pos.z();
							buf[4*(j*sarr[0] + i) + 3] = 1.0f;
						} else if(j == 1) {
							// velocity
							buf[4*(j*sarr[0] + i) + 0] = p.vel.x();
							buf[4*(j*sarr[0] + i) + 1] = p.vel.y();
							buf[4*(j*sarr[0] + i) + 2] = p.vel.z();
							buf[4*(j*sarr[0] + i) + 3] = 1.0f;
						}
					}
				}
				fb->getTexture()->write(buf, zoarr, sarr, gl::Texture::RGBA, gl::FLOAT);
				delete[] buf;
			}
		}
		
		dprop = dprops[0]->getTexture();
	}
	
	virtual void solve(float dt, int steps = 1) override {
		gl::FrameBuffer *fb = nullptr;
		gl::Program *prog = nullptr;
		
		for(int i = 0; i < steps; ++i) {
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve"];
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
