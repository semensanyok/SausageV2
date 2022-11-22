#pragma once

#include "OverlayStruct.h"

namespace BufferSettings {
  const unsigned long MAX_VERTEX = 1000000;
  const unsigned long MAX_INDEX = 2 * MAX_VERTEX;
  const unsigned long MAX_BASE_MESHES = 400;
  // command buffer is shared among all drawcalls
  //const unsigned long MAX_COMMAND = MAX_BASE_MESHES + MAX_UI_COMMANDS + MAX_3D_OVERLAY_COMMANDS;
  const unsigned long MAX_BONES = 100;
  const unsigned long MAX_MESHES_INSTANCES = MAX_BASE_MESHES * 10;
  const unsigned long MAX_TEXTURE = 100;
  const unsigned long MAX_BLEND_TEXTURES = 4;
  const unsigned long MAX_LIGHTS = 100; // TODO: figure out correct size
};
