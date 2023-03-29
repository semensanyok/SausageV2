#include "ShaderManager.h"

void ShaderManager::SetupShaders() {

  all_shaders = new Shaders{
    _RegisterShader("blinn_phong_vs.glsl", "blinn_phong_fs.glsl"),
    _RegisterShader("debug_vs.glsl", "debug_fs.glsl"),
    //_RegisterShader("stencil_vs.glsl", "stencil_fs.glsl"),
    _RegisterShader("ui_font_vs.glsl", "ui_font_fs.glsl"),
    _RegisterShader("ui_back_vs.glsl", "ui_back_fs.glsl"),
    //_RegisterShader("3d_overlay_vs.glsl", "3d_overlay_fs.glsl"),
    _RegisterShader("static_vs.glsl", "static_fs.glsl"),
  };
  all_shaders->blinn_phong->SetMat4Uniform(string("projection_view"), &(camera->projection_view));
  all_shaders->blinn_phong->SetVec3Uniform(string("view_pos"), &(camera->pos));

  all_shaders->mesh_static->SetMat4Uniform(string("projection_view"), &(camera->projection_view));
  all_shaders->mesh_static->SetVec3Uniform(string("view_pos"), &(camera->pos));

  all_shaders->bullet_debug->SetMat4Uniform(string("projection_view"), &(camera->projection_view));

  all_shaders->font_ui->SetMat4Uniform(string("projection_ortho"), &(camera->projection_matrix_ortho));

  all_shaders->back_ui->SetMat4Uniform(string("projection_ortho"), &(camera->projection_matrix_ortho));

  //all_shaders->overlay_3d->SetMat4Uniform(string("projection_view"), &(camera->projection_view));
}

Shader* ShaderManager::_RegisterShader(const char* vs_name, const char* fs_name) {
  auto shader = _CreateShader(vs_name, fs_name);

  bool is_persistent_command = false;
  function<void()> fs_reload_callback = bind(&Shader::ReloadFS, shader);
  fs_reload_callback = bind(&Renderer::AddGlCommand, renderer, fs_reload_callback, is_persistent_command);
  file_watcher->AddCallback(shader->fragment_path, fs_reload_callback);

  function<void()> vs_reload_callback = bind(&Shader::ReloadVS, shader);
  vs_reload_callback = bind(&Renderer::AddGlCommand, renderer, vs_reload_callback, is_persistent_command);
  file_watcher->AddCallback(shader->vertex_path, vs_reload_callback);

  return shader;
}

Shader* ShaderManager::_CreateShader(const char* vs_name, const char* fs_name) {
  for (auto shader : initialized_shaders) {
    if (shader.second->vertex_path.ends_with(vs_name) ||
      shader.second->fragment_path.ends_with(fs_name)) {
      LOG((ostringstream() << "Shader with vs_name=" << string(vs_name)
        << " fs_name=" << string(fs_name)
        << " already registered")
        .str());
      return shader.second;
    }
  }
  Shader* shader = new Shader(GetShaderPath(vs_name), GetShaderPath(fs_name));
  shader->InitOrReload();
  initialized_shaders[shader->id] = shader;
  return shader;
}
