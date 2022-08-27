#pragma once

#include "FileWatcher.h"
#include "Settings.h"
#include "structures/Structures.h"
#include "sausage.h"
#include "ImguiGui.h"
#include "MeshManager.h"
#include "Renderer.h"
#include "SystemsManager.h"
#include "TextureManager.h"
#include "Scene.h"
#include "Macros.h"

using namespace std;
using namespace glm;


namespace SausageGameLoop {
  Controller* controller;

  void Init() {
    // create systems
    SystemsManager::GetInstance()->InitSystems();
    CheckGLError();
  }


  int run(Scene* scene)
  {
    auto systems_manager = SystemsManager::GetInstance();
    scene->Init();
    systems_manager->async_manager->Run();
    while (!GameSettings::quit) {
      systems_manager->PreUpdate();

      IF_PROFILE_ENABLED(auto proft1 = chrono::steady_clock::now(););
      scene->PrepareDraws();
      IF_PROFILE_ENABLED(ProfTime::prepare_draws_ns = chrono::steady_clock::now() - proft1;);
      SDL_Event e;
      while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        systems_manager->controller->ProcessEvent(&e);
      }
      systems_manager->Render();
      systems_manager->Update();
      IF_PROFILE_ENABLED(ProfTime::total_frame_ns = chrono::steady_clock::now() - proft1;);
      CheckGLError();
    }
    systems_manager->Clear();
    delete systems_manager;
    return 0;
  }
}
