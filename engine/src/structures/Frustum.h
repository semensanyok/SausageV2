#pragma once

#include "sausage.h"

using namespace std;
using namespace glm;

struct Plane {
  // distance to closest point on the plane
  float distance;
  vec3 normal;
};

struct Frustrum {
  Plane near;
  Plane far;
  Plane down;
  Plane up;
  Plane left;
  Plane right;
};
