#pragma once

#include "BufferSettings.h"
#include "GPUStructs.h"
#include "LightStruct.h"
#include "OverlayStruct.h"
#include "Settings.h"
#include "Texture.h"
#include "Vertex.h"
#include "sausage.h"

using namespace glm;
using namespace std;

using namespace BufferSettings;

/**
@see './docs/Std140_Std430_GL_alignment.md' for alignment explanation
*/

namespace UniformsLocations {
constexpr int MESH_UNIFORMS_LOC = 0;
constexpr int TEXTURE_LOC = 1;
constexpr int LIGHTS_UNIFORM_LOC = 2;
constexpr int FONT_UNIFORMS_LOC = 3;
constexpr int UI_UNIFORM_LOC = 4;
constexpr int CONTROLLER_UNIFORM_LOC = 5;
constexpr int BLEND_TEXTURES_BY_MESH_ID_LOC = 6;
constexpr int MESH_STATIC_UNIFORMS_LOC = 7;
constexpr int MESH_TERRAIN_UNIFORMS_LOC = 8;
}; // namespace UniformsLocations

#define BLEND_TEXTURES_ALIGNED_TO_16_BYTES(CAPACITY)                           \
  BlendTextures blend_textures[CAPACITY];                                      \
  float pad[3];

// some GPU structs resides in other headers, i.e. Light.h
struct UniformDataMesh {
  mat4 bones_transforms[MAX_BONES];      // aligned to vec4 == 16 bytes
  mat4 transforms[MAX_MESHES_INSTANCES]; // aligned to vec4 == 16 bytes
  BLEND_TEXTURES_ALIGNED_TO_16_BYTES(MAX_MESHES_INSTANCES);
  unsigned int uniform_offset[MAX_MESHES_INSTANCES]; // alignment 4 bytes
  // no padding needed, topmost structure
};

struct UniformDataMeshStatic {
  mat4 transforms[MAX_MESHES_STATIC_INSTANCES]; // aligned to vec4 == 16 bytes
  BLEND_TEXTURES_ALIGNED_TO_16_BYTES(MAX_MESHES_STATIC_INSTANCES);
  unsigned int uniform_offset[MAX_MESHES_STATIC_INSTANCES]; // alignment 4 bytes
  // no padding needed, topmost structure
};

struct UniformDataMeshTerrain {
  mat4 transforms[MAX_MESHES_TERRAIN]; // aligned to vec4 == 16 bytes
  BLEND_TEXTURES_ALIGNED_TO_16_BYTES(TERRAIN_MAX_TEXTURES);
  unsigned int uniform_offset[MAX_MESHES_TERRAIN]; // alignment 4 bytes
  // no padding needed, topmost structure
};

struct UniformDataOverlay3D {
  mat4 transforms[MAX_3D_OVERLAY_INSTANCES];
  BLEND_TEXTURES_ALIGNED_TO_16_BYTES(MAX_3D_OVERLAY_INSTANCES);
  unsigned int uniform_offset[MAX_3D_OVERLAY_COMMANDS];
};

struct UniformDataUI {
  ivec4 min_max_x_y[MAX_UI_INSTANCES]; // aligned to vec4 == 16 bytes
  // TODO: calc offset somehow??? can help - OpenGLHelpers.h -> size_t
  // GetPadCharsNumToLargestElement(int num, ...);
  vec2 transforms[MAX_UI_INSTANCES]; // aligned to vec2 == 8 bytes
  float pad1[2];
  unsigned int texture_id_by_instance_id[MAX_UI_INSTANCES]; // alignment 4 bytes
  float pad2[3];
  unsigned int uniform_offset[MAX_UI_INSTANCES];
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
  float pad[3]; // because Light class is aligned to vec4, all structure members
                // offsets aligned to vec4
  Light lights[MAX_LIGHTS]; // aligned to vec4
};

namespace BufferSizes {
constexpr unsigned long VERTEX_STORAGE_SIZE = MAX_VERTEX * sizeof(Vertex);
constexpr unsigned long VERTEX_STATIC_STORAGE_SIZE =
    MAX_VERTEX_STATIC * sizeof(VertexStatic);
constexpr unsigned long VERTEX_UI_STORAGE_SIZE =
    MAX_VERTEX_UI * sizeof(VertexUI);

constexpr unsigned long INDEX_STORAGE_SIZE = MAX_INDEX * sizeof(unsigned int);
constexpr unsigned long INDEX_STATIC_STORAGE_SIZE =
    MAX_INDEX_STATIC * sizeof(unsigned int);
constexpr unsigned long INDEX_UI_STORAGE_SIZE =
    MAX_INDEX_UI * sizeof(unsigned int);

constexpr unsigned long LIGHT_STORAGE_SIZE = MAX_LIGHTS * sizeof(Light);
///////////
// UNIFORMS
///////////
constexpr unsigned long MESH_UNIFORMS_STORAGE_SIZE = sizeof(UniformDataMesh);
constexpr unsigned long MESH_STATIC_UNIFORMS_STORAGE_SIZE =
    sizeof(UniformDataMeshStatic);
constexpr unsigned long MESH_TERRAIN_UNIFORMS_STORAGE_SIZE =
    sizeof(UniformDataMeshTerrain);
constexpr unsigned long TRANSFORM_OFFSET_STORAGE_SIZE =
    MAX_MESHES_INSTANCES * sizeof(unsigned int);
constexpr unsigned long TEXTURE_HANDLE_BY_TEXTURE_ID_STORAGE_SIZE =
    MAX_TEXTURE * sizeof(GLuint64);

constexpr unsigned long UNIFORM_OVERLAY_3D_STORAGE_SIZE =
    sizeof(UniformDataOverlay3D);
constexpr unsigned long UNIFORM_UI_STORAGE_SIZE = sizeof(UniformDataUI);
constexpr unsigned long UNIFORM_CONTROLLER_SIZE = sizeof(ControllerUniformData);

constexpr unsigned long VERTEX_OUTLINE_STORAGE_SIZE =
    MAX_VERTEX_OUTLINE * sizeof(VertexOutline);
constexpr unsigned long INDEX_OUTLINE_STORAGE_SIZE =
    MAX_INDEX_OUTLINE * sizeof(unsigned int);
}; // namespace BufferSizes
