#pragma once

#include "sausage.h"

using namespace glm;
using namespace std;

namespace LightType {
  enum LightType { Point, Directional, Spot };
}

struct Light {
  vec4 direction;
  vec4 position;
  vec4 color;
  vec4 specular;

  int type;
  float spot_inner_cone_cos;
  float spot_outer_cone_cos;
  float constant_attenuation;

  float linear_attenuation;
  float quadratic_attenuation;
  float pad[2]; // OpenGL padding. Aligned to largest struct elem == vec4 (std430)
};
