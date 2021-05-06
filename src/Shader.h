#pragma once

#include "Texture.h"

class Shader
{
public:
	unsigned int id;
	vector<Texture> textures;

	bool operator==(Shader& other) {
		return id == other.id;
	}
	Shader() {};
	Shader(string vertexPath, string fragmentPath, vector<Texture> textures = {}) : textures{ textures }
	{
		// 1. retrieve the vertex/fragment source code from filePath
		string vertexCode;
		string fragmentCode;
		ifstream vShaderFile;
		ifstream fShaderFile;
		// ensure ifstream objects can throw exceptions:
		vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		try
		{
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			stringstream vShaderStream, fShaderStream;
			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (ifstream::failure& e)
		{
			cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();
		// 2. compile shaders
		unsigned int vertex, fragment;
		// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");
		// fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");
		// shader Program
		id = glCreateProgram();
		glAttachShader(id, vertex);
		glAttachShader(id, fragment);
		glLinkProgram(id);
		checkCompileErrors(id, "PROGRAM");
		// delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);

		BindTextures();
	}
	// activate the shader
	// ------------------------------------------------------------------------
	void use() const
	{
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
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

private:
	void BindTextures() {
		for (unsigned int texture_num = 0; texture_num < textures.size(); texture_num++) {
			// set name for texture
			glActiveTexture(GL_TEXTURE0 + texture_num); // active proper texture unit before binding
			// cout << GetTextureName(textures[texture_num], texture_num).c_str();
			// now set the sampler to the correct texture unit
			glUniform1i(glGetUniformLocation(id, GetTextureName(textures[texture_num], texture_num).c_str()), texture_num);
			// and finally bind the texture
			glBindTexture(GL_TEXTURE_2D, id);
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