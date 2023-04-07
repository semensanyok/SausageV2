#pragma once

#include "sausage.h"
#include "Settings.h"
#include "MeshDataStruct.h"

using namespace glm;

/*

.....
.....
..*..
.....
.....

*/

class TerrainChunk {
  public:
  MeshDataStatic* mesh;
  vector<float> heightmap;
  //vector<float> moistmap;
  //vector<float> temptmap;

  vec3 pos;
  // number of TerrainTile patches by axes.
  int sizeX, sizeY;

  TerrainChunk(int sizeX, int sizeY, vec3 pos) :
    pos{ pos },
    sizeX{ sizeX }, sizeY{ sizeY },
    heightmap{ vector<float>(sizeX * sizeY) } {}
};
