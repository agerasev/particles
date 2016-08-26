#pragma once

#include <cstdio>

#include <string>
#include <map>
#include <regex>

#include <GL/glew.h>

#include "gl/shader.hpp"
#include "gl/program.hpp"
#include "gl/framebuffer.hpp"

#include "glbank.hpp"
#include "solver.hpp"

#include <la/vec.hpp>
#include <la/mat.hpp>

class Proj {
public:
	float f = 1e4, n = 1e-2;
	float w = n, h = n;
	fmat4 mat = unifmat4;

	void update(int sw, int sh) {
		w = (float)sw/sh*h;
		mat(0, 0) = n/w;
		mat(1, 1) = n/h;
		mat(2, 2) = -(f + n)/(f - n);
		mat(3, 2) = -2*f*n/(f - n);
		mat(2, 3) = -1;
		mat(3, 3) = 0;
	}
};

class View {
public:
	float eps = 1e-4f;
	
	float zbase = 1.1;
	float rad = 1.0f;
	
	float sens = 1e-2f;
	float phi = 0.0f, theta = _M_PI_4;
	
	fmat4 mat = unifmat4;
	
	void xyz(fvec3 p) {
		fvec3 z = normalize(p);
		fvec3 x = normalize(fvec3(0, 0, 1) ^ z);
		fvec3 y = z ^ x;
		mat = invert(fmat4(
			x.x(), y.x(), z.x(), p.x(),
			x.y(), y.y(), z.y(), p.y(),
			x.z(), y.z(), z.z(), p.z(),
			0, 0, 0, 1
		));
	}
	
	void sphere(float rad, float phi, float theta) {
		float sp = sin(phi), cp = cos(phi);
		float st = sin(theta), ct = cos(theta);
		xyz(fvec3(rad*sp*ct, rad*cp*ct, rad*st));
	}
	
	void spin(float dx, float dy) {
		phi += sens*dx;
		theta += sens*dy;
		if(theta > _M_PI_2 - eps)
			theta = _M_PI_2 - eps;
		if(theta < -_M_PI_2 + eps)
			theta = -_M_PI_2 + eps;
	}
	
	void zoom(float dz) {
		rad *= pow(zbase, -dz);
	}
	
	void update() {
		sphere(rad, phi, theta);
	}
};

class Render
{
protected:
	Solver *solver;
	
	Proj proj;
	View view;
	int width = 0, height = 0;
	
public:
	Render(Solver *solver) : solver(solver) {
		view.update();
	}
	virtual ~Render() = default;
	
	void zoom(float dz) {
		view.zoom(dz);
		view.update();
	}
	
	void spin(float dx, float dy) {
		view.spin(dx, dy);
		view.update();
	}
	
	void resize(int w, int h) {
		width = w;
		height = h;
		proj.update(width, height);
	}
	
	virtual void render() = 0;
};

class RenderGL : public Render {
protected:
	int size;
	std::vector<ParticleCPU> parts;
	
	GLBank bank;
	
	gl::Texture *sprop = nullptr;
	gl::Texture *dprop = nullptr;
	
	int maxts = 0;
	const int ps = 2;
	
	ivec2 texs;
	std::vector<float> gl_buffer;
	
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
	
public:
	RenderGL(Solver *solver) : Render(solver) {
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxts);
		printf("GL_MAX_TEXTURE_SIZE: %d\n", maxts);
		
		glPointSize(2);
		//glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_LEQUAL);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
		glEnable(GL_BLEND);
		glClearColor(0.0f,0.0f,0.0f,1.0f);
		
		size = solver->size;
		parts.resize(size);
		
		texs = split_size(size*ps);
		gl_buffer.resize(4*ps*size);
		
		sprop = new gl::Texture();
		dprop = new gl::Texture();
		sprop->init(2, texs.data(), gl::Texture::RGBA32F);
		dprop->init(2, texs.data(), gl::Texture::RGBA32F);
	}
	
	virtual ~RenderGL() {
		delete sprop;
		delete dprop;
	}
	
	void store_gls() {
		float *buf = gl_buffer.data();
		
		for(int i = 0; i < size; ++i) {
			const ParticleCPU &p = parts[i];
	
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
	
	void store_gld() {
		float *buf = gl_buffer.data();
		
		for(int i = 0; i < size; ++i) {
			const ParticleCPU &p = parts[i];
	
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
	
	void render() {
		size = solver->size;
		solver->load(parts.data());
		store_gls();
		store_gld();
		
		gl::FrameBuffer::unbind();
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		gl::Program *prog = nullptr;
		
		float f = 4;
		
		prog = bank.progs["draw-point"];
		prog->setUniform("u_wh", ivec2(width, height).data(), 2);
		prog->setUniform("u_f", f);
		prog->setUniform("u_proj", proj.mat.data(), 16);
		prog->setUniform("u_view", view.mat.data(), 16);
		prog->setUniform("u_sprop", sprop);
		prog->setUniform("u_dprop", dprop);
		prog->setUniform("MAXTS", maxts);
		prog->evaluate(GL_POINTS, 0, size);
		
		prog = bank.progs["draw-quad"];
		prog->setUniform("u_h", height);
		prog->setUniform("u_f", f);
		prog->setUniform("u_proj", proj.mat.data(), 16);
		prog->setUniform("u_view", view.mat.data(), 16);
		prog->setUniform("u_sprop", sprop);
		prog->setUniform("u_dprop", dprop);
		prog->setUniform("MAXTS", maxts);
		prog->evaluate(GL_QUADS, 0, 4*size);
		
		glFlush();
		glFinish();
	}
};
