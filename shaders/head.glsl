#pragma once

#version 130

#include <defs.glsl>

uniform int MAXTS; // MAX_TEXTURE_SIZE

ivec2 split_id(int id) {
	return ivec2(id%MAXTS, id/MAXTS);
}

int restore_id(ivec2 c) {
	return c.x + c.y*MAXTS;
}
