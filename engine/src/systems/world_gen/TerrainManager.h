#pragma once

#include "sausage.h"
#include "AssetUtils.h"
#include "BufferSettings.h"
#include "OpenGLHelpers.h"
#include "TerrainTile.h"
#include "TerrainStruct.h"
#include "DrawCallManager.h"
#include "BufferManager.h"
#include "MeshDataBufferConsumer.h"
#include <FastNoise/FastNoise.h>
#include "Constants.h"

using namespace std;
using namespace glm;

/**

// --------------------------------------------------------------
  NoiseTypes : {
          HEIGHT
            randomly select geotype (noise function)
              1) Plain. slope 0% - 10%.
              2) Rolling. slope 10% - 25%.
              3) Mountainous. slope 25% - 60%.
              4) Steep. slope > 60%
          TEMPERATURE
            perlin, smooth
          MOISTURE
            simplex, sharper
        }


// --------------------------------------------------------------
  EventPriorities -> { ASSIGN_HEIGHT ASSIGN_TEXTURE }
  NoiseType ->
      ValueEvent1/2 hash -> [ValueEvent1, ValueEvent2]
      ValueEvent3 hash -> [ValueEvent3]
// --------------------------------------------------------------
Localized Cellular automata
    ValueEventN:
      chance (if all conditions are met): base + enhancer. i.e. base = 0.1. total: 0.1 (10%) - 1.0 (guaranteed)
      conditions: [{ if not ValueEvent2 }]
      actions: [
            Assign texture with blend weight
                  humidity >30% and tile has >30% iron -> rust
                  humidity >60% -> height < 0.3 ? river : waterfall
                  humidity 0 - 30 - grassland, 30 - 60 - forest (type selected on temperature)

                  River/Lake/Sea/WaterfallSpawner(temperature)

                  height > 0.5 -
            Spawn unique terrain feature - volcano
            Spawn unique event site - ruins, catacombs, ...
            Spawn resource - iron, copper, tin,
            SpawnMesh,

            Chance enhancer??? humidity > 30% - chance of water with height: [< 0.3 == 30%, 0.3 - 0.6 == 50%]
          ]


// WorldStates:
    GlobalState:
    Map<int, List<TileState>> tile_states

      TileState:
        // if loaded to GPU/CPU
        is_loaded = false;
        Map<hash(feature_type), List<TerrainFeature>> tFeatures
        Map<hash(feature_type), List<EventFeature>> eFeatures
        Map<hash(feature_type), List<ResourceFeature>> rFeatures


  Tiles States
    used for:
      1) efficient AI pathfidning / decision making, reduce raytracing;

    supports:
      - spatial lookup by (x,y)
      - should be raytraceable, to add/remove features and terraform at runtime

    notes:
      - preserve some ValueEventN, which are used in runtime to update texture or state
        (need to introduce async task frequency)

// START //////////////////////////////////////////////////////////////////

1. form "base light cavity" with Perlin noise?
  and then spawn mountain meshes all over the place? because rocky texture is dull and boring
  but I will have to hand craft thousands of rocks
    SOLUTION:
      - spawn mountains as 'SpawnMesh' event at places with high height values. they will too be procedurally generated, but noise texture 'SCALE' will be large.

      - same goes for places of low height vlaues, rivers/lakes/canyons etc.

      - base texture must be mix of - sand/glacier/steppe/snow/grass/swamp/rocks/...

2. check neighbours with distance of ... to form coherent texture as river/mount, octree
*/

// need to have array of neighbours
// to form coherent textures, i.e. lakes, rivers, etc.
// if 2 adjastent sand/water/... - continue it
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**

TODO_1 (AHORA): calculate each vertex normal as mean of adjastent faces normals
                 1 2 3
                 4 5 6
                 7 8 9
                 normal of vertex 5 is mean of normals of faces
                 [(1 2 4 5), (2 3 5 6), (4, 5, 7, 8), (5, 6, 8, 9)]
                 normal of face 1 2 4 5 == (1 - 2) x (1 - 4), where 1-4 - vertices (x,y,z)
                TBN IS NEEDED IN SHADER !!!

TODO_999 (far compartment): noise generators must be constant at least for height for consistent terrain, or find a way to sew tiles together

TODO ???: dont store thousands of tiles with transform matrices in memory.
          on gen step buffer instances tranforms, noise values and discard - free memory,
                                                              only reside in GPU uniform


19/01/2023 yet another revival.
terrain mesh is consistent. it is split by tiles logically.
each vert assigned tileId.
tile = 4 verts.
to each tileId its slot in blendTextures array.

*/
class TerrainManager
{
  MeshStaticBufferConsumer* buffer;
  MeshManager* mesh_manager;
  // TEST VALUES
  FastNoise::SmartNode<FastNoise::FractalFBm> fnFractal;
  FastNoise::SmartNode<FastNoise::Simplex> fnSimplex;
  vector<TerrainChunk*> chunks;
  DrawCallManager* draw_call_manager;
public:
  TerrainManager(BufferManager* buffer_manager,
    MeshManager* mesh_manager,
    DrawCallManager* draw_call_manager) :
    buffer{ buffer_manager->mesh_static_buffer },
    mesh_manager{ mesh_manager },
    draw_call_manager{ draw_call_manager } {
    //FastNoise::SmartNode<FastNoise::Simplex>
    fnSimplex = FastNoise::New<FastNoise::Simplex>();
    FastNoise::SmartNode sm = fnSimplex;
    //fnSimplex
    fnFractal = FastNoise::New<FastNoise::FractalFBm>();

    fnFractal->SetSource(fnSimplex);
    fnFractal->SetOctaveCount(5);
  };

  // testing purposes
  void CreateTerrain(int size_x, int size_y);

private:

  TerrainChunk* CreateChunk(
    vec3 pos,
    int noise_offset_x,
    int noise_offset_y,
    int size_x,
    int size_y,
    // OUT
    vector<vec3>& vertices,
    // OUT
    vector<unsigned int>& indices,
    // OUT
    vector<vec2>& uvs
  );
  void ReleaseBuffer(TerrainChunk* chunk);
  void Deactivate(TerrainChunk* chunk);
  vector<vec3> GenNormals(vector<vec3> vertices);
  ~TerrainManager() {};
};
