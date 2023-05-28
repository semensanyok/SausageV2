#pragma once

#include "sausage.h"
#include "BoundingVolume.h"
#include "Octree.h"

class Scene {
  Octree* scene_tree;
public:
  Scene(vec3 world_extents, unsigned int octree_num_levels) {
    auto center = vec3(0, 0, 0);
    scene_tree = new Octree(new BoundingBox(center, world_extents, false), octree_num_levels);
  }
  virtual void Init() {};
  virtual void PrepareFrameDraws() {};
};
