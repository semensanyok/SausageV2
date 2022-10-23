#pragma once

#include "OverlayStruct.h"

namespace BufferSettings {
  const unsigned long MAX_VERTEX = 1000000;
  const unsigned long MAX_INDEX = 10000000;
  const unsigned long MAX_BASE_MESHES = 4000;
  // command buffer is shared among all drawcalls
  const unsigned long MAX_COMMAND = MAX_BASE_MESHES + MAX_UI_COMMANDS + MAX_3D_OVERLAY_COMMANDS;
  const unsigned long MAX_BONES = 100000;
  const unsigned long MAX_MESHES_INSTANCES = MAX_BASE_MESHES * 10;
  const unsigned long MAX_TEXTURE = 1000;
  const unsigned long MAX_BLEND_TEXTURES = 16;
  const unsigned long MAX_LIGHTS = 1000; // TODO: figure out correct size
};
