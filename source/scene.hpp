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
		
		sprop = new gl::Texture();
		sarr[1] = 2;
		sprop->init(2, sarr, gl::Texture::RGBA8);
		buf = new float[4*sarr[0]*sarr[1]];
		for(int j = 0; j < sarr[1]; ++j) {
			for(int i = 0; i < sarr[0]; ++i) {
				if(j == 0) {
					buf[4*(j*sarr[0] + i) + 0] = 2e-2; // radius
					buf[4*(j*sarr[0] + i) + 1] = 0.0f;
					buf[4*(j*sarr[0] + i) + 2] = 0.0f;
					buf[4*(j*sarr[0] + i) + 3] = 1.0f; // inverse mass
				} else if(j == 1) {
					// color
					buf[4*(j*sarr[0] + i) + 0] = float(i)/(sarr[0] - 1);
					buf[4*(j*sarr[0] + i) + 1] = 1.0f - float(i)/(sarr[0] - 1);
					buf[4*(j*sarr[0] + i) + 2] = 0.0f;
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
				int side = ceil(pow(double(size), 1.0/3.0));
				for(int j = 0; j < sarr[1]; ++j) {
					for(int i = 0; i < sarr[0]; ++i) {
						if(j == 0) {
							//position
							buf[4*(j*sarr[0] + i) + 0] = float(i%side)/(side - 1) - 0.5f;
							buf[4*(j*sarr[0] + i) + 1] = float((i/side)%side)/(side - 1) - 0.5f;
							buf[4*(j*sarr[0] + i) + 2] = float(i/(side*side))/(side - 1) - 0.5f;
							buf[4*(j*sarr[0] + i) + 3] = 1.0f;
						} else if(j == 1) {
							// velocity
							float vc = 0.1f;
							buf[4*(j*sarr[0] + i) + 0] = vc*(random() - 0.5);
							buf[4*(j*sarr[0] + i) + 1] = vc*(random() - 0.5);
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
