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
  const int MESH_UNIFORMS_LOC = 0;
  const int TEXTURE_LOC = 1;
  const int LIGHTS_UNIFORM_LOC = 2;
  const int FONT_UNIFORMS_LOC = 3;
  const int UI_UNIFORM_LOC = 4;
  const int CONTROLLER_UNIFORM_LOC = 5;
  const int BLEND_TEXTURES_BY_MESH_ID_LOC = 6;
  const int MESH_STATIC_UNIFORMS_LOC = 7;
};

class BufferInstanceOffset {

public:
  // used as offset to array, containing offset to buffer uniform arrays: transform, texture, ...
  // to not rebuffer data for instanced meshes (transform matrices) each time slot is reallocated.
  //    e.g. when we have 2 buffered instanced meshes, but then increased instances to 4.
  //    so we must buffer 4 subsequent data, which identifies given instance.
  //    we cannot rebuffer all buffered data to the end of each array.
  //    so, we rebuffer only subsequent indices to intance data arrays.
  virtual unsigned long GetInstanceOffset() = 0;

  virtual bool IsInstanceOffsetAllocated() = 0;
};

#define BLEND_TEXTURES_ALIGNED_TO_16_BYTES(CAPACITY) BlendTextures blend_textures[CAPACITY];\
float pad[3];

// some GPU structs resides in other headers, i.e. Light.h
struct UniformDataMesh {
  mat4 bones_transforms[MAX_BONES]; // aligned to vec4 == 16 bytes
  mat4 transforms[MAX_MESHES_INSTANCES]; // aligned to vec4 == 16 bytes
  BLEND_TEXTURES_ALIGNED_TO_16_BYTES(MAX_MESHES_INSTANCES);
  unsigned int base_instance_offset[MAX_BASE_MESHES]; // alignment 4 bytes
  // no padding needed, topmost structure
};

struct UniformDataMeshStatic {
  mat4 transforms[MAX_MESHES_STATIC_INSTANCES]; // aligned to vec4 == 16 bytes
  BLEND_TEXTURES_ALIGNED_TO_16_BYTES(MAX_MESHES_STATIC_INSTANCES);
  unsigned int base_instance_offset[MAX_BASE_MESHES_STATIC]; // alignment 4 bytes
  // no padding needed, topmost structure
};

struct UniformDataOverlay3D {
  mat4 transforms[MAX_3D_OVERLAY_INSTANCES];
  BLEND_TEXTURES_ALIGNED_TO_16_BYTES(MAX_3D_OVERLAY_INSTANCES);
  unsigned int base_instance_offset[MAX_3D_OVERLAY_COMMANDS];
};

// TODO: figure eout correct offsets
struct UniformDataUI {
  ivec4 min_max_x_y[MAX_UI_INSTANCES]; // aligned to vec4 == 16 bytes
  // TODO: calc offset somehow??? can help - OpenGLHelpers.h -> size_t GetPadCharsNumToLargestElement(int num, ...);
  vec2 transforms[MAX_UI_INSTANCES]; // aligned to vec2 == 8 bytes
  float pad1[2];
  unsigned int texture_id_by_instance_id[MAX_UI_INSTANCES]; // alignment 4 bytes
  float pad2[3];
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
  const unsigned long VERTEX_STATIC_STORAGE_SIZE = MAX_VERTEX_STATIC * sizeof(VertexStatic);
  const unsigned long VERTEX_UI_STORAGE_SIZE = MAX_VERTEX_UI * sizeof(VertexUI);

  const unsigned long INDEX_STORAGE_SIZE = MAX_INDEX * sizeof(unsigned int);
  const unsigned long INDEX_STATIC_STORAGE_SIZE = MAX_INDEX_STATIC * sizeof(unsigned int);
  const unsigned long INDEX_UI_STORAGE_SIZE = MAX_INDEX_UI * sizeof(unsigned int);

  const unsigned long LIGHT_STORAGE_SIZE = MAX_LIGHTS * sizeof(Light);
  ///////////
  // UNIFORMS
  ///////////
  const unsigned long MESH_UNIFORMS_STORAGE_SIZE = sizeof(UniformDataMesh);
  const unsigned long MESH_STATIC_UNIFORMS_STORAGE_SIZE = sizeof(UniformDataMeshStatic);
  const unsigned long TRANSFORM_OFFSET_STORAGE_SIZE =
    MAX_MESHES_INSTANCES * sizeof(unsigned int);
  const unsigned long TEXTURE_HANDLE_BY_TEXTURE_ID_STORAGE_SIZE = MAX_TEXTURE * sizeof(GLuint64);

  const unsigned long UNIFORM_OVERLAY_3D_STORAGE_SIZE = sizeof(UniformDataOverlay3D);
  const unsigned long UNIFORM_UI_STORAGE_SIZE = sizeof(UniformDataUI);
  const unsigned long UNIFORM_CONTROLLER_SIZE = sizeof(ControllerUniformData);

  const unsigned long VERTEX_OUTLINE_STORAGE_SIZE = MAX_VERTEX_OUTLINE * sizeof(VertexOutline);
  const unsigned long INDEX_OUTLINE_STORAGE_SIZE = MAX_INDEX_OUTLINE * sizeof(unsigned int);
};
