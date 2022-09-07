#pragma once

#include "sausage.h"
#include "Settings.h"
#include "GLHelpers.h"
#include "BufferSettings.h"

using namespace glm;
using namespace std;

using namespace BufferSettings;

namespace UniformsLocations {
  const int UNIFORMS_LOC = 0;
  const int TEXTURE_LOC = 1;
  const int LIGHTS_UNIFORM_LOC = 2;
  const int FONT_UNIFORMS_LOC = 3;
  const int FONT_TEXTURE_UNIFORM_LOC = 4;
  const int UI_UNIFORM_LOC = 5;
  const int CONTROLLER_UNIFORM_LOC = 6;
}

struct TextureBlend {
  float blend_weight; // 4 bytes
  // monotonically increasing, Sausage managed. see Texture->id.
  unsigned int texture_id; // 4 bytes for 64 bit build (who uses 32 anyway)
};

struct BlendTextures {
  unsigned int num_textures; // 4 bytes for 64 bit build (who uses 32 anyway)
  TextureBlend textures[MAX_BLEND_TEXTUERS];
};

// some GPU structs resides in other headers, i.e. Light.h
struct MeshUniformData {
  mat4 bones_transforms[MAX_BONES];
  mat4 transforms[MAX_BASE_MESHES];
  BlendTextures blend_textures[MAX_BASE_AND_INSTANCED_MESHES];
  unsigned int transform_offset[MAX_BASE_AND_INSTANCED_MESHES];
  char pad[GetPadTo16BytesNumOfBytes(
    sizeof(unsigned int) * MAX_BASE_AND_INSTANCED_MESHES
    + sizeof(BlendTextures))];
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
