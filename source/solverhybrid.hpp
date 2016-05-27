#pragma once

#include <cstdio>
#include <cmath>

#include <vector>

#include "solvergpu.hpp"
#include "particle.hpp"
#include "octree.hpp"

class SolverHybrid : public SolverGPU {
private:
	Particle *parts = nullptr;
	
	int *tree_buffer = nullptr;
	int *tree_link_buffer = nullptr;
	float *tree_data_buffer = nullptr;
	
	const int 
		ts = 1, // tree entry size
		tls = 2, // tree_link entry size
		tds = 2; // tree_data entry size
	
	typedef Branch<const _Particle*> PBranch;
	
public:
	SolverHybrid(int size, GLBank *bank) : SolverGPU(size, bank) {
		parts = new _Particle[size];
		
		ivec2 texs;
		texs = split_size(tree_depth*size*ts);
		tree.init(2, texs.data(), gl::Texture::RGBA32I);
		tree_buffer = new int[4*texs[0]*texs[1]];
		
		texs = split_size(tree_depth*size*tls);
		tree_link.init(2, texs.data(), gl::Texture::RGBA32I);
		tree_link_buffer = new int[4*texs[0]*texs[1]];
		
		texs = split_size(tree_depth*size*tds);
		tree_data.init(2, texs.data(), gl::Texture::RGBA32F);
		tree_data_buffer = new float[4*texs[0]*texs[1]];
	}
	virtual ~SolverHybrid() {
		delete[] tree_buffer;
		delete[] tree_link_buffer;
		delete[] tree_data_buffer;
		delete[] parts;
	}
	
	virtual void load(_Particle parts[]) {
		for(int i = 0; i < size; ++i) {
			this->parts[i] = parts[i];
		}
		loadTex(parts);
	}
	
	/*
	virtual void solve(float dt, int steps = 1) override {
		downloadTex();
		
		Branch<const _Particle*> trunk(nullfvec3, tree_size, tree_depth);
		for(int i = 0; i < size; ++i) {
			trunk.add(&parts[i]);
		}
		trunk.update();
		loadTree(&trunk);
		
		loadTex(parts);
		
		gl::FrameBuffer *fb = nullptr;
		gl::Program *prog = nullptr;
		
		for(int i = 0; i < steps; ++i) {
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve-tree-euler"];
			prog->setUniform("u_sprop", sprop);
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			
			prog->setUniform("u_tree", &tree);
			prog->setUniform("u_tree_link", &tree_link);
			prog->setUniform("u_tree_data", &tree_data);
			prog->setUniform("u_gth", gth);
			prog->setUniform("u_eps", eps);
			
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
	*/
	
	void updateTree(gl::Texture *dprop) {
		downloadTex(dprop);
		
		PBranch trunk(nullfvec3, tree_size, tree_depth);
		for(int i = 0; i < size; ++i) {
			trunk.add(&parts[i]);
		}
		trunk.update();
		
		loadTree(&trunk);
	}
	
	void setUniforms(gl::Program *prog, float dt, int steps, bool set_tree = true) {
		prog->setUniform("u_sprop", sprop);
		
		if(set_tree) { 
			prog->setUniform("u_tree", &tree);
			prog->setUniform("u_tree_link", &tree_link);
			prog->setUniform("u_tree_data", &tree_data);
			prog->setUniform("u_gth", gth);
		}
		
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
			
			updateTree(dprops[0]->getTexture());
			
			fb = derivs[0];
			fb->bind();
			prog = bank->progs["solve-tree-rk4-d"];
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve-rk4-v-1-2"];
			prog->setUniform("u_deriv_1_2", derivs[0]->getTexture());
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps, false);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			// stage 2
			
			updateTree(dprops[1]->getTexture());
			
			fb = derivs[1];
			fb->bind();
			prog = bank->progs["solve-tree-rk4-d"];
			prog->setUniform("u_dprop", dprops[1]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve-rk4-v-1-2"];
			prog->setUniform("u_deriv_1_2", derivs[1]->getTexture());
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps, false);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			// stage 3
			
			updateTree(dprops[1]->getTexture());
			
