#pragma once

#include "sausage.h"
#include "AssetUtils.h"
#include "SystemsManager.h"
#include "TerrainTile.h"
#include <FastNoise/FastNoise.h>

using namespace std;
/**

TODO_1 (AHORA): calculate each vertex normal as mean of adjastent faces normals
                 1 2 3
                 4 5 6
                 7 8 9
                 normal of vertex 5 is mean of normals of faces
                 [(1 2 4 5), (2 3 5 6), (4, 5, 7, 8), (5, 6, 8, 9)]
                 normal of face 1 2 4 5 == (1 - 2) x (1 - 4), where 1-4 - vertices (x,y,z)
                TBN IS NEEDED IN SHADER !!!
TODO_2 (AHORA): fuse mesh and terrain MeshData* into single draw call, since they are using same blinn_phong shader

TODO_999 (far compartment): noise generators must be constant at least for height for consistent terrain, or find a way to sew tiles together
*/
class TerrainManager
{
  // distance from one heightmap pixel to other
  const int SCALE = 1;
  FastNoise::SmartNode<FastNoise::FractalFBm> fnFractal;
  FastNoise::SmartNode<FastNoise::Simplex> fnSimplex;

public:
  TerrainManager() {
    //FastNoise::SmartNode<FastNoise::Simplex>
    fnSimplex = FastNoise::New<FastNoise::Simplex>();
    FastNoise::SmartNode sm = fnSimplex;
    //fnSimplex
    fnFractal = FastNoise::New<FastNoise::FractalFBm>();

    fnFractal->SetSource(fnSimplex);
    fnFractal->SetOctaveCount(5);
  };
  // testing purposes
  void CreateTerrain();

  TerrainChunk* CreateChunk(int size_x, int size_y, int noise_offset_x, int noise_offset_y);
  TerrainChunk* BufferTerrain(int world_offset_x, int world_offset_y);
  //ActivateTerrain(TerrainChunk* chunk);
  //DeactivateTerrain(TerrainChunk* chunk);

private:
  ~TerrainManager() {};
};
