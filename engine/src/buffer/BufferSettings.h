#pragma once

namespace BufferSettings {
  const unsigned long MAX_VERTEX = 1000000;
  const unsigned long MAX_INDEX = 10000000;
  const unsigned long MAX_BASE_MESHES = 4000;
  // also need command space for ui front/back, debug drawer, etc.
  const unsigned long MAX_COMMAND = MAX_BASE_MESHES * 5;
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
};
