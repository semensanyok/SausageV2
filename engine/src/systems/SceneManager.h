#pragma once

#include "sausage.h"
#include "Scene.h"

using namespace std;

class SceneManager {
public:
  vector<Scene*> scenes;

  static SceneManager* GetInstance() {
    static SceneManager* instance = new SceneManager();
    return instance;
  };

  inline void PreDraw() {
    for (auto& scene : scenes) {
      scene->PrepareFrameDraws();
    }
  }

  inline void AddScene(Scene* scene) {
    scenes.push_back(scene);
  }

private:
  SceneManager() {};
  ~SceneManager() {};
};