			fb = derivs[2];
			fb->bind();
			prog = bank->progs["solve-tree-rk4-d"];
			prog->setUniform("u_dprop", dprops[1]->getTexture());
			setUniforms(prog, dt, steps);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			fb = dprops[1];
			fb->bind();
			prog = bank->progs["solve-rk4-v-3"];
			prog->setUniform("u_deriv_3", derivs[2]->getTexture());
			prog->setUniform("u_dprop", dprops[0]->getTexture());
			setUniforms(prog, dt, steps, false);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			// stage 4 
			
			updateTree(dprops[1]->getTexture());
			
			fb = derivs[3];
			fb->bind();
			prog = bank->progs["solve-tree-rk4-d"];
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
			setUniforms(prog, dt, steps, false);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			
			fb = dprops[1];
			dprops[1] = dprops[0];
			dprops[0] = fb;
		}
		
		dprop = dprops[0]->getTexture();
	}
	
	int writeBranch(Branch<const _Particle*> *b, int &pos, int &link_pos, int &data_pos) {
		int selfpos = pos;
		int *sb = tree_buffer + 4*pos;
		pos += 1;
		
		sb[0] = b->count;
		sb[1] = data_pos;
		
		float *data = tree_data_buffer + 4*data_pos;
		data_pos += 2;
		data[0] = b->barycenter.x();
		data[1] = b->barycenter.y();
		data[2] = b->barycenter.z();
		data[3] = b->mass;
		data[4] = b->center.x();
		data[5] = b->center.y();
		data[6] = b->center.z();
		data[7] = b->size;
		
		if(b->leaf) {
			sb[2] = 1;
			
			for(int i = 0; i < int(b->buffer.size()); ++i) {
				const _Particle *p = b->buffer[i];
				float *data = tree_data_buffer + 4*data_pos;
				data_pos += 2;
				data[0] = p->pos.x();
				data[1] = p->pos.y();
				data[2] = p->pos.z();
				data[3] = p->mass;
				data[4] = p->vel.x();
				data[5] = p->vel.y();
				data[6] = p->vel.z();
				data[7] = p->rad;
			}
		} else {
			sb[2] = 0;
			sb[3] = link_pos;
			
			int *link = tree_link_buffer + 4*link_pos;
			link_pos += 2;
			for(int i = 0; i < 8; ++i) {
				link[i] = writeBranch(b->next[i], pos, link_pos, data_pos);
			}
		}
		return selfpos;
	}
	
	void loadTree(Branch<const _Particle*> *trunk) {
		int pos = 0, link_pos = 0, data_pos = 0;
		writeBranch(trunk, pos, link_pos, data_pos);
		tree.write(
			tree_buffer, nullivec2.data(), split_size(pos).data(), 
			gl::Texture::RGBA_INT, gl::Type::INT
		);
		tree_link.write(
			tree_link_buffer, nullivec2.data(), split_size(link_pos).data(), 
			gl::Texture::RGBA_INT, gl::Type::INT
		);
		tree_data.write(
			tree_data_buffer, nullivec2.data(), split_size(data_pos).data(), 
			gl::Texture::RGBA, gl::Type::FLOAT
		);
	}
	
	void downloadTex(gl::Texture *dprop) {
		float *buf = buffer;
		
		sprop->read(buf, gl::Texture::RGBA, gl::Type::FLOAT);
		for(int i = 0; i < size; ++i) {
			_Particle &p = parts[i];
	
			p.rad  = buf[4*(i*ps + 0) + 0];
			p.mass = buf[4*(i*ps + 0) + 3];
		
			// color
			p.color.x() = buf[4*(i*ps + 1) + 0];
			p.color.y() = buf[4*(i*ps + 1) + 1];
			p.color.z() = buf[4*(i*ps + 1) + 2];
		}
		
		dprop->read(buf, gl::Texture::RGBA, gl::Type::FLOAT);
		for(int i = 0; i < size; ++i) {
			_Particle &p = parts[i];

			//position
			p.pos.x() = buf[4*(i*ps + 0) + 0];
			p.pos.y() = buf[4*(i*ps + 0) + 1];
			p.pos.z() = buf[4*(i*ps + 0) + 2];

			// velocity
			p.vel.x() = buf[4*(i*ps + 1) + 0];
			p.vel.y() = buf[4*(i*ps + 1) + 1];
			p.vel.z() = buf[4*(i*ps + 1) + 2];
		}
	}
};
