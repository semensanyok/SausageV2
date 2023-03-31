#pragma once

#include "sausage.h"

using namespace glm;

class PhysicsData {
public:
  vec3 max_AABB;
  vec3 min_AABB;
  float mass;
  int collision_group;
  int collides_with_groups;

  PhysicsData(vec3 min_AABB, vec3 max_AABB)
    : min_AABB{ min_AABB },
    max_AABB{ max_AABB },
    mass{ 0.0 },
    collision_group{ 0 },
    collides_with_groups{ 0 } {}
  PhysicsData(vec3 min_AABB, vec3 max_AABB,
    float mass,
    int collision_group,
    int collides_with_groups)
    : min_AABB{ min_AABB }, max_AABB{ max_AABB }, mass{ mass },
    collision_group{ collision_group },
    collides_with_groups{ collides_with_groups } {}
};

class PhysicsTransformUpdate : public SausageUserPointer {
public:
  virtual mat4& GetOutMatrix() = 0;
  virtual void OnTransformUpdate() = 0;
};
