#version 130

out vec2 vf_coord;
out vec3 vf_color;

void main() {
	int pid = gl_VertexID/4;
	int vid = gl_VertexID%4;
	float size = 0.08 - 0.005*float(pid);
	vec2 pos = vec2(0.1*float(pid), 0.0);
	vec2 dir = vec2(0.0, 0.0);
	if(vid == 0 || vid == 1) {
		dir.x = 1.0;
	} else if(vid == 2 || vid == 3) {
		dir.x = -1.0;
	}
	if(vid == 1 || vid == 2) {
		dir.y = 1.0;
	} else if(vid == 0 || vid == 3) {
		dir.y = -1.0;
	}
	vf_coord = dir;
	vf_color = (1.0 - 0.5*float(vid/2))*vec3(0.1*float(pid), 0.8 - 0.1*float(pid), 0.2);
	gl_Position = vec4(pos + size*dir, 0.0, 1.0);
}
