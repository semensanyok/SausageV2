#pragma once

#include <string>
#include <filesystem>
using namespace std;

string GetShaderPath(const char* name) {
	filesystem::path spath(".");
	spath /= "assets";
	spath /= "shaders";
	spath /= name;
	return spath.string(); 
}