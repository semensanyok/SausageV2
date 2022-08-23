#pragma once
#include <TerrainManager.h>
#include <GameLoop.h>

class TerrainTest {
  class TestScene : public Scene {
  public:
    void Init() {
      //string p = "height1.png";
      //string p = "wetlands10.png";
      string p = "document1.png";
      auto ter = new TerrainManager();
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
