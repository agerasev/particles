#pragma once

#include <cstdio>

#include <string>
#include <map>
#include <regex>

#include <GL/glew.h>

#include "gl/shader.hpp"
#include "gl/program.hpp"

#include "solver.hpp"

#include <la/vec.hpp>
#include <la/mat.hpp>

class Proj {
public:
	float f = 1e2, n = 1e-2;
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
	float phi = 0.0f, theta = 0.0f;
	
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
		if(theta > M_PI_2 - eps)
			theta = M_PI_2 - eps;
		if(theta < -M_PI_2 + eps)
			theta = -M_PI_2 + eps;
	}
	
	void zoom(float dz) {
		rad *= pow(zbase, -dz);
	}
	
	void update() {
		sphere(rad, phi, theta);
	}
};

class Graphics {
private:
	Scene scene;
	
	std::map<std::string, gl::Shader*> shaders;
	std::map<std::string, gl::Program*> progs;
	
	Proj proj;
	View view;
	int width = 0, height = 0;
	
public:
	Graphics() : scene(512) {
		view.update();
		
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClearColor(0.0f,0.0f,0.0f,1.0f);
		
		const char *shader_info[] = {
			"draw.vert",
			"draw.frag",
			"fill.vert",
			"solve.frag"
		};
		std::regex re_v("^[a-zA-Z0-9.]*\\.vert$"), re_f("^[a-zA-Z0-9.]*\\.frag$");
		for(int i = 0; i < int(sizeof(shader_info)/sizeof(shader_info[0])); ++i) {
			std::string name(shader_info[i]);
			gl::Shader *shader = nullptr;
			if(std::regex_match(name, re_v)) {
				shader = new gl::Shader(gl::Shader::VERTEX);
			} else if(std::regex_match(name, re_f)) {
				shader = new gl::Shader(gl::Shader::FRAGMENT);
			} else {
				fprintf(stderr, (name + " has wrong extension\n").c_str());
			}
			if(shader == nullptr) {
				continue;
			}
			shader->setName(name);
			shader->loadSourceFromFile(name, "shaders");
			shader->compile();
			shaders.insert(std::pair<std::string, gl::Shader*>(name, shader));
		}
		
		const char *program_info[][3] = {
			{"draw", "draw.vert", "draw.frag"},
			{"solve", "fill.vert", "solve.frag"}
		};
		for(int j = 0; j < int(sizeof(program_info)/sizeof(program_info[0])); ++j) {
			std::string name(program_info[j][0]);
			std::string vs_name(program_info[j][1]);
			std::string fs_name(program_info[j][2]);
			gl::Program *prog = new gl::Program();
			prog->setName(name);
			prog->attach(shaders[vs_name]);
			prog->attach(shaders[fs_name]);
			prog->link();
			progs.insert(std::pair<std::string, gl::Program*>(name, prog));
		}
	}
	
	~Graphics() {
		for(auto p : progs) {
			delete p.second;
		}
		for(auto p : shaders) {
			delete p.second;
		}
	}
	
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
	
	void render() {
		gl::FrameBuffer *fb = nullptr;
		gl::Program *prog = nullptr;
		
		const int FREQ = 16;
		for(int i = 0; i < FREQ; ++i) {
			fb = scene.dprops[1];
			fb->bind();
			prog = progs["solve"];
			prog->setUniform("u_sprop", scene.sprop);
			prog->setUniform("u_dprop", scene.dprops[0]->getTexture());
			prog->setUniform("u_dt", 1e-2f/FREQ);
			prog->setUniform("u_count", scene.size);
			prog->evaluate(GL_QUADS, 0, 4);
			fb->unbind();
			scene.dprops[1] = scene.dprops[0];
			scene.dprops[0] = fb;
		}
		
		gl::FrameBuffer::unbind();
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		prog = progs["draw"];
		prog->setUniform("u_proj", proj.mat.data(), 16);
		prog->setUniform("u_view", view.mat.data(), 16);
		prog->setUniform("u_sprop", scene.sprop);
		prog->setUniform("u_dprop", scene.dprops[0]->getTexture());
		prog->evaluate(GL_QUADS, 0, 4*scene.size);
		
		glFlush();
		glFinish();
	}
};
