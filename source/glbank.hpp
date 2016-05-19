#pragma once

#include <map>
#include <string>
#include <regex>

#include "gl/shader.hpp"
#include "gl/program.hpp"

class GLBank {
public:
	std::map<std::string, gl::Shader*> shaders;
	std::map<std::string, gl::Program*> progs;
	
	GLBank() {
		const char *shader_info[] = {
			"draw-quad.vert",
			"draw-quad.frag",
			"draw-point.vert",
			"draw-point.frag",
			"fill.vert",
			"solve-plain.frag"
		};
		std::regex re_v("^[a-zA-Z0-9.-]*\\.vert$"), re_f("^[a-zA-Z0-9.-]*\\.frag$");
		for(int i = 0; i < int(sizeof(shader_info)/sizeof(shader_info[0])); ++i) {
			std::string name(shader_info[i]);
			gl::Shader *shader = nullptr;
			if(std::regex_match(name, re_v)) {
				shader = new gl::Shader(gl::Shader::VERTEX);
			} else if(std::regex_match(name, re_f)) {
				shader = new gl::Shader(gl::Shader::FRAGMENT);
			} else {
				fprintf(stderr, (name + " has wrong extension\n").c_str());
			}
			if(shader == nullptr) {
				continue;
			}
			shader->setName(name);
			shader->loadSourceFromFile(name, "shaders");
			shader->compile();
			shaders.insert(std::pair<std::string, gl::Shader*>(name, shader));
		}
		
		const char *program_info[][3] = {
			{"draw-quad", "draw-quad.vert", "draw-quad.frag"},
			{"draw-point", "draw-point.vert", "draw-point.frag"},
			{"solve-plain", "fill.vert", "solve-plain.frag"}
		};
		for(int j = 0; j < int(sizeof(program_info)/sizeof(program_info[0])); ++j) {
			std::string name(program_info[j][0]);
			std::string vs_name(program_info[j][1]);
			std::string fs_name(program_info[j][2]);
			gl::Program *prog = new gl::Program();
			prog->setName(name);
			prog->attach(shaders[vs_name]);
			prog->attach(shaders[fs_name]);
			prog->link();
			progs.insert(std::pair<std::string, gl::Program*>(name, prog));
		}
	}
	
	~GLBank() {
		for(auto p : progs) {
			delete p.second;
		}
		
		for(auto p : shaders) {
			delete p.second;
		}
	}
};
