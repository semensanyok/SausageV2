#pragma once

#include "sausage.h"
#include "Settings.h"
#include "MeshDataStruct.h"

using namespace glm;

struct TerrainTile;
/*

.....
.....
..*..
.....
.....

*/
struct AdjastentTiles {
  TerrainTile* w;
  TerrainTile* nw;
  TerrainTile* n;
  TerrainTile* ne;
  TerrainTile* e;
  TerrainTile* se;
  TerrainTile* s;
  TerrainTile* sw;
};

////////// simplest event - trigger per pixel, depending on height/moisture/ value
struct TerrainPixelValues {
  float height;
  float moisture;
  float temperature;
};

/**
 parameters used for MeshLoadData creation,
 to determine instanced mesh.
*/
struct TileSizeParameters {
  // size in vertices
  int size_x;
  int size_y;
};
// hash function for unordered map
template<> struct std::hash<TileSizeParameters> {
  size_t operator()(TileSizeParameters const& t) const {
    return t.size_x + t.size_y;
  }
};
// eq for hashmap/hashset
inline bool operator==(const TileSizeParameters& lhs, const TileSizeParameters& rhs) {
  return lhs.size_x == rhs.size_x && lhs.size_y == rhs.size_y;
}
// compare for map/set
inline bool operator<(const TileSizeParameters& lhs, const TileSizeParameters& rhs) {
  return (lhs.size_x + lhs.size_y) < (rhs.size_x + rhs.size_y);
}

struct TerrainTile {
  // local space offsets from origin of TerrainChunk to origin of this tile.
  vec3 x0y0z;
  vec3 x1y0z;
  vec3 x0y1z;
  vec3 x1y1z;
  
  /*
  average values of all vertices, included in tile

  used only on generation, TODO: pass as paramer and dispose.
      or keep to maybe modify terrain height per vertex at runtime
        (not just tile transform, which affects all vertices simultaneously)
  */
  TerrainPixelValues pixel_values;

  AdjastentTiles adjastent;

  MeshData* mesh_data;
};

class TerrainChunk {
public:
  vec3 pos;
  // number of TerrainTile patches by axes.
  int sizeX, sizeY;
  // number of vertices by axes.
  // for test assume minimal size of 4 vertices (4 == x0y0z, x1y0z, x0y1z, x1y1z in TerrainTile)
  TileSizeParameters tile_size = { 2, 2 };
  // distance between `tiles<TerrainTile>` vertices
  int scale = 1;
  vector<TerrainTile*> tiles;

  TerrainChunk(int sizeX, int sizeY, int scale, vec3 pos) :
    pos{ pos },
    sizeX{ sizeX }, sizeY{ sizeY },
    scale{ scale },
    tiles{ vector<TerrainTile*>(sizeX * sizeY) } {}
};
