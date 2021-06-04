#pragma once

#include "sausage.h"
#include "Camera.h"
#include "Settings.h"
#include "Scene.h"

class Controller {
public:
	Camera* camera;
	Scene* scene;

	Controller(Camera* camera, Scene* scene = nullptr) : camera{ camera }, scene{ scene } {};

	void SetScene(Scene* scene) {};
	void ProcessEvent(SDL_Event* e, float delta_time, bool& quit)
	{
		switch (e->type) {
		case SDL_MOUSEMOTION:
			// camera->MouseMotionCallback(e);
			break;
		case SDL_MOUSEBUTTONDOWN:
			break;
		case SDL_MOUSEBUTTONUP:
			break;
		case SDL_MOUSEWHEEL:
			camera->MouseWheelCallbackRTS(e->wheel, delta_time);
			scene->UpdateMVP(camera->projection_view);
			break;
		case SDL_QUIT:
			quit = 1;
			break;

		case SDL_WINDOWEVENT:
			switch (e->window.event) {
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				SCR_WIDTH = e->window.data1;
				SCR_HEIGHT = e->window.data2;
				glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
				camera->ResizeCallback(SCR_WIDTH, SCR_HEIGHT);
				scene->UpdateMVP(camera->projection_view);
				break;
			}
			break;

		case SDL_KEYDOWN: {
			int k = e->key.keysym.sym;
			int s = e->key.keysym.scancode;

			// Intercept SHIFT + ~ key stroke to toggle libRocket's 
						// visual debugger tool
			if (e->key.keysym.sym == SDLK_BACKQUOTE &&
				e->key.keysym.mod == KMOD_LSHIFT)
			{
				break;
			}

			SDL_Keymod mod;
			mod = SDL_GetModState();
			if (k == SDLK_ESCAPE)
				quit = 1;
			camera->KeyCallbackRTS(s, delta_time);
			scene->UpdateMVP(camera->projection_view);
			break;
		}
		case SDL_KEYUP: {
			break;
		}
		}
	}
};