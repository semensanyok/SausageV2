#pragma once

#include "sausage.h"
using namespace glm;
using namespace std;
using namespace BufferSettings;

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
};

struct Lights {
  int num_lights;
  Light lights[MAX_LIGHTS];
  char pad[GetPadTo16BytesNumOfBytes(sizeof(int) + sizeof(Light) * MAX_LIGHTS)]; // OpenGL padding. Aligned to vec4
};

// GPU END ///////////////////////////////////////////////////
