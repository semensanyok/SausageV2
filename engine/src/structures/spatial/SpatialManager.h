#pragma once

#include "sausage.h"
#include "Spatial.h"

using namespace std;

class SpatialManager {
  vector<Spatial*> world_octree;

  void Init() {

  };

  /**
   * @param interval cube len
   * @param size_x 
   * @param size_y 
   * @param size_z 
  */
  void BuildWorldTree(
    unsigned long interval,
    unsigned long size_x,
    unsigned long size_y,
    unsigned long size_z
  ) {
    world_octree.reserve(size_x * size_y * size_z)
    for 
  }

  Spatial* GetOctreeCubeAt(vec3& pos) {

    // check 6 faces
    // if on negative side - negative, shift to neighbour.
    if ()
  }
};
