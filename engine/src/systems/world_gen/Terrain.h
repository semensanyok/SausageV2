#pragma once

#include "sausage.h"
#include "AssetUtils.h"


class Terrain
{
public:
  Terrain(string& filename) {
    GetTerrainTexturePath(filename);
  };
  ~Terrain() {};
};
