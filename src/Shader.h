#pragma once

#include "sausage.h"
#include "Texture.h"
#include "FileWatcher.h"

using namespace std;

class Shader
{
public:
	GLuint	id = 0;
	bool is_active = true;
	string vertex_path;
	string fragment_path;
	GLuint vs = 0, fs = 0;
	GLuint vs_in_use = 0, fs_in_use = 0;
	bool is_vs_updated = false;
	bool is_fs_updated = false;

	bool operator==(Shader& other) {
		return id == other.id;
	}
	friend ostream &operator<<(ostream& stream, const Shader& shader) {
		stream << "Shader(id=" << shader.id << ",vertex shader name=" << shader.vertex_path << ",fragment shader name=" << shader.fragment_path << ")";
		return stream;
	}
	~Shader() { _Dispose();  };
	Shader(string vertex_path, string fragment_path): vertex_path{ vertex_path }, fragment_path{ fragment_path }
	{
	}
	void InitOrReload() {
		CompileVS();
		CompileFS();
		CheckGLError();
		CreateProgram();
		CheckGLError();
	}
	void ReloadVS() {
		CompileVS();
		CheckGLError();
		CreateProgram();
		CheckGLError();
	}
	void ReloadFS() {
		CompileFS();
		CheckGLError();
		CreateProgram();
		CheckGLError();
	}
private:
	void CreateProgram() {
		// shader Program
		if (id == 0) {
			id = glCreateProgram();
		}
		if (is_vs_updated) {
			glAttachShader(id, vs);
			vs_in_use = vs;
			is_vs_updated = false;
		}
		CheckGLError();
		if (is_fs_updated) {
			glAttachShader(id, fs);
			fs_in_use = fs;
			is_fs_updated = false;
		}
		CheckGLError();
		glLinkProgram(id);
		CheckGLError();
		checkCompileErrors(id, "PROGRAM");
		// delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(vs_in_use);
		glDeleteShader(fs_in_use);
		CheckGLError();
	}
	void CompileFS() {
		if (id != 0 && fs_in_use != 0) {
			glDetachShader(id, fs_in_use);
		}
		auto code = _LoadCode(fragment_path);
		if (!code.empty()) {
			auto code_c = code.c_str();
			fs = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fs, 1, &code_c, NULL);
			glCompileShader(fs);
			checkCompileErrors(fs, "FRAGMENT");
		}
		if (CheckGLError() == false) {
			is_fs_updated = true;
		}
	}
	void CompileVS() {
		if (id != 0 && vs_in_use != 0) {
			glDetachShader(id, vs_in_use);
		}
		auto code = _LoadCode(vertex_path);
		if (!code.empty()) {
			auto code_c = code.c_str();
			vs = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vs, 1, &code_c, NULL);
			glCompileShader(vs);
			checkCompileErrors(vs, "VERTEX");
		}
		CheckGLError();
		if (CheckGLError() == false) {
			is_vs_updated = true;
		}
	}
	void _Dispose() {
		glDetachShader(id, vs_in_use);
		glDetachShader(id, fs_in_use);
		glDeleteProgram(id);
		is_active = false;
	}
public:
	void use() {
		glUseProgram(id);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const string& name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	void setVec2(const string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	void setVec3(const string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	void setVec4(const string& name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const string& name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const string& name, const glm::mat4& mat) const
	{
		int loc = glGetUniformLocation(id, name.c_str());
		glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
	}

private:
	string _LoadCode(string& path) {
		try
		{
			ifstream file;
			file.exceptions(ifstream::failbit | ifstream::badbit);
			file.open(path);
			stringstream sstream;
			// read file's buffer contents into streams
			sstream << file.rdbuf();
			// close file handlers
			file.close();
			// convert stream into string
			return sstream.str();
		}
		catch (ifstream::failure& e)
		{
			LOG((stringstream() << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what()).str());
			return string();
		}
	}
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(GLuint shader, string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				LOG((stringstream() << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- ").str());
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				LOG((stringstream() << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- ").str());
			}
		}
	}
};