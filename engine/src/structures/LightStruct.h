#pragma once

#include "sausage.h"
using namespace glm;
using namespace std;
using namespace BufferSettings;

namespace LightType {
  enum LightType { Point, Directional, Spot };
}

/**
@see './docs/Std140_Std430_GL_alignment.md' for alignment explanation
*/

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
  float pad[2]; // OpenGL padding. Aligned to largest struct elem == vec4 (std430)
};

struct Lights {
  int num_lights;
  Light lights[MAX_LIGHTS];
};

// GPU END ///////////////////////////////////////////////////
