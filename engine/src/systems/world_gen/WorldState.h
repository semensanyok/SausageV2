#pragma once

#include "sausage.h"

using namespace std;

class WorldState {
  map<int, List<TileState>> tile_states;
};
