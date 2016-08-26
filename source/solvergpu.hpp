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
	
	cl::program *program;
	cl::map<cl::kernel*> kernels;
	
	std::vector<ParticleCPU> parts;
	std::vector<float> parts_buffer;
	
public:
	SolverGPU(const int size, int features) : Solver(size, features) {
		parts.resize(size);
		parts_buffer.resize(PART_FSIZE*size);
		
		int cl_features = 0;
		session = new cl::session(1, cl_features);
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
		
		program = new cl::program(context, session->get_device().id(), "kernels.c", "kernels");
		kernels = program->get_kernel_map();
		for(cl::kernel *k : kernels) {
			k->bind_queue(*queue);
		}
	}
	
	virtual ~SolverGPU() {
		session->get_queue().flush();
		
		delete program;
		
		for(cl::buffer_object *b : buffers) {
			delete b;
		}
		
		delete session;
	}
	
	void store_cl_parts(cl::buffer_object *clbuf, const ParticleCPU parts[]) {
		float *buf = parts_buffer.data();
		
		for(int i = 0; i < size; ++i) {
			Particle pg;
			const ParticleCPU &pc = parts[i];
			pc.store(&pg);
			part_store(&pg, i, buf);
		}
		
		clbuf->store_data(buf);
		
		queue->flush();
	}
	
	void load_cl_parts(cl::buffer_object *clbuf, ParticleCPU parts[]) {
		float *buf = parts_buffer.data();
		
		clbuf->load_data(parts_buffer.data());
		
		for(int i = 0; i < size; ++i) {
			const Particle pg = part_load(i, buf);
			ParticleCPU &pc = parts[i];
			pc.load(&pg);
		}
		
		queue->flush();
	}
	
	virtual void store(const ParticleCPU parts[]) override {
		for(int i = 0; i < size; ++i) {
			this->parts[i] = parts[i];
		}
		store_cl_parts(buffers["part0"], parts);
	}
	
	virtual void load(ParticleCPU parts[]) {
		for(int i = 0; i < size; ++i) {
			parts[i] = this->parts[i];
		}
		load_cl_parts(buffers["part0"], parts);
	}
	
	virtual void solve(float dt) override {
		if(features & RK4) {
			// stage 1
			kernels["solve_plain_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part0"], 
				buffers["deriv0"], size
			);
			kernels["solve_rk4_v_1_2"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv0"], size, dt
			);
			
			// stage 2
			kernels["solve_plain_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part1"], 
				buffers["deriv1"], size
			);
			kernels["solve_rk4_v_1_2"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv1"], size, dt
			);
			
			// stage 3
			kernels["solve_plain_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part1"], 
				buffers["deriv2"], size
			);
			kernels["solve_rk4_v_3"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv2"], size, dt
			);
			
			// stage 4
			kernels["solve_plain_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part1"], 
				buffers["deriv3"], size
			);
			kernels["solve_rk4_v_4"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv0"], buffers["deriv1"], buffers["deriv2"], buffers["deriv3"], 
				size, dt
			);
		} else {
			kernels["solve_plain_euler"]->evaluate(
				cl::work_range(size), buffers["part0"], 
				buffers["part1"], size, dt
			);
		}
		
		cl::buffer_object *tmp = buffers["part0"];
		buffers["part0"] = buffers["part1"];
		buffers["part1"] = tmp;
		
		queue->flush();
	}
};
