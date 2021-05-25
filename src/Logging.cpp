#include "Logging.h"
#include "utils/ThreadSafeQueue.h"
#include <mutex>

static ofstream logstream("LOG.log");

static ThreadSafeQueue log_queue;

void LOG(const string& s) {
	log_queue.push(s);
}

thread LogIO(bool& quit) {
	return thread([&] {while (!quit) {
		//this_thread::sleep_for(std::chrono::milliseconds(300));
		auto s = log_queue.pop(quit);
		cout << s << endl;
		logstream << s << endl;
	}});
}

void LogShaderMessage(unsigned int id) {
	int max_length = 2048;
	int actual_length = 0;
	char program_log[2048];
	glGetProgramInfoLog(id, max_length, &actual_length, program_log);
	printf("program info log for GL index %u:\n%s", id, program_log);
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

void WriteShaderMsgsToLogFile()
{
	GLint maxMsgLen = 100000;
	glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &maxMsgLen);
	int numMsgs = 1000;
	std::vector<GLchar> msgData(numMsgs * maxMsgLen);
	std::vector<GLenum> sources(numMsgs);
	std::vector<GLenum> types(numMsgs);
	std::vector<GLenum> severities(numMsgs);
	std::vector<GLuint> ids(numMsgs);
	std::vector<GLsizei> lengths(numMsgs);

	GLuint numFound = glGetDebugMessageLog(numMsgs, maxMsgLen, &sources[0], &types[0], &ids[0], &severities[0], &lengths[0], &msgData[0]);
	ofstream out("shader.log", ios::out);
	while (numFound > 0) {

		sources.resize(numFound);
		types.resize(numFound);
		severities.resize(numFound);
		ids.resize(numFound);
		lengths.resize(numFound);

		std::vector<std::string> messages;
		messages.reserve(numFound);

		std::vector<GLchar>::iterator currPos = msgData.begin();
		for (size_t msg = 0; msg < lengths.size(); ++msg)
		{
			//messages.push_back(std::string(currPos, currPos + lengths[msg] - 1));
			out << " MESSAGE: " << string(currPos, currPos + lengths[msg] - 1) << endl;
			currPos = currPos + lengths[msg];
		}
		numFound = glGetDebugMessageLog(numMsgs, messages.size(), &sources[0], &types[0], &ids[0], &severities[0], &lengths[0], &msgData[0]);
	}
}

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
	glGetProgramiv(programme, GL_ACTIVE_TEXTURE_ARB, &params);
	printf("GL_ACTIVE_TEXTURE_ARB = %i\n", params);
	glGetProgramiv(programme, GL_ACTIVE_UNIFORM_BLOCKS, &params);
	printf("GL_ACTIVE_UNIFORM_BLOCKS = %i\n", params);
	glGetProgramiv(programme, GL_ACTIVE_VERTEX_UNITS_ARB, &params);
	printf("GL_ACTIVE_VERTEX_UNITS_ARB = %i\n", params);


	LogShaderMessage(programme);
}