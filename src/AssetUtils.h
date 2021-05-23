#pragma once

#include "sausage.h"
#include <string>
#include <filesystem>
#include "Texture.h"

template <typename T>
string GetModelPath(T name) {
	std::filesystem::path spath(".");
    spath /= "assets";
    spath /= "models";
    spath /= name;
    return spath.string();
}

template <typename T>
string GetShaderPath(T name) {
	std::filesystem::path spath(".");
	spath /= "assets";
	spath /= "shaders";
	spath /= name;
	return spath.string(); 
}

template <typename T>
string GetTexturePath(T name) {
	std::filesystem::path spath(".");
	spath /= "assets";
	spath /= "textures";
	spath /= name;
	return spath.string();
}
