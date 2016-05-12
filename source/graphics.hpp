#pragma once

#include <cstdio>
#include <string>
#include <memory>

#include <GL/glew.h>

#include "gl/shader.hpp"
#include "gl/program.hpp"

class Graphics {
private:
	gl::Shader vert, frag;
	gl::Program prog;
	int width = 0, height = 0;
	
public:
	Graphics() :
	vert(gl::Shader::VERTEX), frag(gl::Shader::FRAGMENT)
	{
		// glEnable(GL_DEPTH_TEST);
		// glDepthFunc(GL_LEQUAL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClearColor(0.0f,0.0f,0.0f,1.0f);
		
		vert.loadSourceFromFile("shaders/shader.vert");
		frag.loadSourceFromFile("shaders/shader.frag");
		vert.compile();
		frag.compile();
		
		prog.setName("draw");
		prog.attach(&vert);
		prog.attach(&frag);
		prog.link();
	}
	
	~Graphics() {
		
	}
	
	void resize(int w, int h) {
		width = w;
		height = h;
		glViewport(0, 0, w, h);
	}
	
	void render() {
		glClear(GL_COLOR_BUFFER_BIT); // | GL_DEPTH_BUFFER_BIT);
		
		prog.evaluate(GL_QUADS, 0, 4*8);
		
		glFlush();
		glFinish();
	}
};
