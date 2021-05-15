#pragma once

#include "sausage.h"
#include <string>
#include <fstream>
#include <queue>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

void LOG(const string& s);

thread LogIO(bool& quit);

void LogShaderMessage(unsigned int id);

string glGetErrorString(GLenum error);

void WriteShaderMsgsToLogFile();

const char* GL_type_to_string(GLenum type);

void LogShaderFull(GLuint programme);
