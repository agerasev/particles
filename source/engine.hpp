#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "graphics.hpp"

class Engine {
private:
	int width, height;
	SDL_Window *window;
	SDL_GLContext context;
	Graphics *gfx = nullptr;
	
	bool hold = false;

public:
	Engine(int w, int h) {
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
					if(gfx != nullptr) {
						gfx->resize(width, height);
					}
				}
			} else if(event.type == SDL_MOUSEMOTION) {
				if(hold)
					gfx->spin(event.motion.xrel, event.motion.yrel);
			} else if(event.type == SDL_MOUSEBUTTONDOWN) {
				if(event.button.button == SDL_BUTTON_LEFT)
					hold = true;
			} else if(event.type == SDL_MOUSEBUTTONUP) {
				if(event.button.button == SDL_BUTTON_LEFT)
					hold = false;
			} else if(event.type == SDL_MOUSEWHEEL) {
				if(event.wheel.y)
					gfx->zoom(event.wheel.y);
			}
		}
		return true;
	}
	
	void loop() {
		while(handle()) {
			gfx->render();
			SDL_GL_SwapWindow(window);
		}
	}
	
	void setGraphics(Graphics *gfx) {
		this->gfx = gfx;
		gfx->resize(width, height);
	}
};
