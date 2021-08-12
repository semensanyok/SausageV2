#pragma once

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
