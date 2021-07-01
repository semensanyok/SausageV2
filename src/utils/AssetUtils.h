#pragma once

#include "sausage.h"
#include "Texture.h"

using namespace std;
using namespace filesystem;

template <typename T>
string GetModelPath(T name) {
	path spath(".");
    spath /= "assets";
    spath /= "models";
    spath /= name;
    return spath.string();
}

template <typename T>
vector<path> GetAnimationsPathsForModel(T& model_name) {
	vector<path> res;
	path spath(".");
	spath /= "assets";
	spath /= "models";
	auto anim_re_str = string(model_name).append("_").append("[^_]*").append("_").append("anim").append("\\.").append(".{1,5}");
	try {
		regex anim_re(anim_re_str);
		for (auto& p : filesystem::directory_iterator(spath)) {
			string filename = p.path().filename().string();
			smatch sm;
			if (regex_search(filename, sm, anim_re)) {
				res.push_back(p.path());
			};
		}
	}
	catch (const std::regex_error& e) {
		std::cout << "regex_error caught: " << e.what() << '\n';
		if (e.code() == std::regex_constants::error_brack) {
			std::cout << "The code was error_brack\n";
		}
	}
	return res;
}


template <typename T>
string GetShaderPath(T name) {
	path spath(".");
	spath /= "shaders";
	spath /= name;
	return spath.string(); 
}

template <typename T>
string GetTexturePath(T name) {
	path spath(".");
	spath /= "assets";
	spath /= "textures";
	spath /= name;
	return spath.string();
}
