#include <head.glsl>

void main(void) {
	vec2 pos = vec2(0.0, 0.0);
	int id = gl_VertexID;
	if(id == 0 || id == 1) {
		pos.y = 1.0;
	} else if(id == 2 || id == 3) {
		pos.y = -1.0;
	}
	if(id == 1 || id == 2) {
		pos.x = -1.0;
	} else if(id == 0 || id == 3) {
		pos.x = 1.0;
	}
	gl_Position = vec4(pos, 0.0, 1.0);
}
