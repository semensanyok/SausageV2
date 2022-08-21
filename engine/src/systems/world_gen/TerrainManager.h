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
  void CreateTerrain() {
    MeshManager* mesh_manager = (MeshManager*)SausageSystems::registry["MeshManager"];

    int sizeX = 16;
    int sizeY = 16;

    vector<vec3> vertices(sizeX * sizeY);
    vector<unsigned int> indices;
    vector<float> noiseOutput(sizeX * sizeY);
    fnSimplex->GenUniformGrid2D(noiseOutput.data(), 0, 0, sizeX, sizeY, 0.02f, 1337);
    for (int y = 0; y < sizeY; y++) {
      for (int x = 0; x < sizeX; x++) {
        int ind = x + y * sizeY;
        vec3& vert = vertices[ind];
        vert.x = x;
        vert.y = y;
        vert.z = noiseOutput[ind];
        indices.push_back(ind);
        indices.push_back(ind + 1);
        indices.push_back(ind + 2);
      }
    }
    /*
    * TODO:
    *   generate normals, from texture or slope (diff between height values?)
    */
    vector<shared_ptr<MeshLoadData>> load_data = { mesh_manager->CreateMesh(vertices, indices) };

    BufferManager* buffer_manager = (BufferManager*)SausageSystems::registry["BufferManager"];
    vector<MeshDataBase*> mesh = { mesh_manager->CreateMeshData(load_data[0].get()) };
    buffer_manager->mesh_data_buffer->
      BufferMeshData(mesh, load_data);
    buffer_manager->mesh_data_buffer->
      SetBaseMeshForInstancedCommand(mesh, load_data);
  };

private:
  ~TerrainManager() {};
};
