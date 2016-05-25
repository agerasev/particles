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

class SolverGPUGL : public Solver {
public:
	gl::FrameBuffer *dprops[2];
	gl::FrameBuffer *derivs[4];
	
	GLBank *bank;
	
public:
	SolverGPUGL(int size, GLBank *bank)
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
	
	virtual ~SolverGPUGL() {
		dprop = nullptr;
		for(int k = 0; k < sizeofarr(dprops); ++k) {
			delete dprops[k];
		}
		
		for(int k = 0; k < sizeofarr(derivs); ++k) {
			delete derivs[k];
		}
		
		delete sprop;
	}
	
	virtual void load(Particle parts[]) override {
		loadTex(parts);
	}
	
	void setUniforms(gl::Program *prog, float dt, int steps) {
		prog->setUniform("u_sprop", sprop);
		prog->setUniform("u_dt", dt/steps);
		prog->setUniform("u_count", size);
		prog->setUniform("MAXTS", maxts);
		prog->setUniform("u_eps", eps);
	}
	
	virtual void solve(float dt) override {
		gl::FrameBuffer *fb = nullptr;
		gl::Program *prog = nullptr;
		
		
		if(true) {
			
			// Euler
			
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve-plain-euler"];
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
		} else {
			
			// RK4
			
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
			prog = bank->progs["solve-rk4-v-1-2"];
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
			prog = bank->progs["solve-rk4-v-1-2"];
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
			prog = bank->progs["solve-rk4-v-3"];
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
			prog = bank->progs["solve-rk4-v-4"];
			prog->setUniform("u_deriv_1", derivs[0]->getTexture());
			prog->setUniform("u_deriv_2", derivs[1]->getTexture());
			prog->setUniform("u_deriv_3", derivs[2]->getTexture());
			prog->setUniform("u_deriv_4", derivs[3]->getTexture());
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
		
		}
		
		fb = dprops[1];
		dprops[1] = dprops[0];
		dprops[0] = fb;
		
		dprop = dprops[0]->getTexture();
	}
};


#include <cl/session.hpp>
#include <cl/queue.hpp>
#include <cl/context.hpp>
#include <cl/buffer_object.hpp>
#include <cl/program.hpp>
#include <cl/map.hpp>

#include "opencl.hpp"
#include <export/particle.h>

class SolverGPUCL : public Solver {
private:
	cl::session *session;
	cl::queue *queue;
	
	cl::map<cl::buffer_object*> buffers; 
	
	cl::program *program;
	cl::map<cl::kernel*> kernels;
	
	float *clbuffer = nullptr;
	
public:
	SolverGPUCL(const int size) : Solver(size) {
		sprop = new gl::Texture();
		sprop->init(2, split_size(size*ps).data(), gl::Texture::RGBA32F);
		dprop = new gl::Texture();
		dprop->init(2, split_size(size*ps).data(), gl::Texture::RGBA32F);
		
		clbuffer = new float[PART_SIZE*size];
		
		session = new cl::session();
		queue = &session->get_queue();
		
		cl_context context = session->get_context().get_cl_context();
		
		buffers.insert("part0", new cl::buffer_object(context, PART_SIZE*size));
		buffers.insert("part1", new cl::buffer_object(context, PART_SIZE*size));
		for(cl::buffer_object *b : buffers) {
			b->bind_queue(queue->get_cl_command_queue());
		}
		
		program = new cl::program(context, session->get_device_id(), "kernels.c", "kernels");
		kernels = program->get_kernel_map();
		for(cl::kernel *k : kernels) {
			k->bind_queue(*queue);
		}
	}
	
	virtual ~SolverGPUCL() {
		session->get_queue().flush();
		
		delete program;
		
		for(cl::buffer_object *b : buffers) {
			delete b;
		}
		
		delete session;
		
		delete[] clbuffer;
	}
	
	virtual void load(Particle parts[]) override {
		float *buf = clbuffer;
		for(int i = 0; i < size; ++i) {
			Part pg;
			const Particle &pc = parts[i];
			pg.mass = pc.mass;
			pg.rad = pc.rad;
			for(int j = 0; j < 3; ++j) {
				pg.pos.data[j] = pc.pos.data()[j];
				pg.vel.data[j] = pc.vel.data()[j];
			}
			part_store(&pg, i, buf);
		}
		buffers["part0"]->store_data(buf);
		
		loadTex(parts);
	}
	
	virtual void solve(float dt) override {
		kernels["solve_plain_euler"]->evaluate(
			cl::work_range(size), buffers["part0"], 
			buffers["part1"], size, eps, dt
		);
		
		cl::buffer_object *tmp = buffers["part0"];
		buffers["part0"] = buffers["part1"];
		buffers["part1"] = tmp;
		
		buffers["part0"]->load_data(clbuffer);
		clLoadTex(clbuffer);
		
		queue->flush();
	}
};
