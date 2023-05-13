#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Camera.h"
#include "Frustum.h"

using namespace std;
using namespace glm;

class BoundingVolume {
public:
  vec3 center;
  BoundingVolume(mat4& transform) :
    center{ transform[3] } {}
  virtual bool IsCulled(Frustrum& frustrum) = 0;
  virtual void Transform(mat4& transform) = 0;
};

class BoundingBox : public BoundingVolume {
public:
  vec3 half_extents;

  BoundingBox(mat4& transform, vec3& min_AABB, vec3& max_AABB) :
    BoundingVolume(transform),
    half_extents{ max_AABB - min_AABB } {
    half_extents[0] *= 0.5;
    half_extents[1] *= 0.5;
    half_extents[2] *= 0.5;

    TransformExtents(transform);
  }

  void Transform(mat4& transform) override {
    center = transform[3];
    TransformExtents(transform);
  }

  void TransformExtents(glm::mat4& transform)
  {
    // rotate/scale AABB
    // linear. f(cx+cy)==cf(x)+cf(y) -> rot(max_AABB) + rot(min_AABB) / 2 == rot((max_AABB + min_AABB / 2))
    half_extents[0] = transform[0][0] * half_extents[0] + transform[1][0] * half_extents[1] + transform[2][0] * half_extents[0];
    half_extents[1] = transform[0][1] * half_extents[0] + transform[1][1] * half_extents[1] + transform[2][1] * half_extents[0];
    half_extents[2] = transform[0][0] * half_extents[0] + transform[1][0] * half_extents[1] + transform[2][0] * half_extents[0];
  }

  // given:
  // 
  //   normal vector:
  // 
  //     n
  // 
  //   point on a plane:
  // 
  //     x
  // 
  //   direction from world origin:
  // 
  //     p
  //
  // 
  // plane equation:
  // 
  //     n * x = -p,
  // 
  // where `p > 0` means distance vector origin to plane cooriented with normal.
  // (pointing to same part of space, divided by hyperplane)
  //
  // 
  // distance `d` of point `c` to plane:
  // 
  //   d = n * c + p,
  // 
  // where `d > 0` means distance vector point to plane cooriented with normal.
  //
  // test AABB on positive side of frustum:
  //   1. project extent onto normal (find lenght of AABB projection onto axis, || plane normal)
  //   2. project AABB center 'c' on 'n', sub 'p'.
  //      
  //   3. find distance from `c` to plane == `d`. 
  //      if 'd' is positive, it lies on plane positive side.
  //      if 'd' is negative - exit.
  // 
  bool IsCulled(Frustrum& frustrum) override {
    // optimal order. first check sides, then depth, since sides cuts off most space
    return IsOnNormalSide(frustrum.left)
      && IsOnNormalSide(frustrum.right)
      && IsOnNormalSide(frustrum.up)
      && IsOnNormalSide(frustrum.down)
      && IsOnNormalSide(frustrum.near)
      && IsOnNormalSide(frustrum.far);
  };

  bool IsOnNormalSide(Plane& plane) {
    // because rotation of normal doesnt change length of AABB diagonal projection
    float r = dot(half_extents, abs(plane.normal));

    // Compute distance of box center from plane
    // if sides are opposite - sign will be reversed
    auto distance_center_to_plane = dot(center, plane.normal) - plane.distance;

    // >= -r means - AABB side touches Frustum side.
    return distance_center_to_plane >= -r;
  }
};
//class BoundingSphere {
//public:
//  virtual bool IsCulled(frustrum) = 0;
//};

//
//class BoundingPoint : public BoundingVolume {
//public:
//  void Transform(mat4& transform) override {
//    center = transform[3];
//  }
//
//  bool IsCulled(Frustrum& frustrum) override {
//    // optimal order. first check sides, then depth, since sides cuts off most space
//    return IsOnNormalSide(frustrum.left)
//      && IsOnNormalSide(frustrum.right)
//      && IsOnNormalSide(frustrum.up)
//      && IsOnNormalSide(frustrum.down)
//      && IsOnNormalSide(frustrum.near)
//      && IsOnNormalSide(frustrum.far);
//  };
//
//  bool IsOnNormalSide(Plane& plane) {
//    auto& plane = frustrum.left;
//    // because rotation of normal doesnt change length of AABB diagonal projection
//    float r = dot(half_extents, abs(plane.normal));
//
//    // Compute distance of box center from plane
//    // if sides are opposite - sign will be reversed
//    auto distance_center_to_plane = dot(center, plane.normal) - plane.distance;
//
//    // >= -r means - AABB side touches Frustum side.
//    bool is_on_normal_side = distance_center_to_plane >= -r;
//  }
//};

