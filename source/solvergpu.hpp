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

#define sizeofarr(array) \
	int(sizeof(array)/sizeof(array[0]))

class SolverGPU : public Solver {
public:
	gl::FrameBuffer *dprops[2];
	gl::FrameBuffer *derivs[4];
	
	GLBank *bank;
	
public:
	SolverGPU(int size, GLBank *bank)
	: Solver(size), bank(bank) {
		sprop = new gl::Texture();
		sprop->init(2, split_size(size*ps).data(), gl::Texture::RGBA32F);
		
		for(int k = 0; k < sizeofarr(derivs); ++k) {
			gl::FrameBuffer *fb = new gl::FrameBuffer();
			fb->init(2, gl::Texture::RGBA32F, split_size(size*ps).data());
			derivs[k] = fb;
		}
		
		for(int k = 0; k < sizeofarr(dprops); ++k) {
			gl::FrameBuffer *fb = new gl::FrameBuffer();
			fb->init(2, gl::Texture::RGBA32F, split_size(size*ps).data());
			dprops[k] = fb;
		}
		
		dprop = dprops[0]->getTexture();
	}
	
	virtual ~SolverGPU() {
		dprop = nullptr;
		for(int k = 0; k < sizeofarr(dprops); ++k) {
			delete dprops[k];
		}
		
		for(int k = 0; k < sizeofarr(derivs); ++k) {
			delete derivs[k];
		}
		
		delete sprop;
	}
	
	virtual void load(Particle parts[]) {
		loadTex(parts);
	}
	
	void setUniforms(gl::Program *prog, float dt, int steps) {
		prog->setUniform("u_sprop", sprop);
		prog->setUniform("u_dt", dt/steps);
		prog->setUniform("u_count", size);
		prog->setUniform("MAXTS", maxts);
		prog->setUniform("u_eps", eps);
	}
	
	virtual void solve(float dt, int steps = 1) override {
		gl::FrameBuffer *fb = nullptr;
		gl::Program *prog = nullptr;
		
		for(int i = 0; i < steps; ++i) {
			
			// stage 1
			
			fb = derivs[0];
			fb->bind();
			prog = bank->progs["solve-plain-rk4-d"];
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve-plain-rk4-v-1-2"];
			prog->setUniform("u_deriv_1_2", derivs[0]->getTexture());
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			// stage 2
			
			fb = derivs[1];
			fb->bind();
			prog = bank->progs["solve-plain-rk4-d"];
			prog->setUniform("u_dprop", dprops[1]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve-plain-rk4-v-1-2"];
			prog->setUniform("u_deriv_1_2", derivs[1]->getTexture());
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			// stage 3
			
			fb = derivs[2];
			fb->bind();
			prog = bank->progs["solve-plain-rk4-d"];
			prog->setUniform("u_dprop", dprops[1]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve-plain-rk4-v-3"];
			prog->setUniform("u_deriv_3", derivs[2]->getTexture());
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			// stage 4 
			
			fb = derivs[3];
			fb->bind();
			prog = bank->progs["solve-plain-rk4-d"];
			prog->setUniform("u_dprop", dprops[1]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve-plain-rk4-v-4"];
			prog->setUniform("u_deriv_1", derivs[0]->getTexture());
			prog->setUniform("u_deriv_2", derivs[1]->getTexture());
			prog->setUniform("u_deriv_3", derivs[2]->getTexture());
			prog->setUniform("u_deriv_4", derivs[3]->getTexture());
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			fb = dprops[1];
			dprops[1] = dprops[0];
			dprops[0] = fb;
		}
		
		dprop = dprops[0]->getTexture();
	}
};
