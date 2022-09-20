#pragma once
#include <TerrainManager.h>
#include <GameLoop.h>

class TerrainTest {
  class TestScene : public Scene {
  public:
    void Init() {
      auto ter = new TerrainManager(nullptr, nullptr);
      ter->CreateTerrain();
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
