#pragma once
#include <TerrainManager.h>
#include <GameLoop.h>

class TerrainTest {
  class TestScene : public Scene {
  public:
    void Init() {
      auto tm = SystemsManager::GetInstance()->terrain_manager;
      auto chunk = tm->CreateChunk({ 0,0,0 }, 0, 0, 10, 10);
      tm->BufferTerrain(chunk);
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
