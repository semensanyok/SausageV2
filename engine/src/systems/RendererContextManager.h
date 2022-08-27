#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Logging.h"
#include "ImguiGui.h"
#include "structures/Structures.h"

class RendererContextManager : public SausageSystem {
public:
	SDL_Window* window;
	SDL_GLContext context;
	void InitContext();
	void ClearContext();
};
