#pragma once

#include "sausage.h"
#include "BoundingVolume.h"

using namespace std;

// TODO: culling, GameSettings::MAX_DRAW_DISTANCE
// 
// hierarchical bounding volume
// used in broad phase culling
class Spatial {
  Spatial* parent;
  // id into parent's 'children' array.
  // how to avoid multiple parents?
  // allow only 1 parent.
  // for composite meshes only topmost parent is frustum checked
  // 'containers' inspected recursively
  // 'containers' are immovable boxes, splitting the world into non intersecting parts
  // 'meshes' can be static or dynamic. dynamic changes parent 'container'
  unsigned int parent_idx;
  map<unsigned int, Spatial*> children;

public:
  BoundingVolume* bv;

  Spatial(BoundingVolume* bv) :
    bv{ bv },
    parent{ nullptr },
    parent_idx{ 0 } {

  }
  Spatial* AddChild(BoundingVolume* node) {
    Spatial* child = new Spatial(node, this, children.size());
    children[child->parent_idx] = child;
    return new Spatial(node, this, children.size());
  }
  /**
   * @brief return number of elements erased. (expected 1)
  */
  size_t RemoveChild(Spatial* node) {
    return children.erase(node->parent_idx);
  }

private:
  Spatial(BoundingVolume* bv, Spatial* parent, unsigned int parent_idx) :
    bv{ bv },
    parent{ parent },
    parent_idx{ parent_idx } {

  }
};
