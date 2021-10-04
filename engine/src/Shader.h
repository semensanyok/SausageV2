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
	map<string, mat4*> mat4_uniforms;
	map<string, vec3*> vec3_uniforms;

	inline bool operator==(Shader& other) {
		return id == other.id;
	}
	inline friend ostream &operator<<(ostream& stream, const Shader& shader) {
		stream << "Shader(id=" << shader.id << ",vertex shader name=" << shader.vertex_path << ",fragment shader name=" << shader.fragment_path << ")";
		return stream;
	}
	~Shader() { _Dispose();  };
	Shader(string vertex_path, string fragment_path): vertex_path{ vertex_path }, fragment_path{ fragment_path }
	{
	}
	void InitOrReload();
	void ReloadVS();
	void ReloadFS();
	void SetMat4Uniform(string name, mat4* uniform) {
		mat4_uniforms[name] = uniform;
	}
	void SetVec3Uniform(string name, vec3* uniform) {
		vec3_uniforms[name] = uniform;
	}
	void SetUniforms() {
		for (auto& name_uni : mat4_uniforms) {
			setMat4(name_uni.first, *(name_uni.second));
		}
		for (auto& name_uni : vec3_uniforms) {
			setVec3(name_uni.first, *(name_uni.second));
		}
	}
private:
	void CreateProgram();
	void CompileFS();
	void CompileVS();
	void _Dispose();
public:
	void Use();
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const string& name, bool value) const;
	// ------------------------------------------------------------------------
	void setInt(const string& name, int value) const;
	// ------------------------------------------------------------------------
	void setFloat(const string& name, float value) const;
	// ------------------------------------------------------------------------
	void setVec2(const string& name, const glm::vec2& value) const;
	void setVec2(const string& name, float x, float y) const;
	// ------------------------------------------------------------------------
	void setVec3(const string& name, const glm::vec3& value) const;
	void setVec3(const string& name, float x, float y, float z) const;
	// ------------------------------------------------------------------------
	void setVec4(const string& name, const glm::vec4& value) const;
	void setVec4(const string& name, float x, float y, float z, float w) const;
	// ------------------------------------------------------------------------
	void setMat2(const string& name, const glm::mat2& mat) const;
	// ------------------------------------------------------------------------
	void setMat3(const string& name, const glm::mat3& mat) const;
	// ------------------------------------------------------------------------
	void setMat4(const string& name, const glm::mat4& mat) const;
private:
	string _LoadCode(string& path);
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void CheckCompileErrors(GLuint shader, string type);
};