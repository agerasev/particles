#include <head.glsl>

#include "property.glsl"

uniform mat4 u_view;
uniform mat4 u_proj;

flat out int vf_id;
flat out float vf_rad;
flat out vec2 vf_center;

void main() {
	int id = gl_VertexID;
	
	vec4 pos = vec4(pos(id), 1);
	float rad = rad(id);
	
	vec4 view_pos = u_view*pos;
	vf_rad = -rad/view_pos.z;
	
	vf_id = id;
	
	vec4 proj_pos = u_proj*view_pos;
	vf_center = proj_pos.xy/proj_pos.w;
	
	gl_Position = proj_pos;
}
