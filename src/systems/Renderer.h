#pragma once

#include "sausage.h"
#include "Camera.h"
#include "Settings.h"

class Renderer {
public:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_GLContext context;

	Renderer() {};
	~Renderer() {};

	void Render(Camera* camera)
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	}

	void InitContext() {
		SDL_Init(SDL_INIT_VIDEO);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

		//SDL_ShowCursor(SDL_ENABLE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifdef GL_DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		window = SDL_CreateWindow("caveview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			SCR_WIDTH, SCR_HEIGHT,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI // SDL_WINDOW_VULKAN
		);
		if (!window) LOG("Couldn't create window");

		context = SDL_GL_CreateContext(window);
		if (!context) LOG("Couldn't create context");

		SDL_GL_MakeCurrent(window, context); // is this true by default?

		// hide cursor, report only mouse motion events
		// SDL_SetRelativeMouseMode(SDL_TRUE);

		// enable VSync
		SDL_GL_SetSwapInterval(1);

		if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
		{
			LOG("[ERROR] Couldn't initialize glad");
		}
		else
		{
			LOG("[INFO] glad initialized\n");
		}

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glEnable(GL_DEPTH_TEST);
		// debug
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	}

	void ClearContext() {
		WriteShaderMsgsToLogFile();
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
};