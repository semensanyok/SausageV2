#pragma once

#include <sausage.h>
#include <OpenGLHelpers.h>
#include <Settings.h>
#include <Shader.h>
#include <utils/AssetUtils.h>
#include <Logging.h>
#include <Texture.h>
#include <FileWatcher.h>
#include <systems/SystemsManager.h>

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
