#pragma once

#include "sausage.h"
#include "Settings.h"
#include "BufferSettings.h"
#include "Texture.h"
#include "LightStruct.h"
#include "GPUStructs.h"
#include "Vertex.h"
#include "OverlayStruct.h"

using namespace glm;
using namespace std;

using namespace BufferSettings;

/**
@see './docs/Std140_Std430_GL_alignment.md' for alignment explanation
*/

namespace UniformsLocations {
  const int UNIFORMS_LOC = 0;
  const int TEXTURE_LOC = 1;
  const int LIGHTS_UNIFORM_LOC = 2;
  const int FONT_UNIFORMS_LOC = 3;
  const int UI_UNIFORM_LOC = 4;
  const int CONTROLLER_UNIFORM_LOC = 5;
  const int BLEND_TEXTURES_BY_MESH_ID_LOC = 6;
};

class BufferInstanceOffset {
public:
  // - used as offset to buffer uniform arrays: transform, texture, ...?
  virtual unsigned long GetInstanceOffset() = 0;
};

//struct BlendTexturesMeshUniform {
//  BlendTextures blend_textures[MAX_MESHES_INSTANCES]; // alignment 4 bytes
//};

// TODO: maybe??? extract transforms[MAX_MESHES_INSTANCES] to separate SSBO
//       because, for example, overlay elements are using mat4 transform
//       so presumably they can use same instance_offset used for mesh draw calls
// some GPU structs resides in other headers, i.e. Light.h
struct UniformDataMesh {
  mat4 bones_transforms[MAX_BONES]; // aligned to vec4 == 16 bytes
  mat4 transforms[MAX_MESHES_INSTANCES]; // aligned to vec4 == 16 bytes
  BlendTextures blend_textures[MAX_MESHES_INSTANCES]; // alignment 4 bytes
  float pad[3];

  unsigned int base_instance_offset[MAX_BASE_MESHES]; // alignment 4 bytes
  // no padding needed, topmost structure
};

struct UniformDataOverlay3D {
  mat4 transforms[MAX_3D_OVERLAY_INSTANCES];
  unsigned int base_instance_offset[MAX_3D_OVERLAY_COMMANDS];
};

// TODO: figure eout correct offsets
struct UniformDataUI {
  ivec4 min_max_x_y[MAX_UI_INSTANCES]; // aligned to vec4 == 16 bytes
  // TODO: calc offset somehow??? can help - OpenGLHelpers.h -> size_t GetPadCharsNumToLargestElement(int num, ...);
  vec2 transforms[MAX_UI_INSTANCES]; // aligned to vec2 == 8 bytes
  unsigned int texture_id_by_instance_id[MAX_UI_INSTANCES]; // alignment 4 bytes
  unsigned int base_instance_offset[MAX_UI_COMMANDS];
  // no padding needed, topmost structure
};

struct ControllerUniformData {
  int mouse_x;
  int mouse_y;
  int is_pressed;
  int is_click;
};

struct LightsUniform {
  int num_lights;
  float pad[3]; // because Light class is aligned to vec4, all structure members offsets aligned to vec4
  Light lights[MAX_LIGHTS]; // aligned to vec4
};

namespace BufferSizes {
  const unsigned long VERTEX_STORAGE_SIZE = MAX_VERTEX * sizeof(Vertex);
  const unsigned long INDEX_STORAGE_SIZE = MAX_INDEX * sizeof(unsigned int);
  const unsigned long COMMAND_STORAGE_SIZE =
    MAX_COMMAND * sizeof(DrawElementsIndirectCommand);
  const unsigned long LIGHT_STORAGE_SIZE = MAX_LIGHTS * sizeof(Light);
  ///////////
  // UNIFORMS
  ///////////
  const unsigned long MESH_UNIFORMS_STORAGE_SIZE = sizeof(UniformDataMesh);
  //const unsigned long BLEND_TEXTURES_BY_MESH_ID_SIZE = sizeof(BlendTexturesMeshUniform);
  const unsigned long TRANSFORM_OFFSET_STORAGE_SIZE =
    MAX_MESHES_INSTANCES * sizeof(unsigned int);
  const unsigned long TEXTURE_HANDLE_BY_TEXTURE_ID_STORAGE_SIZE = MAX_TEXTURE * sizeof(GLuint64);

  const unsigned long UNIFORM_OVERLAY_3D_STORAGE_SIZE = sizeof(UniformDataOverlay3D);
  const unsigned long UNIFORM_UI_STORAGE_SIZE = sizeof(UniformDataUI);
  const unsigned long UNIFORM_CONTROLLER_SIZE = sizeof(ControllerUniformData);
};
