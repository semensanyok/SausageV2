#pragma once

#include "sausage.h"
#include "AssetUtils.h"
#include "SystemsManager.h"
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
  void CreateTerrain();

private:
  ~TerrainManager() {};
};
