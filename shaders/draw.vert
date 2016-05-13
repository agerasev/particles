#version 130

uniform sampler2D u_sprop;
uniform sampler2D u_dprop;

out vec2 vf_coord;
out float vf_shadow;
flat out int vf_pid;

void main() {
	int pid = gl_VertexID/4;
	int vid = gl_VertexID%4;
	float rad = texelFetch(u_sprop, ivec2(pid, 0), 0).r;
	vec2 pos = texelFetch(u_dprop, ivec2(pid, 0), 0).xy;
	vec2 dir = vec2(0.0, 0.0);
	if(vid == 0 || vid == 1) {
		dir.y = 1.0;
	} else if(vid == 2 || vid == 3) {
		dir.y = -1.0;
	}
	if(vid == 1 || vid == 2) {
		dir.x = -1.0;
	} else if(vid == 0 || vid == 3) {
		dir.x = 1.0;
	}
	vf_pid = pid;
	vf_coord = dir;
	vf_shadow = 1.0 - 0.5*float(vid/2);
	gl_Position = vec4(pos + rad*dir, 0.0, 1.0);
}
