#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Logging.h"
#include "Gui.h"


class RendererContextManager {
public:
	SDL_Window* window;
	SDL_GLContext context;
	void InitContext();
	void ClearContext();
};