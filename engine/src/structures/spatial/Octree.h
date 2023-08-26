#pragma once

#include "sausage.h"
#include "Frustum.h"
#include "Spatial.h"
#include "BoundingVolume.h"
#include "ThreadSafeVector.h"

/**
 * @brief octree of bounding boxes, acceleration of frustum culling and other spatial checks
 *
 * to not have thread safe vectors, allow only one thread to traverse tree at a time.
 * other threads only setting `is_dirty` flag on Spatial, marking its to be reinserted to Octree
 * during frustum or raytest
 *
 * TODO:
 *  one mutex access, on insertion?
 *  currently just keep same thread insert/iterate tree
*/
class Octree {
public:

  Octree* parent;

  BoundingBox* bv;
  //ThreadSafeVector<Spatial*> objects;
  vector<Spatial*> objects;
  bool is_leaf;

  mutex insert_mtx;
  // clockwise start from down south west
  vector<Octree*> children;

  Octree(BoundingBox* bv, unsigned long num_levels) : Octree(nullptr, bv, num_levels) {}

  Octree(Octree* parent, BoundingBox* bv, unsigned long num_levels) : parent{ parent }, bv { bv } {
    is_leaf = num_levels <= 1;
    Split(num_levels);
  };

  void FrustrumCull(
    Frustrum* frustum,
    vec3& camera_pos,
    vector<Spatial*>& out_inside_frustum
  ) {
    ProcessNode(frustum, camera_pos, this, out_inside_frustum);
  }

  // TODO: iterate until smallest fitting AABB found
  //       insert once into corresponding Octree level, not in parent one
  //
  // how to handle intersection?
  //  1.
  //   asign to both
  //   have flag  is_culled to Spatial
  //   allow double tenance
  //  or
  //  2.
  //   count number of containers
  //   if > 2 - just assign to parrent
  //   - start from topmost
  //   - if FULLY inside: check all children. until PARTIALLY inside.
  bool Insert(vector<Spatial*>& in_objects) {
    for (auto object : in_objects) {
      Insert(object);
    }
  }
  /**
   * @return if object is fully inside node -> inserted.
   *         return value used to check if containing parent node can be narrowed to any children.
   *         to stop recurse when none of children returned `true`
   *
   TODO: problem - on rotation/scale, objects AABB changes. need to reinsert into the tree.
  */
  bool Insert(Spatial* object) {
    return Insert(this, object);
  }

  bool Insert(Octree* node, Spatial* object) {
    if (!node->bv->IsInside(object->bv)) {
      return false;
    }
    for (auto child : node->children) {
      if (Insert(child, object)) {
        return true;
      }
    }
    objects.push_back(object);
    object->SetParentIndex(objects.size() - 1);
    return true;
  }

  void ProcessNode(Frustrum* frustum,
    vec3& camera_pos,
    Octree* node,
    vector<Spatial*>& out_inside_frustum
  ) {
    if (node->bv->IsCulledByDistance(camera_pos)) {
      return;
    }
    // TODO: fix culling
    //if (node->bv->IsCulled(frustum)) {
    //  return;
    //}
    // TODO: maybe narrower phase, frustum per object.
    out_inside_frustum.insert(out_inside_frustum.end(),
      objects.begin(), objects.end());
    for (auto child : node->children) {
      ProcessNode(frustum, camera_pos, child, out_inside_frustum);
    }
  }

  void Split(int num_levels) {
    if (is_leaf) {
      return;
    }
    // Bounds.Extens.x is the same as y and z, all nodes are square
    // We want each child node to be half as big as this node
    vec3 child_extents = bv->half_extents / 2.0f;

    children.resize(8, nullptr);

    // order matters
    // clockwise start from down south west
    vector<vec3> childs_centers = {
      bv->center + vec3(-child_extents.x, -child_extents.y, -child_extents.z),
      bv->center + vec3(-child_extents.x, -child_extents.y, child_extents.z),
      bv->center + vec3(child_extents.x, -child_extents.y, child_extents.z),
      bv->center + vec3(child_extents.x, -child_extents.y, -child_extents.z),

      bv->center + vec3(-child_extents.x, child_extents.y, -child_extents.z),
      bv->center + vec3(-child_extents.x, child_extents.y, child_extents.z),
      bv->center + vec3(child_extents.x, child_extents.y, child_extents.z),
      bv->center + vec3(child_extents.x, child_extents.y, -child_extents.z),
    };

    for (int i = 0; i < 8; i++) {
      auto center = childs_centers[i];
      auto max_AABB = center + child_extents;
      auto min_AABB = center - child_extents;
      auto half_extents = max_AABB - min_AABB;
      half_extents[0] *= 0.5;
      half_extents[1] *= 0.5;
      half_extents[2] *= 0.5;

      auto child_bv = new BoundingBox(childs_centers[i], half_extents, true);
      auto child = new Octree(this, child_bv, num_levels - 1);
      children[i] = child;
    }
  }
};
