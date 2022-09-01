#pragma once

#include "sausage.h"
#include "structures/TerrainStruct.h"

using namespace std;

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

////////////////////////////////////////////////////////

// need to have array of neighbours
// to form coherent textures, i.e. lakes, rivers, etc.
// if 2 adjastent sand/water/... - continue it
class TerrainCondition {
public:
  virtual bool IsPassed(TerrainPixelValues& values);
};

class ValueEvent {
  int base_chance;
  vector<TerrainCondition> terrain_conditions;
};

class BaseTerrainEvent {
  void AssignTexture();
};
//////////

/**
 * @brief buffer contains array of textures to blend (mix, multiply or whatever)
*/
class TerrainTexture {
  int blend_weight;
};

enum EventPriority {
  // lowest priority, because doesnt affect other events in any meaningfull way
  ASSIGN_BASE_TEXTURE
};

class TerrainFeature {

};

//class TileState {
//  map<hash(feature_type), vector<TerrainFeature>> tFeatures;
//  map<hash(feature_type), vector<EventFeature>> eFeatures;
//  map<hash(feature_type), vector<ResourceFeature>> rFeatures;
//};
