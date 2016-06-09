#pragma once

#include <vector>

#include <la/vec.hpp>

template <typename T>
class _Branch {
public:
	fvec3 center;
	float size;
	
	int max = 64;
	bool leaf = true;
	int depth;
	float minrad;
	int mindepth = 0;
	
	std::vector<T> buffer;
	
	_Branch *next[8] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	
	fvec3 barycenter;
	float mass;
	int count = 0;
	
	_Branch(fvec3 center, float size, int depth) 
	: center(center), size(size), depth(depth) {
		buffer.reserve(max);
		minrad = size;
		mindepth = depth;
	}
	~_Branch() {
		if(!leaf) {
			for(int i = 0; i < 8; ++i) {
				delete next[i];
			}
		}
	}
	void init_next() {
		leaf = false;
		for(int i = 0; i < 8; ++i) {
			fvec3 dir;
			for(int j = 0; j < 3; ++j) {
				dir[j] = ((i >> j) & 1) - 0.5;
			}
			next[i] = new _Branch<T>(
				center + dir*size,
				0.5*size, depth - 1
			);
		}
	}
	int get_next(fvec3 pos) const {
		fvec3 dir = pos - center;
		int i = 0;
		for(int j = 0; j < 3; ++j) {
			i += (1 << j)*(dir[j] >= 0);
		}
		return i;
	}
	void branch_leaf() {
		init_next();
		std::vector<T> tmpbuf = buffer;
		leaf = false;
		buffer.clear();
		minrad = size;
		for(int i = 0; i < int(tmpbuf.size()); ++i) {
			add(tmpbuf[i]);
		}
	}
	void add_next(T p) {
		next[get_next(p->pos)]->add(p);
	}
	void add(T p) {
		if(leaf || p->rad > 0.5*size) {
			if(p->rad < minrad) {
				minrad = p->rad;
			}
			buffer.push_back(p);
			if(leaf && (int(buffer.size()) >= max && depth > 0) && minrad <= 0.5*size) {
				branch_leaf();
			}
		} else {
			add_next(p);
		}
	}
	void update() {
		barycenter = nullfvec3;
		mass = 0.0f;
		if(!leaf) {
			for(int i = 0; i < 8; ++i) {
				next[i]->update();
				if(next[i]->mindepth < mindepth) {
					mindepth = next[i]->mindepth;
				}
				barycenter += next[i]->barycenter*next[i]->mass;
				mass += next[i]->mass;
			}
		}
		count = buffer.size();
		if(count > 0) {
			for(int i = 0; i < int(buffer.size()); ++i) {
				barycenter += buffer[i]->pos*buffer[i]->mass;
				mass += buffer[i]->mass;
			}
		}
		barycenter /= mass;
	}
};
