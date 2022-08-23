#include "game.h"

int SDL_main(int argc, char** argv)
{
  SausageGameLoop::Init();
  auto scene = new Scene1();
  return SausageGameLoop::run(scene);
}
