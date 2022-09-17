#pragma once

namespace BufferSettings {
  const unsigned long MAX_VERTEX = 1000000;
  const unsigned long MAX_INDEX = 10000000;
  const unsigned long MAX_COMMAND = 1000;
  const unsigned long MAX_BASE_MESHES = 4000;
  const unsigned long MAX_BONES = 100000;
  const unsigned long MAX_BASE_AND_INSTANCED_MESHES = MAX_BASE_MESHES * 10;
  const unsigned long MAX_TEXTURE = 1000;
  const unsigned long MAX_BLEND_TEXTURES = 16;
  const unsigned long MAX_LIGHTS = 1000;
  //FONT
  const unsigned long TEXTURES_SINGLE_FONT = 128;
  const unsigned long MAX_3D_OVERLAY_TRANSFORM = 4000;
  const unsigned long MAX_3D_OVERLAY_TRANSFORM_OFFSET = MAX_3D_OVERLAY_TRANSFORM * 10;
  const unsigned long MAX_FONT_TEXTURES = 1 * TEXTURES_SINGLE_FONT;
  const unsigned long MAX_UI_UNIFORM_TRANSFORM = 400;
  const unsigned long MAX_UI_UNIFORM_OFFSET = MAX_UI_UNIFORM_TRANSFORM * 10;

  namespace Margins {
    const float MESH_DATA_VERTEX_PART = 0.65;
    const float MESH_DATA_INDEX_PART = 0.65;
    // TODO: set to 0 in Release
    const float DEBUG_PHYS_VERTEX_PART = 0.20;
    const float DEBUG_PHYS_INDEX_PART = 0.20;
    const float FONT_VERTEX_PART = 0.03;
    const float FONT_INDEX_PART = 0.03;
    const float OVERLAY_VERTEX_PART = 0.03;
    const float OVERLAY_INDEX_PART = 0.03;
  }
};
