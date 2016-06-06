#include <cstdlib>
#include <cstdio>
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
#include "solverhybrid.hpp"

int main(int argc, char *argv[]) {
	fprintf(stderr, "engine ...\n");
	Engine engine(800, 800);
	
	const int size = 2*1024;
	
	fprintf(stderr, "glbank ...\n");
	GLBank bank;
	
	fprintf(stderr, "solver ...\n");
	//SolverCPU solver(size);
	SolverGPU solver(size, &bank);
	//SolverHybrid solver(size, &bank);
	solver.dt = 1e-2;
	solver.steps = 1;
	
	fprintf(stderr, "graphics ...\n");
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
		
		float side = 2.0*pow(double(size), 1.0/2.0);
		float nr = side/sqrt(2*M_PI);
		float na = side*sqrt(2*M_PI);
		
		int ca = 0, cr = 1;
		for(int i = 0; i < size; ++i) {
			Particle p;
			
			float a = 2*M_PI*ca/(na*cr/nr);
			float x = cr/nr*cos(a);
			float y = cr/nr*sin(a);
			float z = 0.0;
			float l = sqrt(x*x + y*y + z*z + 1e-2);
			
			p.mass = 1e3/size;
			p.rad = 6e-3;
			
			p.pos = fvec3(x, y, z);
			p.vel = 
				0.1*fvec3(rand(), rand(), rand()) + 
				0.5f*fvec3(-y, x, 0.0)*sqrt(l);
			
			p.color = fvec3(
				x + 0.5, 
				y + 0.5, 
				1.0 - 0.5*(x + y + 1.0)
			);
			
			parts[i] = p;
			
			ca += 1;
			if(ca > na*cr/nr) {
				cr += 1;
				ca = 0;
			}
		}
		solver.load(parts.data());
	}
	
	engine.loop();
	
	return 0;
}
