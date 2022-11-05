#pragma once

#include "sausage.h"
#include "Texture.h"
#include "FileWatcher.h"

using namespace std;

class Shader
{
  friend class ShaderManager;
  string vertex_path;
  string fragment_path;
  GLuint vs = 0, fs = 0;
  GLuint vs_in_use = 0, fs_in_use = 0;
  bool is_vs_updated = false;
  bool is_fs_updated = false;
  unordered_map<string, mat4*> mat4_uniforms;
  unordered_map<string, vec3*> vec3_uniforms;
  inline static unsigned int total_shaders = 0;
public:
  const unsigned int shader_id;
  GLuint	id = 0;

  bool is_active = true;
  inline friend ostream& operator<<(ostream& stream, const Shader& shader) {
    stream << "Shader(id=" << shader.id << ",vertex shader name=" << shader.vertex_path << ",fragment shader name=" << shader.fragment_path << ")";
    return stream;
  }
  ~Shader() { _Dispose(); };
  Shader(string vertex_path, string fragment_path) :
    vertex_path{ vertex_path },
    fragment_path{ fragment_path },
    shader_id { total_shaders++ }
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
  void CreateProgram();
  void CompileFS();
  void CompileVS();
  void CompileShader(std::string& code,
      GLuint shader_type,
      bool& out_is_shader_updated,
      GLuint& out_shader_id);
  void _Dispose();
  string _LoadCode(string& path);
  // utility function for checking shader compilation/linking errors.
  // ------------------------------------------------------------------------
  bool CheckCompileErrors(GLuint shader, string type, string info);
};

// hash function for unordered map
template<> struct std::hash<Shader> {
  size_t operator()(Shader const& t) const {
    return t.shader_id;
  }
};

// eq for hashmap/hashset
inline bool operator==(const Shader& lhs, const Shader& rhs) {
  return lhs.shader_id == rhs.shader_id;
}
inline bool operator<(const Shader& lhs, const Shader& rhs) {
  return lhs.shader_id < rhs.shader_id;
}
