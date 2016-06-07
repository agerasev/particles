#version 130

#include <defs.glsl>

uniform sampler2D u_src;

in vec2 vf_pos;

out vec4 f_color;

void main(void) {
	f_color = texture(u_src, vec2(0.5) + 0.5*vf_pos);
}
