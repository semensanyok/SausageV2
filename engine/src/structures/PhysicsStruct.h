#pragma once

#include "sausage.h"
#include "BoundingVolume.h"

using namespace glm;

// bullet supports 32 bit masks. create new dynamics world if not enough.
namespace SausageCollisionMasks {
  const int MESH_GROUP_0 = 1;
  const int CLICKABLE_GROUP_0 = 1 << 1;

  const int ALL = 0b11111111111111111111111111111111;
};

class PhysicsData {
public:
  float mass;
  int collision_group;
  int collides_with_groups;

  PhysicsData() :
    mass{ 0.0 },
    collision_group{ 0 },
    collides_with_groups{ 0 } {}
  PhysicsData(
    float mass,
    int collision_group,
    int collides_with_groups)
    :
    mass{ mass },
    collision_group{ collision_group },
    collides_with_groups{ collides_with_groups } {}
};
