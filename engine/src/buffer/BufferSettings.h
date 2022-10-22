#pragma once

namespace BufferSettings {
  const unsigned long MAX_VERTEX = 1000000;
  const unsigned long MAX_INDEX = 10000000;
  const unsigned long MAX_BASE_MESHES = 4000;
  // also need command space for ui front/back, debug drawer, etc.
  const unsigned long MAX_COMMAND = MAX_BASE_MESHES * 5;
  const unsigned long MAX_BONES = 100000;
  const unsigned long MAX_MESHES_INSTANCES = MAX_BASE_MESHES * 10;
  const unsigned long MAX_TEXTURE = 1000;
  const unsigned long MAX_BLEND_TEXTURES = 16;
  const unsigned long MAX_LIGHTS = 1000;
};
