#pragma once

#include "sausage.h"

using namespace std;

#ifdef _MSC_VER
#define FUNCTION_ADDRESS __FUNCSIG__
#elif __GNUG__
#define FUNCTION_ADDRESS __PRETTY_FUNCTION__
#else
#define FUNCTION_ADDRESS __func__
#endif // _MSVC

void CheckGLError(const source_location& location = source_location::current());

const char* GL_type_to_string(GLenum type);

void LogGetShaderInfoLog(GLuint shader);

string glGetErrorString(GLenum error);

void LogShaderMessage(unsigned int id);

void LogShaderFull(GLuint programme);
