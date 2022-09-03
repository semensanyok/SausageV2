#pragma once

#include "../sausage.h"
#include "../Settings.h"

using namespace glm;
using namespace std;
using namespace BufferSettings;


// some GPU structs resides in other headers, i.e. Light.h
struct MeshUniformData {
  mat4 bones_transforms[MAX_BONES];
  mat4 transforms[MAX_TRANSFORM];
  unsigned int transform_offset[MAX_TRANSFORM_OFFSET];
};

struct UniformData3DOverlay {
  mat4 transforms[MAX_3D_OVERLAY_TRANSFORM];
};

struct UniformDataUI {
  ivec4 min_max_x_y[MAX_UI_UNIFORM_TRANSFORM];
  vec2 transforms[MAX_UI_UNIFORM_TRANSFORM];
};

struct ControllerUniformData {
  int mouse_x;
  int mouse_y;
  int is_pressed;
  int is_click;
};
