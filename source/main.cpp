#include <cstdlib>
#include <cmath>

#include <vector>
#include <random>
#include <functional>

#include <la/vec.hpp>

#include "glbank.hpp"
#include "graphics.hpp"
#include "engine.hpp"
#include "particle.hpp"

#include "solvercpu.hpp"
#include "solvergpu.hpp"
//#include "solverhybrid.hpp"

void distrib_galaxy(const int size, Particle *parts, std::function<float()> rand) {
	float side = 2.0*pow(double(size), 1.0/2.0);
	float nr = side/sqrt(2*M_PI);
	float na = side*sqrt(2*M_PI);
	
	int ca = 0, cr = 1;
	for(int i = 0; i < size; ++i) {
		Particle p;
		int cna = int(na*cr/nr);
		
		float a = 2*M_PI*float(ca)/cna;
		float x = cr/nr*cos(a);
		float y = cr/nr*sin(a);
		float z = 0.0;
		float l = sqrt(x*x + y*y + z*z);
		
		p.id = i;
		
		p.mass = 1e3/size;
		p.rad = 5e-1/sqrt(size);
		
		p.pos = fvec3(x, y, z);
		p.vel = 
			0.1*fvec3(rand(), rand(), rand()) + 
			0.6f*fvec3(-y, x, 0)*sqrt(l);
		
		p.color = fvec3(
			x + 0.5, 
			y + 0.5, 
			1.0 - 0.5*(x + y + 1.0)
		);
		
		parts[i] = p;
		
		ca += 1;
		if(ca >= cna) {
			cr += 1;
			ca = 0;
		}
	}
}

void distrib_cube(const int size, Particle *parts, std::function<float()> rand) {
	int side = round(pow(size, 1.0/3.0));
	
	for(int i = 0; i < size; ++i) {
		Particle p;
		
		float x = float((i/side)/side)/side - 0.5;
		float y = float((i/side)%side)/side - 0.5;
		float z = float(i%side)/side - 0.5;
		
		p.id = i;
		
		p.mass = 1e3/size;
		p.rad = 1e-0/sqrt(size);
		
		p.pos = fvec3(x, y, z);
		p.vel = 
			0.1*fvec3(rand(), rand(), rand());
		
		p.color = fvec3(
			x + 0.5, 
			y + 0.5, 
			z + 0.5
		);
		
		parts[i] = p;
	}
}

int main(int argc, char *argv[]) {
	Engine engine(800, 800);
	
	//const int size = 2*1024 - 19; 
	//const int size = 4*1024 + 70;
	const int size = 16*1024;
	
	GLBank bank;
	//SolverCPU solver(size);
	SolverGPU solver(size);
	//SolverHybrid solver(size, &bank);
	solver.dt = 1e-2;
	
	Graphics gfx(&bank, &solver);
	
	engine.setGraphics(&gfx);
	engine.setSolver(&solver);
	
	{
		std::vector<Particle> parts(size);
		
		std::minstd_rand rand_engine;
		std::uniform_real_distribution<> rand_dist(-0.5, 0.5);
		std::function<float()> rand = [&rand_engine, &rand_dist]() {
			return rand_dist(rand_engine);
		};
		
		distrib_galaxy(size, parts.data(), rand);
		//distrib_cube(size, parts.data(), rand);
		
		solver.load(parts.data());
	}
	
	engine.loop();
	
	return 0;
}
