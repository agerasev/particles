#include <cstdlib>

#include "graphics.hpp"
#include "engine.hpp"


int main(int argc, char *argv[]) {
	Engine engine(800, 800);
	Graphics gfx;
	engine.gfx = &gfx;
	
	engine.loop();
	
	return 0;
}
