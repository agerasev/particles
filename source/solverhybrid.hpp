#pragma once

#include <cstdio>
#include <cmath>

#include <vector>

#include "solvergpu.hpp"
#include "particle.hpp"
#include "octree.hpp"

#include "opencl.h"
#include <export/tree.h>
#include <export/tree_depth.h>

class SolverHybrid : public SolverGPU {
private:
	int tree_depth = MAX_TREE_DEPTH;
	
	int *tree_buffer = nullptr;
	int *tree_link_buffer = nullptr;
	float *tree_data_buffer = nullptr;
	
	typedef _Branch<const _Particle*> PBranch;
	
public:
	SolverHybrid(int size, int features) : SolverGPU(size, features) {
		cl_context context = session->get_context().id();
		
		int nbranch = 10*size;
		int bs;
		
		bs = nbranch*BRANCH_ISIZE;
		buffers.insert("tree", new cl::buffer_object(context, bs*sizeof(int)));
		tree_buffer = new int[bs];
		
		bs = size + nbranch*8;
		buffers.insert("tree_link", new cl::buffer_object(context, bs*sizeof(int)));
		tree_link_buffer = new int[bs];
		
		bs = size*PART_FSIZE + nbranch*BRANCH_DATA_FSIZE;
		buffers.insert("tree_data", new cl::buffer_object(context, bs*sizeof(float)));
		tree_data_buffer = new float[bs];
		
		for(cl::buffer_object *b : buffers) {
			b->bind_queue(queue->get_cl_command_queue());
		}
	}
	virtual ~SolverHybrid() {
		delete[] tree_buffer;
		delete[] tree_link_buffer;
		delete[] tree_data_buffer;
	}
	
	float getTreeSize() {
		float ms = 0.0f;
		for(int i = 0; i < size; ++i) {
			for(int j = 0; j < 3; ++j) {
				float ps = fabs(parts[i].pos[j]);
				if(ps > ms) {
					ms = ps;
				} 
			}
		}
		return size;
	}
	
	void updateTree(cl::buffer_object *clbuf) {
		load_cl_parts(clbuf, parts.data());
		
		float tree_size = getTreeSize();
		
		PBranch trunk(nullfvec3, tree_size, tree_depth);
		for(int i = 0; i < size; ++i) {
			trunk.add(&parts[i]);
		}
		trunk.update();
		
		storeTree(&trunk);
	}
	
	virtual void solve(float dt) override {
		
		// solve
		
		if(features & RK4) {
			// stage 1
			updateTree(buffers["part0"]);
			kernels["solve_tree_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["deriv0"], 
				buffers["tree"], buffers["tree_link"], buffers["tree_data"],
				size, eps
			);
			kernels["solve_rk4_v_1_2"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv0"], size, dt
			);
			
			// stage 2
			updateTree(buffers["part1"]);
			kernels["solve_tree_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part1"], buffers["deriv1"], 
				buffers["tree"], buffers["tree_link"], buffers["tree_data"],
				size, eps
			);
			kernels["solve_rk4_v_1_2"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv1"], size, dt
			);
			
			// stage 3
			updateTree(buffers["part1"]);
			kernels["solve_tree_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part1"], buffers["deriv2"], 
				buffers["tree"], buffers["tree_link"], buffers["tree_data"],
				size, eps
			);
			kernels["solve_rk4_v_3"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv2"], size, dt
			);
			
			// stage 4
			updateTree(buffers["part1"]);
			kernels["solve_tree_rk4_d"]->evaluate(
				cl::work_range(size), buffers["part1"], buffers["deriv3"],  
				buffers["tree"], buffers["tree_link"], buffers["tree_data"],
				size, eps
			);
			kernels["solve_rk4_v_4"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"], 
				buffers["deriv0"], buffers["deriv1"], buffers["deriv2"], buffers["deriv3"], 
				size, dt
			);
		} else {
			updateTree(buffers["part0"]);
			kernels["solve_tree_euler"]->evaluate(
				cl::work_range(size), buffers["part0"], buffers["part1"],
				buffers["tree"], buffers["tree_link"], buffers["tree_data"], 
				size, eps, dt
			);
		}
		
		cl::buffer_object *tmp = buffers["part0"];
		buffers["part0"] = buffers["part1"];
		buffers["part1"] = tmp;
		
		// write result to gl texture
		transfer_cl_to_gl();
		
		queue->flush();
	}
	
	int writeBranch(PBranch *b, int &pos, int &link_pos, int &data_pos) {
		int selfpos = pos;
		
		// store branch
		Branch sb;
		sb.count = b->count;
		sb.data = data_pos;
		sb.isleaf = b->leaf;
		sb.link = link_pos;
		branch_store(&sb, 0, tree_buffer + pos);
		pos += BRANCH_ISIZE;
		
		// store branch data
		BranchData sbd;
		sbd.barycenter = b->barycenter;
		sbd.mass       = b->mass;
		sbd.center     = b->center;
		sbd.size       = b->size;
		branch_data_store(&sbd, 0, tree_data_buffer + data_pos);
		data_pos += BRANCH_DATA_FSIZE;
		
		if(b->leaf) {
			for(int i = 0; i < int(b->buffer.size()); ++i) {
				const _Particle *p = b->buffer[i];
				
				// store particle data
				Particle sp;
				p->store(&sp);
				part_store(&sp, 0, tree_data_buffer + data_pos);
				data_pos += PART_FSIZE;
				
				// store particle id
				tree_link_buffer[link_pos] = p->id;
				link_pos += 1;
			}
		} else {
			int *link = tree_link_buffer + link_pos;
			link_pos += 8;
			for(int i = 0; i < 8; ++i) {
				link[i] = writeBranch(b->next[i], pos, link_pos, data_pos);
			}
		}
		
		return selfpos;
	}
	
	void storeTree(PBranch *trunk) {
		int pos = 0, link_pos = 0, data_pos = 0;
		writeBranch(trunk, pos, link_pos, data_pos);
		buffers["tree"]->store_data(tree_buffer);
		buffers["tree_link"]->store_data(tree_link_buffer);
		buffers["tree_data"]->store_data(tree_data_buffer);
	}
};
