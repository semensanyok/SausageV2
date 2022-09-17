#pragma once

#include "sausage.h"
#include "Settings.h"
#include "BufferSettings.h"
#include "Texture.h"
#include "LightStruct.h"

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
  const int FONT_TEXTURE_UNIFORM_LOC = 4;
  const int UI_UNIFORM_LOC = 5;
  const int CONTROLLER_UNIFORM_LOC = 6;
  const int BLEND_TEXTURES_BY_MESH_ID_LOC = 7;
};

struct BlendTexturesByMeshIdUniform {
  BlendTextures blend_textures[MAX_BASE_AND_INSTANCED_MESHES]; // alignment 4 bytes
};

// TODO (HIGHEST PRIORITY):
//  when 2 mat4 transforms at the start - we get correct transform, but cant get correct texture id
//  when transforms matrices at MeshUniformData bottom - we dont get correct transforms
//  FIX LAYOUT OR SPLIT TO DIFFERENT UNIFORM TO VERIFY OFFSET ISSUE

// some GPU structs resides in other headers, i.e. Light.h
struct MeshUniform {
  mat4 bones_transforms[MAX_BONES]; // aligned to vec4 == 16 bytes
  mat4 transforms[MAX_BASE_AND_INSTANCED_MESHES]; // aligned to vec4 == 16 bytes
  unsigned int transform_offset[MAX_BASE_MESHES]; // alignment 4 bytes
  // BlendTextures blend_textures[MAX_BASE_AND_INSTANCED_MESHES]; // alignment 4 bytes 
  // no padding needed, topmost structure
};

//struct MeshUniformData {
//  mat4 bones_transforms[MAX_BONES]; // aligned to vec4 == 16 bytes
//  mat4 transforms[MAX_BASE_AND_INSTANCED_MESHES]; // aligned to vec4 == 16 bytes
//  BlendTextures blend_textures[MAX_BASE_AND_INSTANCED_MESHES]; // alignment 4 bytes
//  unsigned int transform_offset[MAX_BASE_MESHES]; // alignment 4 bytes
//  // no padding needed, topmost structure
//};

struct UniformData3DOverlay {
  mat4 transforms[MAX_3D_OVERLAY_TRANSFORM];
};

struct UniformDataUI {
  ivec4 min_max_x_y[MAX_UI_UNIFORM_TRANSFORM];
  vec2 transforms[MAX_UI_UNIFORM_TRANSFORM];
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
  Light lights[MAX_LIGHTS];
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
  const unsigned long MESH_UNIFORMS_STORAGE_SIZE = sizeof(MeshUniform);
  const unsigned long BLEND_TEXTURES_BY_MESH_ID_SIZE = sizeof(BlendTexturesByMeshIdUniform);
  const unsigned long TRANSFORM_OFFSET_STORAGE_SIZE =
    MAX_BASE_AND_INSTANCED_MESHES * sizeof(unsigned int);
  const unsigned long TEXTURE_HANDLE_BY_TEXTURE_ID_STORAGE_SIZE = MAX_TEXTURE * sizeof(GLuint64);

  // FONT buffers
  const unsigned long FONT_TEXTURE_STORAGE_SIZE =
    MAX_FONT_TEXTURES * sizeof(GLuint64);
  const unsigned long UNIFORMS_3D_OVERLAY_STORAGE_SIZE = sizeof(UniformData3DOverlay);
  const unsigned long TRANSFORM_3D_OVERLAY_OFFSET_STORAGE_SIZE =
    MAX_3D_OVERLAY_TRANSFORM_OFFSET * sizeof(unsigned int);
  const unsigned long UNIFORMS_UI_STORAGE_SIZE = sizeof(UniformDataUI);
  const unsigned long TRANSFORM_OFFSET_UI_STORAGE_SIZE =
    MAX_UI_UNIFORM_OFFSET * sizeof(unsigned int);
  // parts of buffer;
};
