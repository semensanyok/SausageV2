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
  BoundingBox* bounding_box;
  float mass;
  int collision_group;
  int collides_with_groups;

  PhysicsData(mat4& transform, vec3 min_AABB, vec3 max_AABB)
    : bounding_box{ new BoundingBox(transform, min_AABB, max_AABB) },
    mass{ 0.0 },
    collision_group{ 0 },
    collides_with_groups{ 0 } {}
  PhysicsData(mat4& transform, vec3 min_AABB, vec3 max_AABB,
    float mass,
    int collision_group,
    int collides_with_groups)
    : bounding_box{ new BoundingBox(transform, min_AABB, max_AABB) },
    mass{ mass },
    collision_group{ collision_group },
    collides_with_groups{ collides_with_groups } {}
};

class PhysicsTransformUpdate : public SausageUserPointer {
public:
  virtual mat4& GetOutMatrix() = 0;
  virtual void OnTransformUpdate() = 0;
};
