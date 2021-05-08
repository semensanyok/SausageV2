#pragma once

#include <string>
#include <filesystem>
#include "Texture.h"

using namespace std;

template <typename T>
string GetModelPath(T name) {
    filesystem::path spath(".");
    spath /= "assets";
    spath /= "models";
    spath /= name;
    return spath.string();
}

template <typename T>
string GetShaderPath(T name) {
	filesystem::path spath(".");
	spath /= "assets";
	spath /= "shaders";
	spath /= name;
	return spath.string(); 
}

template <typename T>
string GetTexturePath(T name) {
	filesystem::path spath(".");
	spath /= "assets";
	spath /= "textures";
	spath /= name;
	return spath.string();
}
