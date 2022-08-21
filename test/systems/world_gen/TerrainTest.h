
#include <TerrainManager.h>
#include <GameLoop.h>

class TerrainTest {
public:
  static void run() {
    SausageGameLoop::Init();
    //string p = "height1.png";
    //string p = "wetlands10.png";
    string p = "document1.png";
    auto ter = new TerrainManager();
    ter->CreateTerrain();


  };
};
