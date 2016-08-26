#pragma once

#include <cstdio>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "solver.hpp"
#include "render.hpp"

class Engine {
public:
	static const int
		RECORD = (1 << 0);
	
private:
	int width, height;
	SDL_Window *window;
	SDL_GLContext context;
	
	RenderGL *render = nullptr;
	Solver *solver = nullptr;
	
	bool hold = false;
	
	std::vector<ParticleCPU> parts;
	
	int features = 0;

public:
	Engine(int w, int h, int features = 0) {
		SDL_Init(SDL_INIT_VIDEO);
		
		width = w;
		height = h;
		window = SDL_CreateWindow(
			"GLParticles",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width, height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
		);
		if(window == nullptr) {
			fprintf(stderr, "Could not create SDL_Window\n");
			exit(1);
		}
		
		context = SDL_GL_CreateContext(window);
		if(context == NULL) {
			fprintf(stderr, "Could not create SDL_GL_Context\n");
			exit(1);
		}
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		/*
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		*/
		SDL_GL_SetSwapInterval(1);
		
		GLenum status = glewInit();
		if(status != GLEW_OK) {
			fprintf(stderr, "Could not init GLEW: %s\n", glewGetErrorString(status));
			exit(1);
		}
		if(!GLEW_VERSION_3_0) {
			fprintf(stderr, "OpenGL 3.0 support not found\n");
			exit(1);
		}
		
		this->features = features;
	}
	~Engine() {
		SDL_GL_DeleteContext(context);
		
		SDL_DestroyWindow(window);
		
		SDL_Quit();
	}
	
	bool handle() {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				return false;
			} else if(event.type == SDL_WINDOWEVENT) {
				if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
					width = event.window.data1; 
					height = event.window.data2;
					if(render != nullptr) {
						render->resize(width, height);
					}
				}
			} else if(event.type == SDL_MOUSEMOTION) {
				if(hold)
					render->spin(event.motion.xrel, event.motion.yrel);
			} else if(event.type == SDL_MOUSEBUTTONDOWN) {
				if(event.button.button == SDL_BUTTON_LEFT)
					hold = true;
			} else if(event.type == SDL_MOUSEBUTTONUP) {
				if(event.button.button == SDL_BUTTON_LEFT)
					hold = false;
			} else if(event.type == SDL_MOUSEWHEEL) {
				if(event.wheel.y)
					render->zoom(event.wheel.y);
			}
		}
		return true;
	}
	
	template <typename T>
	static void write(FILE *file, T arg) {
		fwrite((void*) &arg, sizeof(T), 1, file);
	}
	
	template <typename T>
	static void write(FILE *file, T *arg, int size) {
		fwrite((void*) arg, size*sizeof(T), 1, file);
	}
	
	void save(const std::string &name) {
		FILE *file = fopen(name.c_str(), "wb");
		if(!file) {
			fprintf(stderr, "cannot open file %s\n", name.c_str());
			return;
		}
		
		solver->load(parts.data());
		
		write(file, 2); // verion
		write(file, solver->size);
		
		for(int i = 0; i < solver->size; ++i) {
			ParticleCPU p = parts[i];
			write(file, p.mass);
			write(file, p.rad);
			write(file, p.pos.data(), 3);
			
			unsigned char cc[4] = {0,0,0,0xff};
			for(int i = 0; i < 3; ++i)
				cc[i] = (unsigned char) (255*p.color[i]);
			write(file, cc, 4);
		}
		
		fclose(file);
	}
	
	void loop() {
		int counter = 0;
		while(handle()) {
			printf("frame %d\n", counter);
			fflush(stdout);
			
			Uint32 time = SDL_GetTicks();
			solver->solve();
			time = SDL_GetTicks() - time;
			printf("solve time: %f sec\n", 1e-3*time);
			fflush(stdout);
			
			if(features & RECORD) {
				char name[32];
				snprintf(name, sizeof(name), "record/scene%05d", counter); 
				save(name);
			}
			
			
			render->render();
			SDL_GL_SwapWindow(window);
			
			counter += 1;
		}
	}
	
	void setRender(RenderGL *gfx) {
		this->render = gfx;
		gfx->resize(width, height);
	}
	
	void setSolver(Solver *solver) {
		this->solver = solver;
		parts.resize(solver->size);
	}
};
