#pragma once

#include "sausage.h"
#include "TerrainStruct.h"

using namespace std;

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
