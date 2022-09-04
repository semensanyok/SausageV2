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

struct TerrainTile {
  Point x0y0, x1y0, x0y1, x1y1;

  // used only on generation, TODO: pass as paramer and dispose.
  TerrainPixelValues pixel_values;

  AdjastentTiles adjastent;

  // TODO: 
  MeshData* mesh_data;
};

class TerrainChunk {
  int sizeX;
  int sizeY;

public:
  vector<TerrainTile*> tiles;

  TerrainChunk(int sizeX, int sizeY) : sizeX{ sizeX }, sizeY{ sizeY }, tiles{ vector<TerrainTile*>(sizeX * sizeY) } {}
};
