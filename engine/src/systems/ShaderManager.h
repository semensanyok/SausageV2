#pragma once

#include "sausage.h"
#include "SystemsManager.h"
#include "structures/Structures.h"
#include "FileWatcher.h"
#include "Renderer.h"

class ShaderManager : public SausageSystem {
  friend class SystemsManager;
  map<unsigned int, Shader*> initialized_shaders;
public:
  Shaders* all_shaders = nullptr;
  ShaderManager() {};
private:
  void SetupShaders();
  Shader* _RegisterShader(const char* vs_name, const char* fs_name);
  Shader* _CreateShader(const char* vs_name, const char* fs_name);
};
