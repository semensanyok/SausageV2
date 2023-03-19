#include "OpenGLHelpers.h"

using namespace std;

const char* GL_type_to_string(GLenum type) {
	switch (type) {
	case GL_BOOL: return "bool";
	case GL_INT: return "int";
	case GL_FLOAT: return "float";
	case GL_FLOAT_VEC2: return "vec2";
	case GL_FLOAT_VEC3: return "vec3";
	case GL_FLOAT_VEC4: return "vec4";
	case GL_FLOAT_MAT2: return "mat2";
	case GL_FLOAT_MAT3: return "mat3";
	case GL_FLOAT_MAT4: return "mat4";
	case GL_SAMPLER_2D: return "sampler2D";
	case GL_SAMPLER_3D: return "sampler3D";
	case GL_SAMPLER_CUBE: return "samplerCube";
	case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
	default: break;
	}
	return "other";
}

void LogGetShaderInfoLog(GLuint shader)
{
    GLchar infoLog[1024];
    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
    cout << string(infoLog) << endl;
    //LOG(string(infoLog));
}

string glGetErrorString(GLenum error)
{
	switch (error)
	{
	case GL_NO_ERROR:          return string("No Error");
	case GL_INVALID_ENUM:      return string("Invalid Enum");
	case GL_INVALID_VALUE:     return string("Invalid Value");
	case GL_INVALID_OPERATION: return string("Invalid Operation");
	case GL_INVALID_FRAMEBUFFER_OPERATION: return string("Invalid Framebuffer Operation");
	case GL_OUT_OF_MEMORY:     return string("Out of Memory");
	case GL_STACK_UNDERFLOW:   return string("Stack Underflow");
	case GL_STACK_OVERFLOW:    return string("Stack Overflow");
	case GL_CONTEXT_LOST:      return string("Context Lost");
	default:                   return to_string(error);
	}
}

void LogShaderMessage(unsigned int id) {
	int max_length = 2048;
	int actual_length = 0;
	char program_log[2048];
	glGetProgramInfoLog(id, max_length, &actual_length, program_log);
	printf("program info log for GL index %u:\n%s", id, program_log);
}
void LogShaderFull(GLuint programme) {
	printf("--------------------\nshader programme %i info:\n", programme);

	int params = -1;
	glGetProgramiv(programme, GL_LINK_STATUS, &params);
	printf("GL_LINK_STATUS = %i\n", params);

	glGetProgramiv(programme, GL_ATTACHED_SHADERS, &params);
	printf("GL_ATTACHED_SHADERS = %i\n", params);

	glGetProgramiv(programme, GL_ACTIVE_ATTRIBUTES, &params);
	printf("GL_ACTIVE_ATTRIBUTES = %i\n", params);
	for (int i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveAttrib(
			programme,
			i,
			max_length,
			&actual_length,
			&size,
			&type,
			name
		);
		if (size > 1) {
			for (int j = 0; j < size; j++) {
				char long_name[64];
				sprintf(long_name, "%s[%i]", name, j);
				int location = glGetAttribLocation(programme, long_name);
				printf("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string(type), long_name, location);
			}
		}
		else {
			int location = glGetAttribLocation(programme, name);
			printf("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string(type), name, location);
		}
	}

	glGetProgramiv(programme, GL_ACTIVE_UNIFORMS, &params);
	printf("GL_ACTIVE_UNIFORMS = %i\n", params);
	for (int i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveUniform(
			programme,
			i,
			max_length,
			&actual_length,
			&size,
			&type,
			name
		);
		if (size > 1) {
			for (int j = 0; j < size; j++) {
				char long_name[64];
				sprintf(long_name, "%s[%i]", name, j);
				int location = glGetUniformLocation(programme, long_name);
				printf("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string(type), long_name, location);
			}
		}
		else {
			int location = glGetUniformLocation(programme, name);
			printf("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string(type), name, location);
		}
	}
	//glGetProgramiv(programme, GL_ACTIVE_TEXTURE_ARB, &params);
	//printf("GL_ACTIVE_TEXTURE_ARB = %i\n", params);
	glGetProgramiv(programme, GL_ACTIVE_UNIFORM_BLOCKS, &params);
	printf("GL_ACTIVE_UNIFORM_BLOCKS = %i\n", params);
	LogShaderMessage(programme);
	CheckGLError();
}

// note: gl.... commands are valid only in render thread.
// any calls from other threads results in error.
bool CheckGLError(const std::source_location& location) {
  if (std::this_thread::get_id() != main_thread_id) {
    LOG((ostringstream() << "SKIP CheckGLError error called not inside main thread: '"
      << "' at: "
      << location.file_name() << "("
      << location.line() << ":"
      << location.column() << ")#"
      << location.function_name()));
    return false;
  }
  int err = glGetError();
  if (err != GL_NO_ERROR) {
    LOG((ostringstream()
      << "GL error: '" << glGetErrorString(err)
      << "' at: "
      << location.file_name() << "("
      << location.line() << ":"
      << location.column() << ")#"
      << location.function_name()));
    //LOG((ostringstream() << "GL error: " << glGetErrorString(err)));
    return true;
  }
  return false;
};

// cannot be constexpr because of `va_start`
//size_t GetPadCharsNumToLargestElement(int num, ...) {
//  va_list elements;
//  int total_size = 0;
//  int max_elem_size = 0;
//  va_start(elements, num);
//  for (int i = 0; i < num; i++) {
//    size_t elem = va_arg(elements, size_t);
//    total_size += elem;
//    max_elem_size = max_elem_size < elem ? elem : max_elem_size;
//  }
//  va_end(elements, num);
//
//  return (max_elem_size - total_size % max_elem_size) / sizeof(char);
//}

//constexpr size_t GetPadCharsNumToLargestElement(vector<size_t> sizes) {
//  int total_size = 0;
//  int max_elem_size = 0;
//  for (int i = 0; i < sizes.size(); i++) {
//    size_t elem = sizes[i];
//    total_size += elem;
//    max_elem_size = max_elem_size < elem ? elem : max_elem_size;
//  }
//  return (max_elem_size - total_size % max_elem_size) / sizeof(char);
//}
