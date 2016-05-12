#version 130

in vec2 vf_coord;
in vec3 vf_color;

out vec4 f_color;

void main() {
	if(dot(vf_coord, vf_coord) <= 1.0) {
		f_color = vec4(vf_color, 1.0);
	} else {
		f_color = vec4(vf_color, 0.0);
	}
}
