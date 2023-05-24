#pragma once

#include "sausage.h"
#include "BoundingVolume.h"
#include "Octree.h"

class Scene {
  Octree* scene_tree;
public:
  Scene(vec3 world_extents) : scene_tree{new Octree(nullptr, new BoundingBox(), 3)}
  virtual void Init() {};
  virtual void PrepareFrameDraws() {};
};
