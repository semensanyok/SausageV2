#pragma once

#include "sausage.h"
#include "Texture.h"
#include "FileWatcher.h"

using namespace std;

class Shader
{
public:
	GLuint id = 0;
	bool is_active = true;
	string vertex_path;
	string fragment_path;
	GLuint vs = 0, fs = 0;
	GLuint vs_in_use = 0, fs_in_use = 0;

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
		CompileShaders();
		CheckGLError();
		CreateProgram();
		CheckGLError();
	}
	void CreateProgram() {
		if (id != 0 && fs_in_use != 0) {
			glDetachShader(id, fs_in_use);
		}
		if (id != 0 && vs_in_use != 0) {
			glDetachShader(id, vs_in_use);
		}
		vs_in_use = vs;
		fs_in_use = fs;
		// shader Program
		if (id == 0) {
			id = glCreateProgram();
		}
		glAttachShader(id, vs_in_use);
		CheckGLError();
		glAttachShader(id, fs_in_use);
		CheckGLError();
		glLinkProgram(id);
		CheckGLError();
		checkCompileErrors(id, "PROGRAM");
		// delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(vs_in_use);
		glDeleteShader(fs_in_use);
		CheckGLError();
	}
	void CompileShaders() {
		//static map<string, GLuint> path_to_shader_id;
		//
		//auto existing_vs = path_to_shader_id.find(vertex_path);
		//if (existing_vs != path_to_shader_id.end()) {
		//	vs = (*existing_vs).second;
		//}
		//else {
		CheckGLError();
		auto code = _LoadCode(vertex_path);
		if (!code.empty()) {
			auto code_c = code.c_str();
			vs = glCreateShader(GL_VERTEX_SHADER);
			CheckGLError();
			glShaderSource(vs, 1, &code_c, NULL);
			CheckGLError();
			glCompileShader(vs);
			CheckGLError();
			checkCompileErrors(vs, "VERTEX");
		}

		//auto existing_fs = path_to_shader_id.find(fragment_path);
		//if (existing_fs != path_to_shader_id.end()) {
		//	fs = (*existing_fs).second;
		//}
		//else {
		code = _LoadCode(fragment_path);
		if (!code.empty()) {
			auto code_c = code.c_str();
			fs = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fs, 1, &code_c, NULL);
			glCompileShader(fs);
			checkCompileErrors(fs, "FRAGMENT");
		}

		CheckGLError();
	}
	void _Dispose() {
		glDeleteProgram(id);
		is_active = false;
	}
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
			cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << endl;
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
				cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
			}
		}
	}
};