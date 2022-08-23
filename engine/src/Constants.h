#pragma once

#include <string>

const std::string EMPTY_STRING = {};

// bullet supports 32 bit masks. create new dynamics world if not enough.
namespace SausageCollisionMasks {
  const int MESH_GROUP_0 = 1;
  const int CLICKABLE_GROUP_0 = 1 << 1;

  const int ALL = 0b11111111111111111111111111111111;
};
