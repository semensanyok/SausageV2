#pragma once

#include "Scene1.h"
#include <systems/GameLoop.h>

struct WorldEvent {

};

class CoefficientCallback {
public:
	int coef_index;
	virtual void call();
};

class WorldEventCallback {
public:
	virtual void call(WorldEvent evt);
};
