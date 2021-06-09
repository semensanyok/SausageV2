#pragma once

#include "sausage.h"
#include "Texture.h"

using namespace std;

template <typename T>
string GetModelPath(T name) {
	using namespace filesystem;
	path spath(".");
    spath /= "assets";
    spath /= "models";
    spath /= name;
    return spath.string();
}

template <typename T>
string GetShaderPath(T name) {
	using namespace filesystem;
	path spath(".");
	spath /= "shaders";
	spath /= name;
	return spath.string(); 
}

template <typename T>
string GetTexturePath(T name) {
	using namespace filesystem;
	path spath(".");
	spath /= "assets";
	spath /= "textures";
	spath /= name;
	return spath.string();
}
