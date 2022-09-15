#include "Shader.h"
using namespace std;
using namespace glm;

void Shader::InitOrReload() {
  CompileVS();
  CompileFS();
  CheckGLError();
  CreateProgram();
  CheckGLError();
}

void Shader::ReloadVS() {
  CompileVS();
  CheckGLError();
  CreateProgram();
  CheckGLError();
}
void Shader::ReloadFS() {
  CompileFS();
  CheckGLError();
  CreateProgram();
  CheckGLError();
}
void Shader::CreateProgram() {
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
  CheckCompileErrors(id, "PROGRAM");
  // delete the shaders as they're linked into our program now and no longer
  // necessary
  glDeleteShader(vs_in_use);
  glDeleteShader(fs_in_use);
  CheckGLError();
}
void Shader::CompileFS() {
  if (id != 0 && fs_in_use != 0) {
    glDetachShader(id, fs_in_use);
  }
  auto code = _LoadCode(fragment_path);
  if (!code.empty()) {
    auto code_c = code.c_str();
    CompileShader(code, GL_FRAGMENT_SHADER, is_fs_updated, fs);
  }
}
void Shader::CompileVS() {
  if (id != 0 && vs_in_use != 0) {
    glDetachShader(id, vs_in_use);
  }
  auto code = _LoadCode(vertex_path);
  if (!code.empty()) {
    CompileShader(code, GL_VERTEX_SHADER, is_vs_updated, vs);
  }
}
void Shader::CompileShader(
  std::string& code,
  GLuint shader_type,
  bool& out_is_shader_updated,
  GLuint& out_shader_id
)
{
  auto code_c = code.c_str();
  GLuint shader_id = glCreateShader(shader_type);
  glShaderSource(shader_id, 1, &code_c, NULL);
  glCompileShader(shader_id);
  auto is_success = CheckCompileErrors(shader_id,
    format("{} {}",
      shader_type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT",
      this->vertex_path
    ));
  if (is_success && CheckGLError() == false) {
    out_is_shader_updated = true;
    out_shader_id = shader_id;
  }
}

void Shader::_Dispose() {
  glDetachShader(id, vs_in_use);
  glDetachShader(id, fs_in_use);
  glDeleteProgram(id);
  is_active = false;
}
void Shader::Use() { glUseProgram(id); }
// utility uniform functions
// ------------------------------------------------------------------------
void Shader::setBool(const string &name, bool value) const {
  glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const string &name, int value) const {
  glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const string &name, float value) const {
  glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setVec2(const string &name, const glm::vec2 &value) const {
  glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const string &name, float x, float y) const {
  glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const string &name, const glm::vec3 &value) const {
  glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const string &name, float x, float y, float z) const {
  glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setVec4(const string &name, const glm::vec4 &value) const {
  glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const string &name, float x, float y, float z,
                     float w) const {
  glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(const string &name, const glm::mat2 &mat) const {
  glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const string &name, const glm::mat3 &mat) const {
  glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const string &name, const glm::mat4 &mat) const {
  int loc = glGetUniformLocation(id, name.c_str());
  glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

string Shader::_LoadCode(string &path) {
  try {
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
  } catch (ifstream::failure &e) {
    LOG((stringstream() << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: "
                        << e.what())
            .str());
    return string();
  }
}
// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
bool Shader::CheckCompileErrors(GLuint shader, string type) {
  GLint success;
  GLchar infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      LOG((stringstream()
           << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
           << infoLog
           << "\n -- --------------------------------------------------- -- ")
              .str());
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      LOG((stringstream()
           << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
           << infoLog
           << "\n -- --------------------------------------------------- -- ")
              .str());
    }
  }
  return success;
}
