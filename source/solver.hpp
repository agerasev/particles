#pragma once

#include <cmath>

#include <random>

#include "gl/texture.hpp"
#include "gl/framebuffer.hpp"

class Scene {
public:
	int size;
	gl::Texture *sprop;
	static const int NSTAGES = 2;
	gl::FrameBuffer *dprops[NSTAGES];
	
	std::minstd_rand rand_engine;
	std::uniform_real_distribution<> rand_dist;
	
	float random() {
		return float(rand_dist(rand_engine));
	}
	
public:
	Scene(int size) : rand_dist(0.0, 1.0) {
		this->size = size;
		int sarr[] = {size, 0};
		int zoarr[] = {0, 0};
		float *buf;
		
		int side = ceil(pow(double(size), 1.0/3.0));
		
		sprop = new gl::Texture();
		sarr[1] = 2;
		sprop->init(2, sarr, gl::Texture::RGBA8);
		buf = new float[4*sarr[0]*sarr[1]];
		for(int j = 0; j < sarr[1]; ++j) {
			for(int i = 0; i < sarr[0]; ++i) {
				float x = float(i%side)/(side - 1) - 0.5;
				float y = float((i/side)%side)/(side - 1) - 0.5;
				float z = float(i/(side*side))/(side - 1) - 0.5f;
				if(j == 0) {
					buf[4*(j*sarr[0] + i) + 0] = 1e-2; // radius
					buf[4*(j*sarr[0] + i) + 1] = 0.0f;
					buf[4*(j*sarr[0] + i) + 2] = 0.0f;
					buf[4*(j*sarr[0] + i) + 3] = 1.0f; // inverse mass
				} else if(j == 1) {
					// color
					buf[4*(j*sarr[0] + i) + 0] = x + 0.5;
					buf[4*(j*sarr[0] + i) + 1] = y + 0.5;
					buf[4*(j*sarr[0] + i) + 2] = z + 0.5;
					buf[4*(j*sarr[0] + i) + 3] = 1.0f;
				}
			}
		}
		sprop->write(buf, zoarr, sarr, gl::Texture::RGBA, gl::FLOAT);
		delete[] buf;
		
		sarr[1] = 2;
		for(int k = 0; k < NSTAGES; ++k) {
			gl::FrameBuffer *fb = new gl::FrameBuffer();
			fb->init(gl::Texture::RGBA32F, sarr[0], sarr[1]);
			if(k == 0) {
				float *buf = new float[4*sarr[0]*sarr[1]];
				for(int j = 0; j < sarr[1]; ++j) {
					for(int i = 0; i < sarr[0]; ++i) {
						float x = float(i%side)/(side - 1) - 0.5;
						float y = float((i/side)%side)/(side - 1) - 0.5;
						float z = float(i/(side*side))/(side - 1) - 0.5f;
						if(j == 0) {
							//position
							buf[4*(j*sarr[0] + i) + 0] = x;
							buf[4*(j*sarr[0] + i) + 1] = y;
							buf[4*(j*sarr[0] + i) + 2] = z;
							buf[4*(j*sarr[0] + i) + 3] = 1.0f;
						} else if(j == 1) {
							// velocity
							float tv = 0.1f, v = 0.2f;
							buf[4*(j*sarr[0] + i) + 0] = tv*(random() - 0.5) - v*y;
							buf[4*(j*sarr[0] + i) + 1] = tv*(random() - 0.5) + v*x;
							buf[4*(j*sarr[0] + i) + 2] = 0.0f;
							buf[4*(j*sarr[0] + i) + 3] = 1.0f;
						}
					}
				}
				fb->getTexture()->write(buf, zoarr, sarr, gl::Texture::RGBA, gl::FLOAT);
				delete[] buf;
			}
			dprops[k] = fb;
		}
	}
	
	~Scene() {
		for(int k = 0; k < NSTAGES; ++k) {
			delete dprops[k];
		}
		
		delete sprop;
	}
};
