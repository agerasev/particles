#include <cstdlib>
#include <cmath>

#include <vector>
#include <random>
#include <functional>

#include <la/vec.hpp>

#include "glbank.hpp"
#include "solvercpu.hpp"
#include "solvergpu.hpp"
#include "graphics.hpp"
#include "engine.hpp"
#include "particle.hpp"

int main(int argc, char *argv[]) {
	Engine engine(800, 800);
	
	const int size = 4*1024;
	
	GLBank bank;
	SolverCPU solver(size);
	//SolverGPU solver(size, &bank);
	solver.dt = 1e-2;
	solver.steps = 1;
	
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
		
		int side = ceil(pow(double(size), 1.0/2.0));
		for(int i = 0; i < size; ++i) {
			Particle p;
			float x = float(i%side)/(side - 1) - 0.5;
			float y = float(i/side)/(side - 1) - 0.5;
			float z = 0.0;
			float l = sqrt(x*x + y*y + z*z + 1e-2);
			
			p.mass = 1e3/size;
			p.rad = 6e-3;
			
			p.pos = fvec3(x, y, 0.0);
			p.vel = 
				0.1*fvec3(rand(), rand(), rand()) + 
				1.0f*fvec3(-y, x, 0.0)*sqrt(l);
			
			p.color = fvec3(
				x + 0.5, 
				y + 0.5, 
				1.0 - 0.5*(x + y + 1.0)
			);
			
			parts[i] = p;
		}
		solver.load(parts.data());
	}
	
	engine.loop();
	
	return 0;
}
