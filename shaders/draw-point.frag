#include <head.glsl>

#include <property.glsl>

flat in int vf_id;

out vec4 f_color;

void main() {
	f_color = vec4(color(vf_id), 1.0);
}
