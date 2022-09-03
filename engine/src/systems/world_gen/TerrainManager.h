#pragma once

#include "sausage.h"
#include "AssetUtils.h"
#include "SystemsManager.h"
#include "TerrainTile.h"
#include <FastNoise/FastNoise.h>

using namespace std;

/**
 * @brief manages existing TerrainTile
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
