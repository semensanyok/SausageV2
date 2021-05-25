#pragma once

#include "sausage.h"
#include "Logging.h"

void CheckGLError() {
    int err = glGetError();
    if (err != GL_NO_ERROR) {
        //LOG(glGetErrorString(err));
        cout << glGetErrorString(err) << endl;
    }
}

void LogGetShaderInfoLog(GLuint shader)
{
	GLchar infoLog[1024];
	glGetProgramInfoLog(shader, 1024, NULL, infoLog);
	LOG(string(infoLog));
}
