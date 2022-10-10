#pragma once
#include <TerrainManager.h>
#include <GameLoop.h>

class TerrainTest {
  class TestScene : public Scene {
  public:
    void Init() {
      auto tm = SystemsManager::GetInstance()->terrain_manager;
      tm->CreateTerrain();
    };
    void PrepareDraws() {
    };
  };
public:
  static void run() {
    SausageGameLoop::Init();
    SausageGameLoop::run(new TestScene());
  };
};
