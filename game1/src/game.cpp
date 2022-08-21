#include "game.h"

int SDL_main(int argc, char** argv)
{
  SausageGameLoop::Init();
  auto scene = new Scene1(SausageGameLoop::systems_manager);
  scene->Init();
  return SausageGameLoop::run(scene);
}
