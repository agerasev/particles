#include <head.glsl>

#include "property.glsl"

uniform mat4 u_view;
uniform mat4 u_proj;

flat out int vf_id;

void main() {
	int id = gl_VertexID;
	
	vec4 pos = vec4(pos(id), 1);
	
	vf_id = id;
	gl_Position = u_proj*(u_view*pos);
}
