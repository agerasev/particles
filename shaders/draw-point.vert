#include <head.glsl>

#include "property.glsl"

uniform mat4 u_view;
uniform mat4 u_proj;

uniform sampler2D u_sprop;
uniform sampler2D u_dprop;

flat out int vf_id;

void main() {
	int id = gl_VertexID;
	
	vec4 pos = vec4(pos(u_dprop, id), 1);
	
	vf_id = id;
	gl_Position = u_proj*(u_view*pos);
}
