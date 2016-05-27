#pragma once

#include <la/vec.hpp>

template <typename T>
class _Branch {
public:
	fvec3 center;
	float size;
	
	static const int max = 8;
	bool leaf = true;
	int depth;
	
	std::vector<T> buffer;
	
	_Branch *next[8] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	
	fvec3 barycenter;
	float mass;
	int count = 0;
	
	_Branch(fvec3 center, float size, int depth) 
	: center(center), size(size), depth(depth) {
		buffer.reserve(max);
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
	void add_next(T p) {
		next[get_next(p->pos)]->add(p);
	}
	void add_current(T p) {
		if(int(buffer.size()) >= max && depth > 0) {
			init_next();
			add_next(p);
			for(int i = 0; i < int(buffer.size()); ++i) {
				add_next(buffer[i]);
			}
			buffer.clear();
		} else {
			buffer.push_back(p);
		}
	}
	void add(T p) {
		if(leaf) {
			add_current(p);
		} else {
			add_next(p);
		}
	}
	void update() {
		barycenter = nullfvec3;
		mass = 0.0f;
		count = 0;
		if(!leaf) {
			for(int i = 0; i < 8; ++i) {
				next[i]->update();
				if(next[i]->count > 0) {
					count += next[i]->count;
					barycenter += next[i]->barycenter*next[i]->mass;
					mass += next[i]->mass;
				}
			}
			barycenter /= mass;
		} else {
			count = buffer.size();
			if(count > 0) {
				for(int i = 0; i < int(buffer.size()); ++i) {
					barycenter += buffer[i]->pos*buffer[i]->mass;
					mass += buffer[i]->mass;
				}
				barycenter /= mass;
			}
		}
	}
};
