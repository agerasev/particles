#pragma once

#include "solver.hpp"

#include <cl/session.hpp>
#include <cl/queue.hpp>
#include <cl/context.hpp>
#include <cl/program.hpp>
#include <cl/map.hpp>
#include <cl/buffer_object.hpp>
#include <cl/gl_image_object.hpp>

#include "opencl.hpp"
#include <export/particle.h>
#include <export/deriv.h>

class SolverGPU : public Solver {
protected:
	cl::session *session;
	cl::queue *queue;
	
	cl::map<cl::buffer_object*> buffers;
	cl::gl_image_object *image = nullptr;
	
	cl::program *program;
	cl::map<cl::kernel*> kernels;
	
	ivec2 texs;
	bool interop = false;
	
	std::vector<float> gl_buffer;
	
	std::vector<_Particle> parts;
	std::vector<float> parts_buffer;
	
public:
	SolverGPU(const int size) : Solver(size) {
		texs = split_size(size*ps);
		
		parts.resize(size);
		parts_buffer.resize(PART_FSIZE*size);
		gl_buffer.resize(4*ps*size);
		
		sprop = new gl::Texture();
		dprop = new gl::Texture();
		sprop->init(2, texs.data(), gl::Texture::RGBA32F);
		
		session = new cl::session(1);
		queue = &session->get_queue();
		
		cl_context context = session->get_context().id();
		
		buffers.insert("part0", new cl::buffer_object(context, PART_SIZE*size));
		buffers.insert("part1", new cl::buffer_object(context, PART_SIZE*size));
		buffers.insert("deriv0", new cl::buffer_object(context, DERIV_SIZE*size));
		buffers.insert("deriv1", new cl::buffer_object(context, DERIV_SIZE*size));
		buffers.insert("deriv2", new cl::buffer_object(context, DERIV_SIZE*size));
		buffers.insert("deriv3", new cl::buffer_object(context, DERIV_SIZE*size));
		for(cl::buffer_object *b : buffers) {
			b->bind_queue(queue->get_cl_command_queue());
		}
		
		if(interop) {
			image = new cl::gl_image_object(context, texs.x(), texs.y());
			image->bind_queue(queue->get_cl_command_queue());
			dprop->wrap(image->get_texture(), 2, texs.data(), gl::Texture::RGBA32F);
		} else {
			dprop->init(2, texs.data(), gl::Texture::RGBA32F);
		}
		
		
		program = new cl::program(context, session->get_device().id(), "kernels.c", "kernels");
		kernels = program->get_kernel_map();
		for(cl::kernel *k : kernels) {
			k->bind_queue(*queue);
		}
	}
	
	virtual ~SolverGPU() {
		session->get_queue().flush();
		
		delete program;
		
		if(image != nullptr) {
			delete image;
		}
		
		for(cl::buffer_object *b : buffers) {
			delete b;
		}
		
		delete session;
		
		delete sprop;
		delete dprop;
	}
	
	void store_gls(_Particle parts[]) {
		float *buf = gl_buffer.data();
		
		for(int i = 0; i < size; ++i) {
			_Particle &p = parts[i];
	
			buf[4*(i*ps + 0) + 0] = p.rad;
			buf[4*(i*ps + 0) + 1] = 0.0f;
			buf[4*(i*ps + 0) + 2] = 0.0f;
			buf[4*(i*ps + 0) + 3] = p.mass;
			
			buf[4*(i*ps + 1) + 0] = p.color.x();
			buf[4*(i*ps + 1) + 1] = p.color.y();
			buf[4*(i*ps + 1) + 2] = p.color.z();
			buf[4*(i*ps + 1) + 3] = 1.0f;
		}
		
		sprop->write(
			buf, nullivec2.data(), split_size(size*ps).data(), 
			gl::Texture::RGBA, gl::FLOAT
		);
	}
	
	void store_gld(_Particle parts[]) {
		float *buf = gl_buffer.data();
		
		for(int i = 0; i < size; ++i) {
			_Particle p = parts[i];
	
			buf[4*(i*ps + 0) + 0] = p.pos.x();
			buf[4*(i*ps + 0) + 1] = p.pos.y();
			buf[4*(i*ps + 0) + 2] = p.pos.z();
			buf[4*(i*ps + 0) + 3] = 1.0f;
			
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
	
	void store_cl_parts(cl::buffer_object *clbuf, _Particle parts[]) {
		float *buf = parts_buffer.data();
		
		for(int i = 0; i < size; ++i) {
			Particle pg;
			const _Particle &pc = parts[i];
			pc.store(&pg);
			part_store(&pg, i, buf);
		}
		
		clbuf->store_data(buf);
	}
	
	void load_cl_parts(cl::buffer_object *clbuf, _Particle parts[]) {
		float *buf = parts_buffer.data();
		
		clbuf->load_data(parts_buffer.data());
		
		for(int i = 0; i < size; ++i) {
			const Particle pg = part_load(i, buf);
			_Particle &pc = parts[i];
			pc.load(&pg);
		}
	}
	
	virtual void store(_Particle parts[]) override {
		for(int i = 0; i < size; ++i) {
			this->parts[i] = parts[i];
		}
		store_cl_parts(buffers["part0"], parts);
		store_gls(parts);
		store_gld(parts);
	}
	
	void transfer_cl_to_gl() {
		if(interop) {
			kernels["write_gl_tex"]->evaluate(
				cl::work_range(size), buffers["part0"],
				image, size, maxts
			);
		} else {
			load_cl_parts(buffers["part0"], parts.data());
			store_gld(parts.data());
		}
	}
	
	virtual void solve(float dt) override {
		
		// solve
		
		if(rk4) {
			// stage 1
			kernels["solve_plain_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part0"], 
				buffers["deriv0"], size, eps
			);
			kernels["solve_rk4_v_1_2"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv0"], size, dt
			);
			
			// stage 2
			kernels["solve_plain_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part1"], 
				buffers["deriv1"], size, eps
			);
			kernels["solve_rk4_v_1_2"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv1"], size, dt
			);
			
			// stage 1
			kernels["solve_plain_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part1"], 
				buffers["deriv2"], size, eps
			);
			kernels["solve_rk4_v_3"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv2"], size, dt
			);
			
			// stage 1
			kernels["solve_plain_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part1"], 
				buffers["deriv3"], size, eps
			);
			kernels["solve_rk4_v_4"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv0"], buffers["deriv1"], buffers["deriv2"], buffers["deriv3"], 
				size, dt
			);
		} else {
			kernels["solve_plain_euler"]->evaluate(
				cl::work_range(size), buffers["part0"], 
				buffers["part1"], size, eps, dt
			);
		}
		
		cl::buffer_object *tmp = buffers["part0"];
		buffers["part0"] = buffers["part1"];
		buffers["part1"] = tmp;
		
		
		// write result to gl texture
		transfer_cl_to_gl();
		
		queue->flush();
	}
};
