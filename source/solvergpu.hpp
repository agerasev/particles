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
	
	GLBank *bank;
	
public:
	SolverGPU(int size, GLBank *bank)
	: Solver(size), bank(bank) {
		sprop = new gl::Texture();
		sprop->init(2, split_size(size*ps).data(), gl::Texture::RGBA32F);
		
		for(int k = 0; k < stages; ++k) {
			gl::FrameBuffer *fb = new gl::FrameBuffer();
			ivec2 ss = split_size(size*ps);
			fb->init(gl::Texture::RGBA32F, ss.x(), ss.y());
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
		loadTex(parts);
	}
	
	virtual void solve(float dt, int steps = 1) override {
		gl::FrameBuffer *fb = nullptr;
		gl::Program *prog = nullptr;
		
		for(int i = 0; i < steps; ++i) {
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve-plain"];
			prog->setUniform("u_sprop", sprop);
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			prog->setUniform("u_dt", dt/steps);
			prog->setUniform("u_count", size);
			prog->setUniform("MAXTS", maxts);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			dprops[1] = dprops[0];
			dprops[0] = fb;
		}
		
		dprop = dprops[0]->getTexture();
	}
};
