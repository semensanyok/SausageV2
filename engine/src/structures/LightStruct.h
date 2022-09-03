#pragma once

#include "sausage.h"
using namespace glm;
using namespace std;

namespace LightType {
  enum LightType { Point, Directional, Spot };
}

// GPU START ///////////////////////////////////////////////////
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
  float padding[2] = { 0, 0 };  // OpenGL padding. Aligned to vec4
};

struct Lights {
  int num_lights;
  float padding[3] = { 0, 0, 0 }; // OpenGL padding. Aligned to vec4
  Light lights[];
};

// GPU END ///////////////////////////////////////////////////
