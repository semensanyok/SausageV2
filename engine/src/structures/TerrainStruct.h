#pragma once

#include "sausage.h"
#include "Settings.h"
#include "MeshDataStruct.h"

using namespace glm;

/*

inherits BufferInstanceOffset to buffer texture per TerrainChunk.
(which is not buffered for MeshDataTerrain. maybe it could also support 1 blend for all non-tiled terrain)

.....
.....
..*..
.....
.....

*/

struct TerrainBlendTextures : public BufferInstanceOffset {
public:
  // index into blend information array.
  long uniform_id;
  BlendTextures textures;
  TerrainBlendTextures(long uniform_id) : uniform_id{ uniform_id } {};

  unsigned long GetInstanceOffset() override {
    return uniform_id;
  }
  bool IsInstanceOffsetAllocated() override {
    return uniform_id >= 0;
  }
};

struct TileVerticesIndices {
  unsigned int nw;
  unsigned int ne;
  unsigned int sw;
  unsigned int se;
};

class TerrainChunk {
  public:
  MeshDataTerrain* mesh;
  // maybe no need to store it for mesh with non changing height.
  // however, altering it in runtime could be a boon to gameplay
  vector<float> heightmap;
  //vector<float> moistmap;
  //vector<float> temptmap;

  vec3 pos;
  // number of TerrainTile patches by axes.
  int size_x, size_y;
  float spacing;

  /*
  * get 4 vertices indices to buffer uniform_id for a tile
  */
  TileVerticesIndices GetTileIndices(
    unsigned int tile_start_x,
    unsigned int tile_start_y) {
    assert(tile_start_x < size_x);
    assert(tile_start_x % 2 == 0);
    assert(tile_start_y < size_y);
    assert(tile_start_y % 2 == 0);
    auto x = tile_start_x;
    auto y = tile_start_y;

    int current_row_shift = y * size_x;
    int next_row_shift = (y + 1) * size_x;

    unsigned int ind_nw = x + next_row_shift;
    unsigned int ind_ne = (x + 1) + next_row_shift;
    unsigned int ind_sw = x + current_row_shift;
    unsigned int ind_se = (x + 1) + current_row_shift;
    return { ind_nw, ind_ne, ind_sw, ind_se };
  };

  TerrainChunk(int size_x, int size_y, float spacing, vec3 pos) :
    pos{ pos },
    size_x{ size_x }, size_y{ size_y },
    spacing{ spacing },
    heightmap{ vector<float>(size_x * size_y) } {}
};
