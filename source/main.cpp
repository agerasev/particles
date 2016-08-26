#include <cstdlib>
#include <cmath>

#include <vector>
#include <random>
#include <functional>

#include "const.hpp"

#include <la/vec.hpp>

#include "glbank.hpp"
#include "render.hpp"
#include "engine.hpp"
#include "particle.hpp"

//#include "solvercpu.hpp"
#include "solvergpu.hpp"
#include "solverhybrid.hpp"

double clamp(double a) {
	if(a < 0.0)
		return 0.0;
	if(a > 1.0)
		return 1.0;
	return a;
}

void distrib_galaxy(const int size, ParticleCPU *parts, std::function<float()> rand) {
	float side = 2.0*pow(double(size), 1.0/2.0);
	float nr = side/sqrt(2*_M_PI);
	float na = side*sqrt(2*_M_PI);
	
	int ca = 0, cr = 1;
	for(int i = 0; i < size; ++i) {
		ParticleCPU p;
		int cna = int(na*cr/nr);
		
		float a = 2*_M_PI*float(ca)/cna;
		float r = cr/nr;
		float x = r*cos(a);
		float y = r*sin(a);
		float z = 0.0;
		float l = sqrt(x*x + y*y + z*z);
		
		p.id = i;
		
		p.mass = 1e3/size;
		p.update();
		
		p.pos = fvec3(x, y, z);
		p.vel = 
			0.1*fvec3(rand(), rand(), rand()) + 
			0.7f*fvec3(-y, x, 0)*sqrt(l);
		
		float _a = 3*a/_M_PI;
		float _r = clamp(2 - fabs((_a > 3 ? _a - 6 : _a)));
		float _g = clamp(2 - fabs(_a - 2));
		float _b = clamp(2 - fabs(_a - 4));
		p.color = (clamp(1 - 2*r) + 0.1)*fvec3(1, 1, 1) + clamp(2*r)*fvec3(_r, _g, _b);
		p.color = normalize(p.color);
		
		parts[i] = p;
		
		ca += 1;
		if(ca >= cna) {
			cr += 1;
			ca = 0;
		}
	}
}

void distrib_cube(const int size, ParticleCPU *parts, std::function<float()> rand) {
	int side = round(pow(size, 1.0/3.0));
	
	for(int i = 0; i < size; ++i) {
		ParticleCPU p;
		
		float x = float((i/side)/side)/side - 0.5;
		float y = float((i/side)%side)/side - 0.5;
		float z = float(i%side)/side - 0.5;
		
		p.id = i;
		
		p.mass = 1e3/size;
		p.update();
		
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

#include <cl/session.hpp>
#include <cl/platform.hpp>
#include <cl/device.hpp>

int main(int argc, char *argv[]) {
	Engine engine(800, 800);//, Engine::RECORD);
	
	const int size = 16*1024;
	
	int features = 0; //Solver::RK4;
	
	//SolverCPU
	//SolverGPU
	SolverHybrid 
		solver(size, features);
	
	solver.dt = 2e-2;
	
	RenderGL gfx(&solver);
	
	engine.setRender(&gfx);
	engine.setSolver(&solver);
	
	{
		std::vector<ParticleCPU> parts(size);
		
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
