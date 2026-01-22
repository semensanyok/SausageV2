#ifndef OPENGLHELPERS_H
#define OPENGLHELPERS_H

#include "Logging.h"
#include "Settings.h"
#include "sausage.h"

bool CheckGLError(
    const std::source_location &location = std::source_location::current());

const char *GL_type_to_string(GLenum type);

void LogGetShaderInfoLog(GLuint shader);

std::string glGetErrorString(GLenum error);

void LogShaderMessage(unsigned int id);

void LogShaderFull(GLuint programme);

// cannot be constexpr because of `va_start`
// size_t GetPadCharsNumToLargestElement(int num, ...);

// constexpr size_t GetPadCharsNumToLargestElement(vector<size_t> sizes);

#endif // OPENGLHELPERS_H
