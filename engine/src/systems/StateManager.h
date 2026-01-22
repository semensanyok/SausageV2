#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Settings.h"
#include "Interfaces.h"

using namespace std;

class StateManager : public SausageSystem {
  static inline StateManager* instance;
  //unordered_map<unsigned long, pair<MeshData*, mat4>> physics_update;

public:
#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
  bool phys_debug_draw = true;
#else
  bool phys_debug_draw = false;
#endif

  static StateManager* GetInstance() {
    assert(instance != nullptr);
    return instance;
  }
  StateManager() {
    instance = this;
  };
	float delta_time = 0;
	float last_ticks = 0;
	uint32_t milliseconds_since_start = 0;
	double seconds_since_start = 0;

	void UpdateDeltaTimeTimings() {
		float this_ticks = SDL_GetTicks();
		delta_time = this_ticks - last_ticks;
		last_ticks = this_ticks;
		milliseconds_since_start = SDL_GetTicks();
		seconds_since_start = (double)milliseconds_since_start / 1000;
	}
	void Reset() {
		//physics_update.clear();
		milliseconds_since_start = 0;
		seconds_since_start = 0;
	}
	~StateManager() {};
};
