#pragma once

#include "sausage.h"

class TerrainTile;
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

struct TerrainTile {
public:
  // local space offsets from origin of TerrainChunk to origin of this tile.
  vec3 x0y0z, x1y0z, x0y1z, x1y1z;
  // number of vertices by axes.
  // for test assume minimal size of 4 vertices
  TileSizeParameters size = { 2, 2 };
  /*
  average values of all vertices, included in tile

  used only on generation, TODO: pass as paramer and dispose.
      or keep to maybe modify terrain height per vertex at runtime
        (not just tile transform, which affects all vertices simultaneously)
  */
  TerrainPixelValues pixel_values;

  AdjastentTiles adjastent;

  // TODO: make sure it becomes instanced when Buffer it. (thousands rectangular patches)
  MeshData* mesh_data;
};

class TerrainChunk {
public:
  vec3 pos;
  // number of TerrainTile patches by axes.
  int sizeX, sizeY;
  // distance between `tiles<TerrainTile>` vertices
  int scale = 1;
  vector<TerrainTile*> tiles;

  TerrainChunk(int sizeX, int sizeY, int scale, vec3 pos) :
    pos{ pos },
    sizeX{ sizeX }, sizeY{ sizeY },
    scale{ scale },
    tiles{ vector<TerrainTile*>(sizeX * sizeY) } {}
};
