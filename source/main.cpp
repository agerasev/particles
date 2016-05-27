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
#include "solverhybrid.hpp"

double clamp(double a) {
	if(a < 0.0)
		return 0.0;
	if(a > 1.0)
		return 1.0;
	return a;
}

void distrib_galaxy(const int size, _Particle *parts, std::function<float()> rand) {
	float side = 2.0*pow(double(size), 1.0/2.0);
	float nr = side/sqrt(2*M_PI);
	float na = side*sqrt(2*M_PI);
	
	int ca = 0, cr = 1;
	for(int i = 0; i < size; ++i) {
		_Particle p;
		int cna = int(na*cr/nr);
		
		float a = 2*M_PI*float(ca)/cna;
		float r = cr/nr;
		float x = r*cos(a);
		float y = r*sin(a);
		float z = 0.0;
		float l = sqrt(x*x + y*y + z*z);
		
		p.id = i;
		
		p.mass = 1e3/size;
		p.rad = 1e0/sqrt(size);
		
		p.pos = fvec3(x, y, z);
		p.vel = 
			0.1*fvec3(rand(), rand(), rand()) + 
			0.7f*fvec3(-y, x, 0)*sqrt(l);
		
		float _a = 3*a/M_PI;
		float _r = clamp(2 - fabs((_a > 3 ? _a - 6 : _a)));
		float _g = clamp(2 - fabs(_a - 2));
		float _b = clamp(2 - fabs(_a - 4));
		p.color = (clamp(1 - 2*r) + 0.1)*fvec3(1, 1, 1) + clamp(2*r)*fvec3(_r, _g, _b);
		
		parts[i] = p;
		
		ca += 1;
		if(ca >= cna) {
			cr += 1;
			ca = 0;
		}
	}
}

void distrib_cube(const int size, _Particle *parts, std::function<float()> rand) {
	int side = round(pow(size, 1.0/3.0));
	
	for(int i = 0; i < size; ++i) {
		_Particle p;
		
		float x = float((i/side)/side)/side - 0.5;
		float y = float((i/side)%side)/side - 0.5;
		float z = float(i%side)/side - 0.5;
		
		p.id = i;
		
		p.mass = 1e3/size;
		p.rad = 1e-0/sqrt(size);
		
		p.pos = fvec3(x, y, z);
		p.vel = 0.1*fvec3(rand(), rand(), rand());
		
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
	const int size = 256*1024;
	
	GLBank bank;
	//SolverCPU solver(size);
	//SolverGPU solver(size);
	SolverHybrid solver(size);
	solver.dt = 1e-2;
	solver.rk4 = false;
	
	Graphics gfx(&bank, &solver);
	
	engine.setGraphics(&gfx);
	engine.setSolver(&solver);
	
	{
		std::vector<_Particle> parts(size);
		
		std::minstd_rand rand_engine;
		std::uniform_real_distribution<> rand_dist(-0.5, 0.5);
		std::function<float()> rand = [&rand_engine, &rand_dist]() {
			return rand_dist(rand_engine);
		};
		
		distrib_galaxy(size, parts.data(), rand);
		//distrib_cube(size, parts.data(), rand);
		
		solver.store(parts.data());
	}
	
	engine.loop();
	
	return 0;
}
