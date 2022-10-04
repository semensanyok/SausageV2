#pragma once

#include "sausage.h"
#include "ShaderStruct.h"
#include "FileWatcher.h"
#include "Renderer.h"

class ShaderManager : public SausageSystem {
  friend class SystemsManager;
  unordered_map<unsigned int, Shader*> initialized_shaders;
public:
  FileWatcher* file_watcher;
  Renderer* renderer;
  Shaders* all_shaders = nullptr;
  Camera* camera;
  ShaderManager(FileWatcher* file_watcher, Renderer* renderer,
    Camera* camera)
    : file_watcher{ file_watcher }, renderer{ renderer }, camera{ camera } {};
private:
  void SetupShaders();
  Shader* _RegisterShader(const char* vs_name, const char* fs_name);
  Shader* _CreateShader(const char* vs_name, const char* fs_name);
};
