#pragma once

#include "OverlayStruct.h"

namespace BufferSettings {
  const unsigned long MAX_VERTEX = 1000000;
  const unsigned long MAX_INDEX = 2 * MAX_VERTEX;
  const unsigned long MAX_BASE_MESHES = 400;
  const unsigned long MAX_MESHES_INSTANCES = MAX_BASE_MESHES * 10;
  const unsigned long MAX_BONES = 100;

  const unsigned long MAX_VERTEX_STATIC = 1000000;
  const unsigned long MAX_INDEX_STATIC = 2 * MAX_VERTEX_STATIC;
  const unsigned long MAX_BASE_MESHES_STATIC = 40;
  const unsigned long MAX_MESHES_STATIC_INSTANCES = MAX_BASE_MESHES_STATIC * 10;

  const unsigned long MAX_MESHES_TERRAIN = 40;

  const unsigned long MAX_VERTEX_UI = 10000;
  const unsigned long MAX_INDEX_UI = 2 * MAX_VERTEX_UI;

  const unsigned long MAX_TEXTURE = 100;
  const unsigned int MAX_BLEND_TEXTURES = 4;
  const unsigned long MAX_LIGHTS = 100; // TODO: figure out correct size

  const unsigned long MAX_VERTEX_OUTLINE = 1500000;
  const unsigned long MAX_INDEX_OUTLINE = 2 * MAX_VERTEX_OUTLINE;

  // max num of BlendTexture's for terrain chunk's tiles. (number of differently coloured tiles in chunk)
  const unsigned long TERRAIN_MAX_TEXTURES = 1000;
};
