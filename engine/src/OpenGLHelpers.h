#pragma once

#include "sausage.h"
#include "Settings.h"

bool CheckGLError(const std::source_location& location = std::source_location::current());

const char* GL_type_to_string(GLenum type);

void LogGetShaderInfoLog(GLuint shader);

std::string glGetErrorString(GLenum error);

void LogShaderMessage(unsigned int id);

void LogShaderFull(GLuint programme);

bool CheckGLError(const std::source_location& location);