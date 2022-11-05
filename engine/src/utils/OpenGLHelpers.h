#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Logging.h"

bool CheckGLError(const std::source_location& location = std::source_location::current());

const char* GL_type_to_string(GLenum type);

void LogGetShaderInfoLog(GLuint shader);

std::string glGetErrorString(GLenum error);

void LogShaderMessage(unsigned int id);

void LogShaderFull(GLuint programme);

// cannot be constexpr because of `va_start`
//size_t GetPadCharsNumToLargestElement(int num, ...);

//constexpr size_t GetPadCharsNumToLargestElement(vector<size_t> sizes);
