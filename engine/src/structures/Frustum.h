#pragma once

#include "sausage.h"

using namespace std;
using namespace glm;

struct Plane {
  // distance to closest point on the plane
  float distance;
  vec3 normal;
};

struct Frustum {
  Plane near;
  Plane far;
  Plane down;
  Plane up;
  Plane left;
  Plane right;
};

namespace SausageDebug {
  inline Frustum* GetCentered45DownFrustum() {
    return new Frustum{
        {56.7659798, {0 , -0.707107, -0.707107} },
        {977.861145, {-0 , 0.707107, 0.707107} },
        {56.8072853, {-0.433013, 0.435596, -0.789149} },
        {56.8072853, {-0.433013, -0.789149, 0.435596} },
        {56.8072853, {-0.866025, 0.482963, -0.129409} },
        {56.8072853, {-0.866025, -0.482963, 0.129409} }
    };
  }
}
